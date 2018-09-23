#ifndef TLSLIBRARY_HH
#define TLSLIBRARY_HH

#include <string>
#include <boost/smart_ptr/intrusive_ref_counter.hpp>

class TLSLibrary: public boost::intrusive_ref_counter<TLSLibrary>
{
	struct NSPRLibrary
	{
		~NSPRLibrary();
	};

	struct NSSLibrary
	{
		NSSLibrary();

		~NSSLibrary();
	};

	struct Config
	{
		Config();
	};

	NSPRLibrary nspr;
	NSSLibrary nss;
	Config cfg;

public:
	TLSLibrary()
		: nss() {}
};

#endif // TLSLIBRARY_HH
