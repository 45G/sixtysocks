#include <stdexcept>
#include <boost/filesystem.hpp>
#include "tlsexception.hh"
#include "tlscontext.hh"

using namespace std;

TLSContext::TLSContext(const std::string &veriFile)
	: session(NULL), server(false)
{
	ctx = wolfSSL_CTX_new(wolfTLSv1_3_client_method());
	if (ctx == NULL)
		throw runtime_error("Can't initialize WolfSSL context");
	
	try
	{
		int rc;
		
		boost::filesystem::path resolved = boost::filesystem::canonical(veriFile);
		if (boost::filesystem::is_directory(resolved))
			rc = wolfSSL_CTX_load_verify_locations(ctx, NULL, veriFile.c_str());
		else
			rc = wolfSSL_CTX_load_verify_locations(ctx, veriFile.c_str(), NULL);
		if (rc != SSL_SUCCESS)
			throw TLSException(rc);
	}
	catch (std::exception &ex)
	{
		wolfSSL_CTX_free(ctx);
		throw ex;
	}
}

TLSContext::TLSContext(const string &certFile, const string keyFile)
	: session(NULL), server(true)
{
	ctx = wolfSSL_CTX_new(wolfTLSv1_3_server_method());
	if (ctx == NULL)
		throw runtime_error("Can't initialize WolfSSL context");
	
	try
	{
		int rc;
		
		rc = wolfSSL_CTX_use_certificate_file(ctx, certFile.c_str(), SSL_FILETYPE_PEM);
		if (rc != SSL_SUCCESS)
			throw TLSException(rc);
	
		rc = wolfSSL_CTX_use_PrivateKey_file(ctx, keyFile.c_str(), SSL_FILETYPE_PEM);
		if (rc != SSL_SUCCESS)
			throw TLSException(rc);
	}
	catch (std::exception &ex)
	{
		wolfSSL_CTX_free(ctx);
		throw ex;
	}
}
