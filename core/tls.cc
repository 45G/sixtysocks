#include <stdexcept>
#include "rescheduleexception.hh"
#include "tlsexception.hh"
#include "poller.hh"
#include "tls.hh"

using namespace std;

TLS::TLS(WOLFSSL_CTX *ctx, int fd)
	: rfd(fd), wfd(fd)
{
	readTLS.assign(wolfSSL_new(ctx));
	if (readTLS == NULL)
		throw runtime_error("Error creating context");
	wolfSSL_SetIOWriteFlags(readTLS, MSG_NOSIGNAL);
}

TLS::~TLS()
{
	wolfSSL_free(writeTLS);
	wolfSSL_free(readTLS);
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
	{
		int err = wolfSSL_get_error(writeTLS, rc);
		if (err == WOLFSSL_ERROR_WANT_READ)
			throw RescheduleException(wfd, Poller::IN_EVENTS);
		if (err == WOLFSSL_ERROR_WANT_WRITE)
			throw RescheduleException(wfd, Poller::OUT_EVENTS);
		throw TLSException(err);
	}
	
	if (useEarlyData)
		buf->unuseHead(earlyDataWritten);
		
}

void TLS::tlsAccept(StreamBuffer *buf)
{
	
}

size_t TLS::tlsWrite(StreamBuffer *buf)
{
	int bytes = wolfSSL_write(writeTLS, buf->getHead(), buf->usedSize());
	if (bytes < 0)
	{
		int err = wolfSSL_get_error(writeTLS, bytes);
		if (err == WOLFSSL_ERROR_WANT_WRITE)
			throw RescheduleException(wfd, Poller::OUT_EVENTS);
		if (err == WOLFSSL_ERROR_WANT_READ)
			throw RescheduleException(wfd, Poller::IN_EVENTS);
		throw TLSException(err);
	}
	buf->unuseHead(bytes);
	return bytes;
}

size_t TLS::tlsRead(StreamBuffer *buf)
{
	int bytes = wolfSSL_read(readTLS, buf->getTail(), buf->availSize());
	if (bytes < 0)
	{
		int err = wolfSSL_get_error(writeTLS, bytes);
		if (err == WOLFSSL_ERROR_WANT_READ)
			throw RescheduleException(wfd, Poller::IN_EVENTS);
		if (err == WOLFSSL_ERROR_WANT_WRITE)
			throw RescheduleException(wfd, Poller::OUT_EVENTS);
		throw TLSException(err);
	}
	buf->unuseHead(bytes);
	return bytes;
}
