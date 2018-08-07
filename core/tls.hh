#ifndef TLS_HH
#define TLS_HH

#include <boost/smart_ptr/intrusive_ref_counter.hpp>
#include <wolfssl/options.h>
#include <wolfssl/ssl.h>
#include <socks6util/socks6util.hh>
#include "streambuffer.hh"
#include "uniqtls.hh"

class TLS: public boost::intrusive_ref_counter<TLS>
{
	int rfd;
	int wfd;
	
	UniqTLS readTLS;
	UniqTLS writeTLS;
	
public:
	TLS(WOLFSSL_CTX *ctx, int fd);
	
	void setReadFD(int fd)
	{
		rfd = fd;
	}
	
	void setWriteFD(int fd)
	{
		wfd = fd;
	}
	
	void tlsConnect(S6U::SocketAddress addr, StreamBuffer *buf, bool useEarlyData);
	
	void tlsAccept();
	
	void tlsWrite(StreamBuffer *buf);
	
	void tlsRead(StreamBuffer *buf);
};

#endif // TLS_HH
