#ifndef UNIQFD_H
#define UNIQFD_H

#include <unistd.h>
#include <errno.h>
#include <system_error>

class UniqFD
{
	int fd;
	
public:
	UniqFD()
		: fd(-1) {}
	
	UniqFD(int fd)
		: fd(fd) {}
	
	UniqFD(UniqFD *other)
		: fd(other->fd)
	{
		other->fd = -1;
	}
	
	UniqFD(const UniqFD &) = delete;
	
	operator =(const UniqFD &) = delete;
	
//	ExFD duplicate()
//	{
//		int fd2 = dup(fd);
//		if (fd2 < 0)
//			throw system_error(errno, system_category());
		
//		return ExFD(fd);
//	}
	
	int operator int()
	{
		return fd;
	}
	
	~UniqFD()
	{
		if (fd != -1)
			close(fd);
	}
};

class ExRecvFD: public

#endif // UNIQFD_H
