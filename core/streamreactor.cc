#include <unistd.h>
#include "streamreactor.hh"

StreamReactor::~StreamReactor()
{
	if (srcFD != -1)
		close(srcFD);
	if (dstFD != -1)
		close(dstFD);
}
