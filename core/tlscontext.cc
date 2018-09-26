#include <stdexcept>
#include "tlsexception.hh"
#include "tlscontext.hh"
#include <boost/filesystem.hpp>

using namespace std;

/* client */
TLSContext::TLSContext(const std::string &veriFile)
	: server(false)
{
	//TODO
//	boost::filesystem::path resolved = boost::filesystem::canonical(veriFile);
//	if (boost::filesystem::is_directory(resolved))
//		...
//	else
//		...
}

/* server */
TLSContext::TLSContext(const string &certFile, const string keyFile)
	: server(true)
{
	//TODO
}

TLSContext::~TLSContext() {}
