#ifndef TLSLIBRARY_HH
#define TLSLIBRARY_HH

#include <string>

class TLSLibrary
{
	struct NSPRLibrary
	{
		NSPRLibrary();
		
		~NSPRLibrary();
	};

	struct NSSLibrary
	{
		NSSLibrary(const std::string &configDir);

		~NSSLibrary();
	};

	NSPRLibrary nsprLibrary;
	NSSLibrary  nssLibrary;

public:
	TLSLibrary(const std::string &configDir);
};

#endif // TLSLIBRARY_HH
