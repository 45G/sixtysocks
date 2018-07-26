#ifndef UNIQFD_H
#define UNIQFD_H

#include <unistd.h>
#include <errno.h>
#include <system_error>
#include <sys/socket.h>
#include <assert.h>

class UniqFD
{
protected:
	int fd;
	
public:
	UniqFD()
		: fd(-1) {}
	
	explicit UniqFD(int fd)
		: fd(fd) {}

	void assign(int fd)
	{
		assert(fd == -1);
		this->fd = fd;
	}
	
	operator int()
	{
		return fd;
	}
	
	void reset()
	{
		if (fd != -1)
		{
			close(fd);
			fd = -1;
		}
	}
	
	~UniqFD()
	{
		if (fd != -1)
			close(fd);
	}
};

class UniqRecvFD: public UniqFD
{
	void reset()
	{
		if (fd != -1)
		{
			shutdown(fd, SHUT_RD);
			close(fd);
			fd = -1;
		}
	}
	
	~UniqRecvFD()
	{
		if (fd != -1)
			shutdown(fd, SHUT_RD);
	}
};

class UniqSendFD: public UniqFD
{
	void reset()
	{
		if (fd != -1)
		{
			shutdown(fd, SHUT_WR);
			close(fd);
			fd = -1;
		}
	}
	
	~UniqSendFD()
	{
		if (fd != -1)
			shutdown(fd, SHUT_WR);
	}
};

#endif // UNIQFD_H
