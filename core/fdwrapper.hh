#ifndef FDWRAPPER_HH
#define FDWRAPPER_HH

#include <unistd.h>
#include <sys/socket.h>

struct FDWrapper
{
	int fd;

	FDWrapper()
		: fd(-1) {}

	FDWrapper(int fd)
		: fd(fd) {}

	~FDWrapper()
	{
		if (fd != -1)
			close(fd);
	}
};

struct SendFDWrapper;

struct RecvFDWrapper: public FDWrapper
{
	RecvFDWrapper(SendFDWrapper sender);

	~RecvFDWrapper()
	{
		if (fd != -1)
			shutdown(fd, SHUT_RD);
	}
};

struct SendFDWrapper: public FDWrapper
{
	SendFDWrapper(RecvFDWrapper recver);

	~SendFDWrapper()
	{
		if (fd != -1)
			shutdown(fd, SHUT_RD);
	}
};

#endif // FDWRAPPER_HH
