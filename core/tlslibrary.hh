#ifndef TLSLIBRARY_HH
#define TLSLIBRARY_HH

#include <string>
#include <boost/smart_ptr/intrusive_ref_counter.hpp>

class TLSLibrary: public boost::intrusive_ref_counter<TLSLibrary>
{
	struct NSPRLibrary
	{
		NSPRLibrary();
		
		~NSPRLibrary();
	};

	struct NSSLibrary
	{
		NSSLibrary();
		
		//TODO: get rid of this
		NSSLibrary(const std::string &configDir);

		~NSSLibrary();
	};

	NSPRLibrary nsprLibrary;
	NSSLibrary nssLibrary;

public:
	TLSLibrary(const std::string &configDir);
};

#endif // TLSLIBRARY_HH
