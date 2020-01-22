#ifndef SIMPLEPASSWORDCHECKER_HH
#define SIMPLEPASSWORDCHECKER_HH

#include "passwordchecker.hh"

class SimplePasswordChecker: public PasswordChecker
{
	const std::string user;
	const std::string password;
	
public:
	SimplePasswordChecker(const std::pair<std::string_view, std::string_view> &credentials);
	
	bool check(const std::pair<std::string_view, std::string_view> &credentials);
};

#endif // SIMPLEPASSWORDCHECKER_HH
