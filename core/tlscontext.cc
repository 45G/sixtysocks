#include <stdexcept>
#include "tlsexception.hh"
#include "tlscontext.hh"
#include <boost/filesystem.hpp>

using namespace std;

TLSContext::TLSContext(bool server)
	: server(server) {}

TLSContext::~TLSContext() {}
