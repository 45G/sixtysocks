#include <errno.h>
#include <stdexcept>
#include <socks6util/socks6util.hh>
#include "rescheduleexception.hh"
#include "tlsexception.hh"
#include "poller.hh"
#include "tls.hh"

using namespace std;

extern PRIntn     _PR_InvalidInt(void);
extern PRInt16    _PR_InvalidInt16(void);
extern PRInt64    _PR_InvalidInt64(void);
extern PRStatus   _PR_InvalidStatus(void);
extern PRFileDesc *_PR_InvalidDesc(void);

NSPR_API(PRThread*) _PR_MD_CURRENT_THREAD(void);

extern void _MD_unix_map_recv_error(int err);
extern void _MD_unix_map_send_error(int err);
extern void _MD_unix_map_connect_error(int err);
extern void _MD_unix_map_getsockname_error(int err);
extern void _MD_unix_map_getpeername_error(int err);

struct PRTCPDescriptor: public PRFileDesc
{
	static void PR_CALLBACK destructor(PRFileDesc *fd)
	{
		PRTCPDescriptor *tcpDescriptor = reinterpret_cast<PRTCPDescriptor *>(fd);

		if (fd->lower !=NULL)
			fd->lower->higher = fd->higher;
		if (fd->higher !=NULL)
			fd->higher->lower = fd->lower;

		delete tcpDescriptor;
	}

	static PRStatus PR_CALLBACK dClose(PRFileDesc *fd)
	{
		destructor(fd);

		return PR_SUCCESS;
	}

	static PRInt32 PR_CALLBACK dRecv(PRFileDesc *fd, void *buf, PRInt32 amount, PRIntn flags, PRIntervalTime timeout)
	{
		(void)timeout;

		PRTCPDescriptor *tcpDescriptor = reinterpret_cast<PRTCPDescriptor *>(fd);

		int rc = recv(tcpDescriptor->rfd, buf, amount, flags);
		if (rc < 0)
			_MD_unix_map_recv_error(errno);

		return rc;
	}

	static PRInt32 PR_CALLBACK dRead(PRFileDesc *fd, void *buf, PRInt32 amount)
	{
		return dRecv(fd, buf, amount, MSG_NOSIGNAL, PR_INTERVAL_NO_TIMEOUT);
	}

	static PRInt32 PR_CALLBACK dSend(PRFileDesc *fd, const void *buf, PRInt32 amount, PRIntn flags, PRIntervalTime timeout)
	{
		(void)timeout;

		PRTCPDescriptor *tcpDescriptor = reinterpret_cast<PRTCPDescriptor *>(fd);

		int rc;

		if (tcpDescriptor->attemptSendTo)
		{
			tcpDescriptor->attemptSendTo = false;
			rc = sendto(tcpDescriptor->rfd, buf, amount, flags | MSG_FASTOPEN, &tcpDescriptor->addr.sockAddress, tcpDescriptor->addr.size());

		}
		else
		{
			rc = send(tcpDescriptor->rfd, buf, amount, flags);
		}

		if (rc < 0)
			_MD_unix_map_send_error(errno);

		return rc;
	}

	static PRInt32 PR_CALLBACK dWrite(PRFileDesc *fd, const void *buf, PRInt32 amount)
	{
		return dSend(fd, buf, amount, MSG_NOSIGNAL, PR_INTERVAL_NO_TIMEOUT);
	}

	static PRStatus PR_CALLBACK dConnectContinue(PRFileDesc *fd, PRInt16 outFlags)
	{
		(void)outFlags;

		PRTCPDescriptor *tcpDescriptor = reinterpret_cast<PRTCPDescriptor *>(fd);

		int err;
		socklen_t optlen = sizeof(err);
		int rc = getsockopt(tcpDescriptor->rfd, SOL_SOCKET, SO_ERROR, &err, &optlen);
		if (rc < 0)
		{
			_MD_unix_map_connect_error(errno);
			return PR_FAILURE;
		}
		else if (err != 0)
		{
			_MD_unix_map_connect_error(err);
			return PR_FAILURE;
		}
		return PR_SUCCESS;
	}

