#ifndef SIMPLEPASSWORDCHECKER_HH
#define SIMPLEPASSWORDCHECKER_HH

#include "passwordchecker.hh"

class SimplePasswordChecker: public PasswordChecker
{
	const std::string user;
	const std::string password;
	
public:
	SimplePasswordChecker(const std::string &user, const std::string &password);
	
	bool check(std::pair<const std::string *, const std::string *> creds);
};

#endif // SIMPLEPASSWORDCHECKER_HH
