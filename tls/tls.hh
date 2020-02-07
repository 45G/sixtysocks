#ifndef TLS_HH
#define TLS_HH

#include <socks6util/socks6util.hh>
#include <ssl.h>
#include <prio.h>
#include <private/pprio.h>
#include "tlscontext.hh"
#include "../core/streambuffer.hh"

class Proxifier;

class TLS
{
	static PRStatus PR_CALLBACK dClose          (PRFileDesc *fd) noexcept;
	static PRInt32  PR_CALLBACK dRecv           (PRFileDesc *fd, void       *buf, PRInt32 amount, PRIntn flags, PRIntervalTime timeout) noexcept;
	static PRInt32  PR_CALLBACK dSend           (PRFileDesc *fd, const void *buf, PRInt32 amount, PRIntn flags, PRIntervalTime timeout) noexcept;
	static PRInt32  PR_CALLBACK dRead           (PRFileDesc *fd, void       *buf, PRInt32 amount) noexcept;
	static PRInt32  PR_CALLBACK dWrite          (PRFileDesc *fd, const void *buf, PRInt32 amount) noexcept;
	static PRStatus PR_CALLBACK dGetName        (PRFileDesc *fd, PRNetAddr *addr) noexcept;
	static PRStatus PR_CALLBACK dGetPeerName    (PRFileDesc *fd, PRNetAddr *addr) noexcept;
	static PRStatus PR_CALLBACK dGetSocketOption(PRFileDesc *fd, PRSocketOptionData *data) noexcept;
	
	static const PRIOMethods METHODS;
	
	enum HandshakeState
	{
		S_WANT_EARLY,
		S_WANT_HANDSHAKE,
		S_LAISEZ_FAIRE,
	};

	HandshakeState state { S_WANT_EARLY };
	ssize_t earlyWritten = 0;

	int readFD;
	int writeFD;

	struct UniqPRFileDesc: public std::unique_ptr<PRFileDesc, PRStatus (*)(PRFileDesc *)>
	{
		using unique_ptr::unique_ptr;

		UniqPRFileDesc(PRFileDesc *d) noexcept
			: unique_ptr(d, PR_Close) {}

		UniqPRFileDesc() noexcept
			: unique_ptr(nullptr, PR_Close) {}
	};

	UniqPRFileDesc TLSImport(UniqPRFileDesc &&lower)
	{
		UniqPRFileDesc higher { SSL_ImportFD(nullptr, lower.get()) };
		if (!higher)
			throw TLSException();

		/* woop woop woop double ownership! lower is now owned by higher */
		lower.release();

		return higher;
	}

	UniqPRFileDesc descriptor;

public:
	TLS(TLSContext *ctx, int fd);
	
	void setReadFD(int fd)
	{
		this->readFD = fd;
	}
	
	void setWriteFD(int fd)
	{
		this->writeFD = fd;
	}
	
	void tlsDisableEarlyData();

	void clientHandshake(StreamBuffer *buf);
	
	size_t tlsWrite(StreamBuffer *buf);
	
	size_t tlsRead(StreamBuffer *buf);
};

#endif // TLS_HH
