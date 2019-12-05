#include <errno.h>
#include <stdexcept>
#include <socks6util/socks6util.hh>
extern "C"
{
#include <private/pprio.h>
}
#include "../core/rescheduleexception.hh"
#include "tlsexception.hh"
#include "../core/poller.hh"
#include "tls.hh"
#include "nspr_stuff.h"

using namespace std;

static SECStatus canFalseStartCallback(PRFileDesc *fd, void *arg, PRBool *canFalseStart)
{
	(void)fd; (void)arg;

	*canFalseStart = PR_TRUE;
	return SECSuccess;
}

static void PR_CALLBACK descriptorDeleter(PRFileDesc *fd)
{
	if (fd->higher)
		fd->higher->lower = nullptr;
	delete fd;
}

TLS::TLS(TLSContext *ctx, int fd)
	: readFD(fd), writeFD(fd)
{
	PRFileDesc *lowerDesc = new PRFileDesc({
		.methods  = &METHODS,
		.secret   = reinterpret_cast<PRFilePrivate *>(this),
		.lower    = nullptr,
		.higher   = nullptr,
		.dtor     = descriptorDeleter,
		.identity = PR_NSPR_IO_LAYER,
	});

	PRFileDesc *higherDesc = SSL_ImportFD(nullptr, lowerDesc);
	if (!higherDesc)
	{
		PRErrorCode err = PR_GetError();
		PR_Close(lowerDesc); //might return error
		throw TLSException(err);
	}

	descriptor.reset(higherDesc);
	

	if (ctx->isServer())
	{
		/* set key + cert */
		SECStatus rc = SSL_ConfigServerCert(descriptor.get(), ctx->getCert(), ctx->getKey(), nullptr, 0);
		if (rc != SECSuccess)
			throw TLSException();

		/* setup anti-replay */
#ifdef SSL_CreateAntiReplayContext
		auto antiReplayCtx = ctx->getAntiReplayCtx();
		if (antiReplayCtx)
		{
			rc = SSL_SetAntiReplayContext(descriptor.get(), antiReplayCtx);
			if (rc != SECSuccess)
				throw TLSException();
		}
#endif
	}
	
	if (ctx->isClient())
	{
		/* set SNI */
		SECStatus rc = SSL_SetURL(descriptor.get(), ctx->getSNI()->c_str());
		if (rc != SECSuccess)
			throw TLSException();
		
		/* false start */
		rc = SSL_SetCanFalseStartCallback(descriptor.get(), canFalseStartCallback, nullptr);
		if (rc != SECSuccess)
			throw TLSException();
	}

	//static const int CERT_VERIFY_DEPTH = 3; //TODO: do something with this?
	SECStatus rc = SSL_ResetHandshake(descriptor.get(), ctx->isServer());
	if (rc != SECSuccess)
		throw TLSException();
}

void TLS::setReadFD(int fd)
{
	this->readFD = fd;
}

void TLS::setWriteFD(int fd)
{
	this->writeFD = fd;
}

enum BlockDirection
{
	BD_IN,
	BD_OUT,
};

static thread_local BlockDirection blockDirection;

static void tlsHandleErr(int fd)
{
	PRErrorCode err = PR_GetError();
	if (err == PR_WOULD_BLOCK_ERROR || err == PR_IN_PROGRESS_ERROR)
	{
		if (blockDirection == BD_IN)
			throw RescheduleException(fd, Poller::IN_EVENTS);
		else /* BD_OUT */
			throw RescheduleException(fd, Poller::OUT_EVENTS);
	}
	if (err == PR_END_OF_FILE_ERROR)
		return;
	throw TLSException(err);
}

void TLS::tlsDisableEarlyData()
{
	SECStatus rc = SSL_OptionSet(descriptor.get(), SSL_ENABLE_0RTT_DATA, PR_FALSE);
	if (rc < 0)
		throw TLSException();
	state = S_LAISEZ_FAIRE;

}

