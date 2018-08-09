#include <stdexcept>
#include "rescheduleexception.hh"
#include "tlsexception.hh"
#include "poller.hh"
#include "tls.hh"

using namespace std;

#define TLS_HANDLE(tls, rc, fd) \
{ \
	int err = wolfSSL_get_error((tls), (rc)); \
	if (err == WOLFSSL_ERROR_WANT_READ) \
		throw RescheduleException((fd), Poller::IN_EVENTS); \
	if (err == WOLFSSL_ERROR_WANT_WRITE) \
		throw RescheduleException((fd), Poller::OUT_EVENTS); \
	throw TLSException(err); \
}

TLS::TLS(TLSContext *ctx, int fd)
	: rfd(fd), wfd(fd)
{
	readTLS = wolfSSL_new(ctx->get());
	if (readTLS == NULL)
		throw runtime_error("Error creating context");
	writeTLS = readTLS;
		
	wolfSSL_SetIOWriteFlags(readTLS, MSG_NOSIGNAL);
	
	static const int CERT_VERIFY_DEPTH = 3;
	if (ctx->isClient())
		wolfSSL_set_verify_depth(readTLS, CERT_VERIFY_DEPTH);
}

TLS::~TLS()
{
	if (readTLS == writeTLS)
		writeTLS = NULL;
	
	wolfSSL_free(writeTLS);
	wolfSSL_free(readTLS);
}

void TLS::setReadFD(int fd)
{
	if (readTLS == writeTLS)
	{
		writeTLS = wolfSSL_write_dup(readTLS);
		if (writeTLS == NULL)
			throw runtime_error("Error duplicating WOLFSSL");
	}
	rfd = fd;
}

void TLS::setWriteFD(int fd)
{
	if (readTLS == writeTLS)
	{
		writeTLS = wolfSSL_write_dup(readTLS);
		if (writeTLS == NULL)
			throw runtime_error("Error duplicating WOLFSSL");
	}
	wfd = fd;
}

void TLS::tlsConnect(S6U::SocketAddress *addr, StreamBuffer *buf, bool useEarlyData)
{
	int rc;
	int earlyDataWritten = 0;
	
	useEarlyData = useEarlyData && addr != NULL && buf != NULL;
	
	if (addr != NULL)
		wolfSSL_SetTFOAddr(readTLS, &addr->storage, addr->size());
	
	if (useEarlyData && buf->usedSize() > 0)
		rc = wolfSSL_write_early_data(readTLS, buf->getHead(), buf->usedSize(), &earlyDataWritten);
	else
		rc = wolfSSL_connect(readTLS);
	if (rc < 0)
		TLS_HANDLE(readTLS, rc, rfd);
	
	if (useEarlyData)
		buf->unuse(earlyDataWritten);
		
}

void TLS::tlsAccept(StreamBuffer *buf)
{
	int earlyDataRead = 0;
	int rc = wolfSSL_read_early_data(readTLS, buf->getTail(), buf->usedSize(), &earlyDataRead);
	if (rc < 0)
		TLS_HANDLE(writeTLS, rc, rfd);
	
	buf->use(earlyDataRead);
}

size_t TLS::tlsWrite(StreamBuffer *buf)
{
	int bytes = wolfSSL_write(writeTLS, buf->getHead(), buf->usedSize());
	if (bytes < 0)
		TLS_HANDLE(writeTLS, bytes, wfd);
	
	buf->unuse(bytes);
	return bytes;
}

size_t TLS::tlsRead(StreamBuffer *buf)
{
	int bytes = wolfSSL_read(readTLS, buf->getTail(), buf->availSize());
	if (bytes < 0)
		TLS_HANDLE(readTLS, bytes, rfd);
	
	buf->unuse(bytes);
	return bytes;
}
