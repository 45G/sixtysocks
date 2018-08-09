#ifndef TLS_HH
#define TLS_HH

#include <boost/smart_ptr/intrusive_ref_counter.hpp>
#include <wolfssl/options.h>
#include <wolfssl/ssl.h>
#include <socks6util/socks6util.hh>
#include "streambuffer.hh"

class TLS: public boost::intrusive_ref_counter<TLS>
{
	int rfd;
	int wfd;
	
	WOLFSSL *readTLS;
	WOLFSSL *writeTLS;
	
	bool connected;
	
public:
	TLS(WOLFSSL_CTX *ctx, int fd);
	
	~TLS();
	
	void setReadFD(int fd);
	
	void setWriteFD(int fd);
	
	void tlsConnect(S6U::SocketAddress *addr, StreamBuffer *buf, bool useEarlyData);
	
	void tlsAccept(StreamBuffer *buf);
	
	size_t tlsWrite(StreamBuffer *buf);
	
	size_t tlsRead(StreamBuffer *buf);
};

#endif // TLS_HH
