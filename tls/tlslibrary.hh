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
	
	TLSLibrary(const TLSLibrary &) = delete;
	
	auto operator =(const TLSLibrary &) = delete;
	auto operator =(TLSLibrary &&)      = delete;
};

#endif // TLSLIBRARY_HH
