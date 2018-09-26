#include <stdexcept>
#include "rescheduleexception.hh"
#include "tlsexception.hh"
#include "poller.hh"
#include "tls.hh"

using namespace std;

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

TLS::TLS(TLSContext *ctx, int fd)
	: rfd(fd), wfd(fd), descriptor(fd)
{
	//static const int CERT_VERIFY_DEPTH = 3;
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
	int rc;
	int earlyDataWritten = 0;
	
	useEarlyData = useEarlyData && addr != NULL && buf != NULL;
	
	bool firstConnnect = !connectCalled;
	connectCalled = true;

	if (firstConnnect && addr != NULL)
		wolfSSL_SetTFOAddr(readTLS, &addr->storage, addr->size());

	if (firstConnnect && useEarlyData && buf->usedSize() > 0)
	{
		rc = wolfSSL_write_early_data(readTLS, buf->getHead(), buf->usedSize(), &earlyDataWritten);
		if (rc < 0)
			tlsHandleErr(readTLS, rc, rfd);
	}
	else
	{
		rc = wolfSSL_connect(readTLS);
		if (rc != SSL_SUCCESS)
			tlsHandleErr(readTLS, rc, rfd);
	}
	
	if (useEarlyData)
		buf->unuse(earlyDataWritten);
}

void TLS::tlsAccept(StreamBuffer *buf)
{
	//SECStatusCheck(SSL_OptionSetDefault(SSL_ENABLE_0RTT_DATA, PR_TRUE)); //TODO: move to TLSConnect
	
	int earlyDataRead = 0;
	int rc = wolfSSL_read_early_data(readTLS, buf->getTail(), buf->availSize(), &earlyDataRead);
	if (rc < 0)
		tlsHandleErr(writeTLS, rc, rfd);
	
	buf->use(earlyDataRead);
}

size_t TLS::tlsWrite(StreamBuffer *buf)
{
	int bytes = PR_Write(descriptor, buf->getHead(), buf->usedSize());
	if (bytes < 0)
		tlsHandleErr(BD_OUT, wfd);
	
	buf->unuse(bytes);
	return bytes;
}

size_t TLS::tlsRead(StreamBuffer *buf)
{
	int bytes = PR_Read(descriptor, buf->getTail(), buf->availSize());
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
