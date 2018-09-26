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
		
		NSSLibrary(const std::string &configDir);

		~NSSLibrary();
	};

	NSPRLibrary nsprLibrary;
	NSSLibrary nssLibrary;
	
	void init();

public:
	TLSLibrary();
	
	TLSLibrary(const std::string &configDir);
};

#endif // TLSLIBRARY_HH
