#ifndef SOCKIO_HH
#define SOCKIO_HH

#include <unistd.h>
#include "streambuffer.hh"
#include "uniqfd.hh"


size_t sockFill(UniqFD *fd, StreamBuffer *buf);

size_t sockSpill(UniqFD *fd, StreamBuffer *buf);

#endif // SOCKIO_HH
