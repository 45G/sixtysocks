#include <unistd.h>
#include "streamreactor.hh"

StreamReactor::~StreamReactor()
{
	if (srcFD != -1)
	{
		shutdown(srcFD, SHUT_RD);
		close(srcFD);
	}
	if (dstFD != -1)
	{
		shutdown(dstFD, SHUT_WR);
		close(dstFD);
	}
}
