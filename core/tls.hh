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

extern PRIntn     _PR_InvalidInt(void);
extern PRInt16    _PR_InvalidInt16(void);
extern PRInt64    _PR_InvalidInt64(void);
extern PRStatus   _PR_InvalidStatus(void);
extern PRFileDesc *_PR_InvalidDesc(void);

class TLS: public boost::intrusive_ref_counter<TLS>
{
	struct PRTCPLayer: public PRFileDesc
	{
		static void PR_CALLBACK destructor(PRFileDesc *fd);

		static PRStatus PR_CALLBACK dClose(PRFileDesc *fd);

		static PRInt32 PR_CALLBACK dRecv(PRFileDesc *fd, void *buf, PRInt32 amount, PRIntn flags, PRIntervalTime timeout);

		static PRInt32 PR_CALLBACK dRead(PRFileDesc *fd, void *buf, PRInt32 amount);

		static PRInt32 PR_CALLBACK dSend(PRFileDesc *fd, const void *buf, PRInt32 amount, PRIntn flags, PRIntervalTime timeout);

		static PRInt32 PR_CALLBACK dWrite(PRFileDesc *fd, const void *buf, PRInt32 amount);

		static PRStatus PR_CALLBACK dConnectContinue(PRFileDesc *fd, PRInt16 outFlags);

		static PRStatus PR_CALLBACK dGetName(PRFileDesc *fd, PRNetAddr *addr);

		static PRStatus PR_CALLBACK dGetPeerName(PRFileDesc *fd, PRNetAddr *addr);

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

		PRTCPLayer(int fd)
			: rfd(fd), wfd(fd), attemptSendTo(false)
		{
			methods = &METHODS;
			secret = NULL;
			lower = NULL;
			higher = NULL;
			dtor = destructor;
			identity = PR_NSPR_IO_LAYER;
		}

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
