#ifndef UNIQFD_H
#define UNIQFD_H

#include <unistd.h>
#include <errno.h>
#include <system_error>
#include <sys/socket.h>
#include <assert.h>

class UniqSendFD;
class UniqRecvFD;

class UniqFD
{
protected:
	int fd;
	
public:
	UniqFD()
		: fd(-1) {}
	
	explicit UniqFD(int fd)
		: fd(fd) {}
	
	UniqFD(const UniqFD &other) = delete;
	
	UniqFD(UniqFD &&other)
		: fd(other.fd)
	{
		other.fd = -1;
	}
	
	void operator =(const UniqFD &other) = delete;

	void assign(int fd)
	{
		assert(this->fd == -1);
		
		this->fd = fd;
	}
	
	operator int() const
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
	
	friend class UniqSendFD;
	friend class UniqRecvFD;
};

class UniqRecvFD: public UniqFD
{
public:
	using UniqFD::UniqFD;
	
	UniqRecvFD &operator =(UniqFD &&other)
	{
		assert(this != &other);
		assert(fd == -1);
		
		fd = other.fd;
		other.fd = -1;
		return *this;
	}

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
public:
	using UniqFD::UniqFD;
	
	UniqSendFD &operator =(UniqFD &&other)
	{
		assert(this != &other);
		assert(fd == -1);
		
		fd = other.fd;
		other.fd = -1;
		return *this;
	}

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