void TLS::clientHandshake(StreamBuffer *buf)
{
	switch (state)
	{
	case S_WANT_EARLY:
	{
		PRInt32 bytes = PR_Write(descriptor.get(), buf->getHead(), buf->usedSize());
		if (bytes < 0)
		{
			tlsHandleErr(writeFD);
			bytes = 0;
		}
		earlyWritten = bytes;
		state = S_WANT_HANDSHAKE;
		[[fallthrough]];
	}

	case S_WANT_HANDSHAKE:
	{
		SECStatus rc = SSL_ForceHandshake(descriptor.get());
		if (rc < 0)
			tlsHandleErr(writeFD);

		SSLChannelInfo info;
		rc = SSL_GetChannelInfo(descriptor.get(), &info, sizeof(info));
		if (rc < 0)
			tlsHandleErr(writeFD);

		if (info.earlyDataAccepted)
			buf->unuse(earlyWritten);

		state = S_LAISEZ_FAIRE;
		[[fallthrough]];
	}

	case S_LAISEZ_FAIRE:
		return;
	}
}

size_t TLS::tlsWrite(StreamBuffer *buf)
{
	PRInt32 bytes = PR_Write(descriptor.get(), buf->getHead(), buf->usedSize());
	if (bytes < 0)
	{
		tlsHandleErr(writeFD);
		bytes = 0;
	}
	
	buf->unuse(bytes);
	return bytes;
}

size_t TLS::tlsRead(StreamBuffer *buf)
{
	PRInt32 bytes = PR_Read(descriptor.get(), buf->getTail(), buf->availSize());
	if (bytes < 0)
	{
		tlsHandleErr(readFD);
		bytes = 0;
	}
	
	buf->use(bytes);
	return bytes;
}

//TODO: cull this
static const unordered_map<int, PRErrorCode> DEFAULT_ERRORS = {
	{ EACCES,          PR_NO_ACCESS_RIGHTS_ERROR },
	{ EADDRINUSE,      PR_ADDRESS_IN_USE_ERROR },
	{ EADDRNOTAVAIL,   PR_ADDRESS_NOT_AVAILABLE_ERROR },
	{ EAFNOSUPPORT,    PR_ADDRESS_NOT_SUPPORTED_ERROR },
	{ EAGAIN,          PR_WOULD_BLOCK_ERROR },
	/* On QNX and Neutrino, EALREADY is defined as EBUSY. */
#if EALREADY != EBUSY
	{ EALREADY,        PR_ALREADY_INITIATED_ERROR },
#endif
	{ EBADF,           PR_BAD_DESCRIPTOR_ERROR },
#ifdef EBADMSG
	{ EBADMSG,         PR_IO_ERROR },
#endif
	{ ECONNABORTED,    PR_CONNECT_ABORTED_ERROR },
	{ ECONNREFUSED,    PR_CONNECT_REFUSED_ERROR },
	{ ECONNRESET,      PR_CONNECT_RESET_ERROR },
	{ EDEADLK,         PR_DEADLOCK_ERROR },
	{ EEXIST,          PR_FILE_EXISTS_ERROR },
	{ EFAULT,          PR_ACCESS_FAULT_ERROR },
	{ EHOSTUNREACH,    PR_HOST_UNREACHABLE_ERROR },
	{ EHOSTDOWN,       PR_HOST_UNREACHABLE_ERROR },
	{ EINPROGRESS,     PR_IN_PROGRESS_ERROR },
	{ EINTR,           PR_PENDING_INTERRUPT_ERROR },
	{ EINVAL,          PR_INVALID_ARGUMENT_ERROR },
	{ EIO,             PR_IO_ERROR },
	{ EISCONN,         PR_IS_CONNECTED_ERROR },
	{ ELOOP,           PR_LOOP_ERROR },
	{ EMFILE,          PR_PROC_DESC_TABLE_FULL_ERROR },
	{ EMSGSIZE,        PR_INVALID_ARGUMENT_ERROR },
#ifdef EMULTIHOP
	{ EMULTIHOP,       PR_REMOTE_FILE_ERROR },
#endif
	{ ENAMETOOLONG,    PR_NAME_TOO_LONG_ERROR },
	{ ENETUNREACH,     PR_NETWORK_UNREACHABLE_ERROR },
	{ ENFILE,          PR_SYS_DESC_TABLE_FULL_ERROR },
#ifdef ENOBUFS
	{ ENOBUFS,         PR_INSUFFICIENT_RESOURCES_ERROR },
#endif
#ifdef ENOLINK
	{ ENOLINK,         PR_REMOTE_FILE_ERROR },
#endif
	{ ENOMEM,          PR_OUT_OF_MEMORY_ERROR },
	{ ENOPROTOOPT,     PR_INVALID_ARGUMENT_ERROR },
#ifdef ENOSR
	{ ENOSR,           PR_INSUFFICIENT_RESOURCES_ERROR },
#endif
	{ ENOSYS,          PR_NOT_IMPLEMENTED_ERROR },
	{ ENOTCONN,        PR_NOT_CONNECTED_ERROR },
	{ ENOTSOCK,        PR_NOT_SOCKET_ERROR },
	{ ENXIO,           PR_FILE_NOT_FOUND_ERROR },
	{ EOPNOTSUPP,      PR_NOT_TCP_SOCKET_ERROR },
#ifdef EOVERFLOW
	{ EOVERFLOW,       PR_BUFFER_OVERFLOW_ERROR },
#endif
	{ EPERM,           PR_NO_ACCESS_RIGHTS_ERROR },
	{ EPIPE,           PR_CONNECT_RESET_ERROR },
#ifdef EPROTO
	{ EPROTO,          PR_IO_ERROR },
#endif
	{ EPROTONOSUPPORT, PR_PROTOCOL_NOT_SUPPORTED_ERROR },
	{ EPROTOTYPE,      PR_ADDRESS_NOT_SUPPORTED_ERROR },
	{ ERANGE,          PR_INVALID_METHOD_ERROR },
	{ ESPIPE,          PR_INVALID_METHOD_ERROR },
	{ ETIMEDOUT,       PR_IO_TIMEOUT_ERROR },
	{ EWOULDBLOCK,     PR_WOULD_BLOCK_ERROR },
	{ EXDEV,           PR_NOT_SAME_DEVICE_ERROR },
};

