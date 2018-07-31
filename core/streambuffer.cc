#include <string.h>
#include "streambuffer.hh"

using namespace std;

void StreamBuffer::prepend(uint8_t *stuff, uint8_t size)
{
	if (BUF_SIZE - usedSize() < size)
		throw NoRoomException();
	if (size > head)
		memmove(&buf[size], &buf[head], usedSize());
	memcpy(&buf[head - size], stuff, size);
	head = head - size;
}

const char *StreamBuffer::NoRoomException::what() const throw()
{
	return "No room in stream buffer";
}
