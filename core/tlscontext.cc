#include <stdexcept>
#include "tlsexception.hh"
#include "tlscontext.hh"
#include <boost/filesystem.hpp>

using namespace std;

/* client */
TLSContext::TLSContext(boost::intrusive_ptr<TLSLibrary> tlsLibrary, const std::string &veriFile)
	: tlsLibrary(tlsLibrary), server(false)
{
	//TODO
//	boost::filesystem::path resolved = boost::filesystem::canonical(veriFile);
//	if (boost::filesystem::is_directory(resolved))
//		...
//	else
//		...
}

/* server */
TLSContext::TLSContext(boost::intrusive_ptr<TLSLibrary> tlsLibrary, const string &certFile, const string keyFile)
	: tlsLibrary(tlsLibrary), server(true)
{
	//TODO
}

TLSContext::~TLSContext() {}