void mapDefaultError(int err)
{
	PRErrorCode prErr = PR_UNKNOWN_ERROR;
	auto it = DEFAULT_ERRORS.find(err);
	if (it != DEFAULT_ERRORS.end())
		prErr = it->second;
	PR_SetError(prErr, err);
}

template <typename T>
T invalidFn()
{
	PR_SetError(PR_INVALID_METHOD_ERROR, 0);
	
	if constexpr (is_same<T, PRFileDesc *>::value)
		return nullptr;
	if constexpr (is_same<T, PRStatus>::value)
		return PR_FAILURE;
	if constexpr (is_integral<T>::value)
		return -1;
	assert(false);
}

template <typename T>
constexpr auto INVALID = (T)invalidFn<typename decltype(function{declval<T>()})::result_type>;

const PRIOMethods TLS::METHODS = {
	.file_type       = PR_DESC_SOCKET_TCP,
	.close           = dClose,
	.read            = dRead,
	.write           = dWrite,
	.available       = INVALID<PRAvailableFN>,       /* wasSocketAvailable */
	.available64     = INVALID<PRAvailable64FN>,     /* wasSocketAvailable64 */
	.fsync           = INVALID<PRFsyncFN>,           /* wasSocketSync */
	.seek            = INVALID<PRSeekFN>,
	.seek64          = INVALID<PRSeek64FN>,
	.fileInfo        = INVALID<PRFileInfoFN>,
	.fileInfo64      = INVALID<PRFileInfo64FN>,
	.writev          = INVALID<PRWritevFN>,          /* wasSocketWritev */
	.connect         = INVALID<PRConnectFN>,         /* wasSocketConnect */
	.accept          = INVALID<PRAcceptFN>,          /* wasSocketAccept */
	.bind            = INVALID<PRBindFN>,            /* wasSocketBind */
	.listen          = INVALID<PRListenFN>,          /* wasSocketListen */
	.shutdown        = INVALID<PRShutdownFN>,        /* wasSocketShutdown */
	.recv            = dRecv,
	.send            = dSend,
	.recvfrom        = INVALID<PRRecvfromFN>,
	.sendto          = INVALID<PRSendtoFN>,
	.poll            = INVALID<PRPollFN>,            /* wasSocketPoll */
	.acceptread      = INVALID<PRAcceptreadFN>,      /* wasSocketAcceptRead */
	.transmitfile    = INVALID<PRTransmitfileFN>,    /* wasSocketTransmitFile */
	.getsockname     = dGetName,
	.getpeername     = dGetPeerName,
	.reserved_fn_6   = INVALID<PRReservedFN>,
	.reserved_fn_5   = INVALID<PRReservedFN>,
	.getsocketoption = dGetSocketOption,             /* was_PR_SocketGetSocketOption */
	.setsocketoption = INVALID<PRSetsocketoptionFN>, /* was_PR_SocketSetSocketOption */
	.sendfile        = INVALID<PRSendfileFN>,        /* wasSocketSendFile */
	.connectcontinue = INVALID<PRConnectcontinueFN>, //TODO
	.reserved_fn_3   = INVALID<PRReservedFN>,
	.reserved_fn_2   = INVALID<PRReservedFN>,
	.reserved_fn_1   = INVALID<PRReservedFN>,
	.reserved_fn_0   = INVALID<PRReservedFN>
};

