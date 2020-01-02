#ifndef STREAMBUFFER_HH
#define STREAMBUFFER_HH

#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <stdexcept>

class StreamBuffer
{
	static constexpr size_t BUF_SIZE = 35 * 1024; //100KB
	
	uint8_t buf[BUF_SIZE];
	
	size_t head = 0;
	size_t tail = 0;
	
public:
	uint8_t *getHead()
	{
		return &buf[head];
	}
	
	size_t usedSize() const
	{
		return tail - head;
	}
	
	void unuse(size_t count)
	{
		head += count;
		if (head == tail)
		{
			head = 0;
			tail = 0;
		}
	}
	
	uint8_t *getTail()
	{
		return &buf[tail];
	}
	
	size_t availSize() const
	{
		return BUF_SIZE - tail;
	}
	
	void use(size_t count)
	{
		tail += count;
	}
	
	void makeHeadroom(size_t size)
	{
		size_t dataSize = usedSize();
		
		if (BUF_SIZE - dataSize < size)
			throw std::runtime_error("No room in stream buffer");
		if (size > head)
		{
			memmove(&buf[size], &buf[head], dataSize);
			tail += size - head;
			head = size;
		}
	}
	
	void prepend(uint8_t *stuff, uint8_t size)
	{
		makeHeadroom(size);
		memcpy(&buf[head - size], stuff, size);
		head -= size;
	}
};

#endif // STREAMBUFFER_HH
