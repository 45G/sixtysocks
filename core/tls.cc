#include <stdexcept>
#include <socks6util/socks6util.hh>
#include "rescheduleexception.hh"
#include "tlsexception.hh"
#include "poller.hh"
#include "tls.hh"

using namespace std;

static PRIOMethods tcpMethods = {
	PR_DESC_SOCKET_TCP,
	SocketClose,
	SocketRead,
	SocketWrite,
	SocketAvailable,
	SocketAvailable64,
	SocketSync,
	(PRSeekFN)_PR_InvalidInt,
	(PRSeek64FN)_PR_InvalidInt64,
	(PRFileInfoFN)_PR_InvalidStatus,
	(PRFileInfo64FN)_PR_InvalidStatus,
	SocketWritev,
	SocketConnect,
	SocketAccept,
	SocketBind,
	SocketListen,
	SocketShutdown,
	SocketRecv,
	SocketSend,
	(PRRecvfromFN)_PR_InvalidInt,
	#if defined(_WIN64) && defined(WIN95)
	SocketTCPSendTo, /* This is for fast open. We imitate Linux interface. */
	#else
	(PRSendtoFN)_PR_InvalidInt,
	#endif
	SocketPoll,
	SocketAcceptRead,
	SocketTransmitFile,
	SocketGetName,
	SocketGetPeerName,
	(PRReservedFN)_PR_InvalidInt,
	(PRReservedFN)_PR_InvalidInt,
	_PR_SocketGetSocketOption,
	_PR_SocketSetSocketOption,
	SocketSendFile,
	SocketConnectContinue,
	(PRReservedFN)_PR_InvalidInt, 
	(PRReservedFN)_PR_InvalidInt, 
	(PRReservedFN)_PR_InvalidInt, 
	(PRReservedFN)_PR_InvalidInt
};

struct PRTCPDescriptor: public PRFileDesc
{
	int rfd;
	int wfd;
	
	S6U::SocketAddress addr;
	bool attemptSendTo;
	
	void destructor(PRFileDesc *fd)
	{
		PRTCPDescriptor *descriptor = reinterpret_cast<PRTCPDescriptor *>(fd);
		
		//TODO
		
		delete descriptor;
	}
	
	PRTCPDescriptor(int fd, const S6U::SocketAddress &addr = S6U::SocketAddress())
		: methods(), secret(NULL), lower(NULL), higher(NULL), dtor(destructor), identity(),
		  rfd(fd), wfd(fd), addr(addr), attemptSendTo(addr.isValid()) {}
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
