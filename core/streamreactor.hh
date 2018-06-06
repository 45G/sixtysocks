#ifndef STREAMREACTOR_HH
#define STREAMREACTOR_HH

#include "reactor.hh"

class StreamBuffer
{
	static const size_t BUF_SIZE = 1024 * 1024; //1MB
	
	uint8_t buf[BUF_SIZE];
	
	size_t head;
	size_t tail;
	
public:
	StreamBuffer()
		: head(0), tail(0) {}
	
	uint8_t *getHead()
	{
		return &buf[head];
	}
	
	size_t usedSize() const
	{
		return tail - head;
	}
	
	void free(size_t count)
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
	
	size_t freeSize() const
	{
		return BUF_SIZE - tail;
	}
	
	void use(size_t count)
	{
		tail += count;
	}
};

class StreamReactor: public Reactor
{
protected:
	int srcFD;
	int dstFD;
	
	StreamBuffer buf;
	
public:
	StreamReactor(int srcFD, int dstFD)
		: srcFD(srcFD), dstFD(dstFD) {}
	
	int getSrcFD() const
	{
		return srcFD;
	}
	
	int getDstFD() const
	{
		return dstFD;
	}
	
	~StreamReactor();
};

#endif // STREAMREACTOR_HH