PRStatus PR_CALLBACK TLS::dClose(PRFileDesc *fd)
{
	fd->secret = nullptr;
	fd->dtor(fd);

	return PR_SUCCESS;
}

PRInt32 PR_CALLBACK TLS::dRecv(PRFileDesc *fd, void *buf, PRInt32 amount, PRIntn flags, PRIntervalTime timeout)
{
	(void)timeout;

	TLS *tls = reinterpret_cast<TLS *>(fd->secret);

	int rc = recv(tls->readFD, buf, amount, flags);
	if (rc < 0)
		mapDefaultError(errno);
	
	blockDirection = BD_IN;

	return rc;
}

PRInt32 PR_CALLBACK TLS::dRead(PRFileDesc *fd, void *buf, PRInt32 amount)
{
	return dRecv(fd, buf, amount, MSG_NOSIGNAL, PR_INTERVAL_NO_TIMEOUT);
}

PRInt32 PR_CALLBACK TLS::dSend(PRFileDesc *fd, const void *buf, PRInt32 amount, PRIntn flags, PRIntervalTime timeout)
{
	(void)timeout;

	TLS *tls = reinterpret_cast<TLS *>(fd->secret);

	ssize_t rc = send(tls->writeFD, buf, amount, flags);
	if (rc < 0)
		mapDefaultError(errno);
	
	blockDirection = BD_OUT;

	return rc;
}

PRInt32 PR_CALLBACK TLS::dWrite(PRFileDesc *fd, const void *buf, PRInt32 amount)
{
	return dSend(fd, buf, amount, MSG_NOSIGNAL, PR_INTERVAL_NO_TIMEOUT);
}

PRStatus PR_CALLBACK TLS::dGetName(PRFileDesc *fd, PRNetAddr *addr)
{
	TLS *tls = reinterpret_cast<TLS *>(fd->secret);

	socklen_t addrLen = sizeof(PRNetAddr);
	int rc = getsockname(tls->readFD, (struct sockaddr *) addr, &addrLen);
	if (rc < 0)
	{
		_MD_unix_map_getsockname_error(errno);
		return PR_FAILURE;
	}

	if (addr->raw.family == AF_INET6)
		addr->raw.family = PR_AF_INET6;

	return PR_SUCCESS;
}

PRStatus PR_CALLBACK TLS::dGetPeerName(PRFileDesc *fd, PRNetAddr *addr)
{
	TLS *tls = reinterpret_cast<TLS *>(fd->secret);

	socklen_t addrLen = sizeof(PRNetAddr);
	int rc = getpeername(tls->readFD, (struct sockaddr *) addr, &addrLen);
	if (rc < 0)
		_MD_unix_map_getpeername_error(errno);

	return PR_SUCCESS;
}

PRStatus PR_CALLBACK TLS::dGetSocketOption(PRFileDesc *fd, PRSocketOptionData *data)
{
	(void)fd;
	
	if (PR_SockOpt_Nonblocking == data->option)
	{
		data->value.non_blocking = PR_TRUE;
		return PR_SUCCESS;
	}
	
	//TODO: the rest
	return PR_FAILURE;
}
