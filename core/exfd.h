#ifndef EXFD_H
#define EXFD_H

#include <unistd.h>
#include <errno.h>
#include <system_error>

class ExFD
{
	int fd;
	
public:
	ExFD()
		: fd(-1) {}
	
	ExFD(int fd)
		: fd(fd) {}
	
	ExFD(ExFD *other)
		: fd(other->fd)
	{
		other->fd = -1;
	}
	
	ExFD(const ExFD &) = delete;
	
	operator =(const ExFD &) = delete;
	
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
	
	~ExFD()
	{
		if (fd != -1)
			close(fd);
	}
};

class ExRecvFD: public

#endif // EXFD_H
