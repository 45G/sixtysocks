#include <string.h>
#include "streambuffer.hh"

using namespace std;

void StreamBuffer::makeHeadroom(size_t size)
{
	size_t dataSize = usedSize();

	if (BUF_SIZE - dataSize < size)
		throw NoRoomException();
	if (size > head)
	{
		memmove(&buf[size], &buf[head], dataSize);
		tail += size - head;
		head = size;
	}
}

void StreamBuffer::prepend(uint8_t *stuff, uint8_t size)
{
	makeHeadroom(size);
	memcpy(&buf[head - size], stuff, size);
	head -= size;
}

const char *StreamBuffer::NoRoomException::what() const throw()
{
	return "No room in stream buffer";
}
