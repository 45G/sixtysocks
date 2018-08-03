#ifndef SOCKIO_HH
#define SOCKIO_HH

#include <unistd.h>
#include <socks6util/socks6util.hh>
#include "streambuffer.hh"
#include "uniqfd.hh"


size_t sockFill(UniqFD *fd, StreamBuffer *buf);

size_t sockSpill(UniqFD *fd, StreamBuffer *buf);

ssize_t sockSpillTFO(UniqFD *fd, StreamBuffer *buf, S6U::SocketAddress dest);

#endif // SOCKIO_HH