	static PRStatus PR_CALLBACK dGetName(PRFileDesc *fd, PRNetAddr *addr)
	{
		PRTCPDescriptor *tcpDescriptor = reinterpret_cast<PRTCPDescriptor *>(fd);

		socklen_t addrLen = sizeof(PRNetAddr);
		int rc = getsockname(tcpDescriptor->rfd, (struct sockaddr *) addr, &addrLen);
		if (rc < 0)
		{
		    _MD_unix_map_getsockname_error(errno);
		    return PR_FAILURE;
		}

		if (addr->raw.family == AF_INET6)
			addr->raw.family = PR_AF_INET6;

		return PR_SUCCESS;
	}

	static PRStatus PR_CALLBACK dGetPeerName(PRFileDesc *fd, PRNetAddr *addr)
	{
		PRTCPDescriptor *tcpDescriptor = reinterpret_cast<PRTCPDescriptor *>(fd);

		socklen_t addrLen = sizeof(PRNetAddr);
		int rc = getpeername(tcpDescriptor->rfd, (struct sockaddr *) addr, &addrLen);
		if (rc < 0)
		{
		    _MD_unix_map_getpeername_error(errno);
		    return PR_FAILURE;
		}

		if (addr->raw.family == AF_INET6)
			addr->raw.family = PR_AF_INET6;

		return PR_SUCCESS;
	}

	static constexpr PRIOMethods METHODS = {
		PR_DESC_SOCKET_TCP,
		dClose,
		dRead,
		dWrite,
		(PRAvailableFN)_PR_InvalidInt, //SocketAvailable,
		(PRAvailable64FN)_PR_InvalidInt64, //SocketAvailable64,
		(PRFsyncFN)_PR_InvalidInt, //SocketSync,
		(PRSeekFN)_PR_InvalidInt,
		(PRSeek64FN)_PR_InvalidInt64,
		(PRFileInfoFN)_PR_InvalidStatus,
		(PRFileInfo64FN)_PR_InvalidStatus,
		(PRWritevFN)_PR_InvalidInt, //SocketWritev,
		(PRConnectFN)_PR_InvalidInt, //SocketConnect,
		(PRAcceptFN)_PR_InvalidDesc, //SocketAccept,
		(PRBindFN)_PR_InvalidInt, //SocketBind,
		(PRListenFN)_PR_InvalidInt, //SocketListen,
		(PRShutdownFN)_PR_InvalidInt, //SocketShutdown,
		dRecv,
		dSend,
		(PRRecvfromFN)_PR_InvalidInt,
		(PRSendtoFN)_PR_InvalidInt,
		(PRPollFN)_PR_InvalidInt16, //SocketPoll,
		(PRAcceptreadFN)_PR_InvalidInt, //SocketAcceptRead,
		(PRTransmitfileFN)_PR_InvalidInt, //SocketTransmitFile,
		dGetName,
		dGetPeerName,
		(PRReservedFN)_PR_InvalidInt,
		(PRReservedFN)_PR_InvalidInt,
		(PRGetsocketoptionFN)_PR_InvalidInt, //_PR_SocketGetSocketOption,
		(PRSetsocketoptionFN)_PR_InvalidInt, //_PR_SocketSetSocketOption,
		(PRSendfileFN)_PR_InvalidInt, //SocketSendFile,
		dConnectContinue,
		(PRReservedFN)_PR_InvalidInt,
		(PRReservedFN)_PR_InvalidInt,
		(PRReservedFN)_PR_InvalidInt,
		(PRReservedFN)_PR_InvalidInt
	};

	int rfd;
	int wfd;
	
	S6U::SocketAddress addr;
	bool attemptSendTo;
	
	PRTCPDescriptor(int fd, const S6U::SocketAddress &addr = S6U::SocketAddress())
		: rfd(fd), wfd(fd), addr(addr), attemptSendTo(addr.isValid())
	{
		methods = &METHODS;
		secret = NULL;
		lower = NULL;
		higher = NULL;
		dtor = destructor;
		identity = PR_NSPR_IO_LAYER;
	}
};

enum BlockDirection
{
	BD_IN,
	BD_OUT,
};

static void tlsHandleErr(BlockDirection bd, int fd)
{
	PRErrorCode err = PR_GetError();
	if (err == PR_WOULD_BLOCK_ERROR)
	{
		if (bd == BD_IN)
			throw RescheduleException(fd, Poller::IN_EVENTS);
		else /* BD_OUT */
			throw RescheduleException(fd, Poller::OUT_EVENTS);
	}
	throw TLSException(err);
}

