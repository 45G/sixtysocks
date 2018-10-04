#ifndef TLS_HH
#define TLS_HH

#include <boost/smart_ptr/intrusive_ref_counter.hpp>
#include <socks6util/socks6util.hh>
#include <ssl.h>
#include <prio.h>
#include <private/pprio.h>
#include "tlscontext.hh"
#include "streambuffer.hh"

class Proxifier;

class TLS: public boost::intrusive_ref_counter<TLS>
{
	struct PRTCPLayer: public PRFileDesc
	{
		static void     PR_CALLBACK destructor      (PRFileDesc *fd);

		static PRStatus PR_CALLBACK dClose          (PRFileDesc *fd);
		static PRInt32  PR_CALLBACK dRecv           (PRFileDesc *fd, void *buf, PRInt32 amount, PRIntn flags, PRIntervalTime timeout);
		static PRInt32  PR_CALLBACK dRead           (PRFileDesc *fd, void *buf, PRInt32 amount);
		static PRInt32  PR_CALLBACK dSend           (PRFileDesc *fd, const void *buf, PRInt32 amount, PRIntn flags, PRIntervalTime timeout);
		static PRInt32  PR_CALLBACK dWrite          (PRFileDesc *fd, const void *buf, PRInt32 amount);
		static PRStatus PR_CALLBACK dConnectContinue(PRFileDesc *fd, PRInt16 outFlags);
		static PRStatus PR_CALLBACK dGetName        (PRFileDesc *fd, PRNetAddr *addr);
		static PRStatus PR_CALLBACK dGetPeerName    (PRFileDesc *fd, PRNetAddr *addr);

		int rfd;
		int wfd;

		S6U::SocketAddress addr;
		bool attemptSendTo;

		PRTCPLayer(int fd);

		void setReadFD(int rfd)
		{
			this->rfd = rfd;
		}

		int getReadFD() const
		{
			return rfd;
		}

		void setWriteFD(int wfd)
		{
			this->wfd = wfd;
		}

		int getWriteFD() const
		{
			return wfd;
		}

		void setConnectAddr(const S6U::SocketAddress &addr)
		{
			this->addr = addr;
			attemptSendTo = true;
		}
	};

	static void descriptorDeleter(PRFileDesc *fd)
	{
		PR_Close(fd); //might return error
	}

	PRTCPLayer *bottomLayer;
	std::unique_ptr<PRFileDesc, void (*)(PRFileDesc *)> descriptor;

	bool connectCalled;
	bool handshakeFinished;

	static void handshakeCallback(PRFileDesc *fd, void *clientData);

	static SECStatus canFalseStartCallback(PRFileDesc *fd, void *arg, PRBool *canFalseStart);

public:
	TLS(TLSContext *ctx, int fd);
	
	~TLS();
	
	void setReadFD(int fd);
	
	void setWriteFD(int fd);
	
	void tlsConnect(S6U::SocketAddress *addr, StreamBuffer *buf, bool useEarlyData);
	
	void tlsAccept(StreamBuffer *buf);
	
	size_t tlsWrite(StreamBuffer *buf);
	
	size_t tlsRead(StreamBuffer *buf);
};

#endif // TLS_HH