void TLS::handshakeCallback(PRFileDesc *fd, void *clientData)
{
	(void)fd;

	TLS *tls = reinterpret_cast<TLS *>(clientData);

	tls->handshakeFinished = true;
}

SECStatus TLS::canFalseStartCallback(PRFileDesc *fd, void *arg, PRBool *canFalseStart)
{
	(void)fd; (void)arg;

	*canFalseStart = PR_TRUE;
	return SECSuccess;
}

TLS::TLS(TLSContext *ctx, int fd)
	: rfd(fd), wfd(fd), connectCalled(false), handshakeFinished(false), descriptor(fd)
{
	//static const int CERT_VERIFY_DEPTH = 3; //TODO: do something with this?
	SECStatus rc = SSL_ResetHandshake(descriptor, ctx->isServer());
	if (rc != SECSuccess)
		throw TLSException();

	rc = SSL_HandshakeCallback(descriptor, handshakeCallback, this);

	rc = SSL_SetCanFalseStartCallback(descriptor, canFalseStartCallback, NULL);
	if (rc != SECSuccess)
		throw TLSException();
}

TLS::~TLS()
{
}

void TLS::setReadFD(int fd)
{
	rfd = fd;
	//TODO: something else?
}

void TLS::setWriteFD(int fd)
{
	wfd = fd;
	//TODO: something else?
}

void TLS::tlsConnect(S6U::SocketAddress *addr, StreamBuffer *buf, bool useEarlyData)
{
	SECStatus rc;
	useEarlyData = useEarlyData && addr != NULL && buf != NULL;

	if (!connectCalled && useEarlyData && buf->usedSize() > 0)
	{
		connectCalled = true;

		PRInt32 earlyDataWritten = PR_SendTo(descriptor, buf->getHead(), buf->usedSize(), MSG_FASTOPEN | MSG_NOSIGNAL,
			reinterpret_cast<const PRNetAddr *>(addr), PR_INTERVAL_NO_TIMEOUT);
		if (earlyDataWritten < 0)
			tlsHandleErr(BD_OUT, wfd);

		buf->unuse(earlyDataWritten);
	}
	else
	{
		rc = SSL_ForceHandshake(descriptor);
		if (rc != SECSuccess)
			tlsHandleErr(BD_IN, rfd);
	}
}

void TLS::tlsAccept(StreamBuffer *buf)
{
	do
	{
		SECStatus rc = SSL_ForceHandshake(descriptor);
		if (rc != SECSuccess)
			tlsHandleErr(BD_IN, rfd);

		if (handshakeFinished)
			return;

		PRInt32 dataRead = PR_Recv(descriptor, buf->getTail(), buf->availSize(), MSG_NOSIGNAL, PR_INTERVAL_NO_TIMEOUT);
		if (dataRead < 0)
			tlsHandleErr(BD_IN, rfd);

		buf->use(dataRead);
	}
	while (true);
}

size_t TLS::tlsWrite(StreamBuffer *buf)
{
	PRInt32 bytes = PR_Send(descriptor, buf->getHead(), buf->usedSize(), MSG_NOSIGNAL, PR_INTERVAL_NO_TIMEOUT);
	if (bytes < 0)
		tlsHandleErr(BD_OUT, wfd);
	
	buf->unuse(bytes);
	return bytes;
}

size_t TLS::tlsRead(StreamBuffer *buf)
{
	PRInt32 bytes = PR_Recv(descriptor, buf->getTail(), buf->availSize(), MSG_NOSIGNAL, PR_INTERVAL_NO_TIMEOUT);
	if (bytes < 0)
		tlsHandleErr(BD_IN, rfd);
	
	buf->use(bytes);
	return bytes;
}

TLS::Descriptor::Descriptor(int fd)
{
	fileDescriptor = PR_ImportTCPSocket(fd);
	if (!fileDescriptor)
		throw TLSException();
	PRFileDesc *newDesc = SSL_ImportFD(NULL, fileDescriptor);
	if (!newDesc)
	{
		PR_Close(fileDescriptor); //might return error
		throw TLSException();
	}
	fileDescriptor = newDesc;
}

TLS::Descriptor::~Descriptor()
{
	PR_Close(fileDescriptor); //might return error
}
