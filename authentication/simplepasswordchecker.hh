#ifndef SIMPLEPASSWORDCHECKER_HH
#define SIMPLEPASSWORDCHECKER_HH

#include "passwordchecker.hh"

class SimplePasswordChecker: public PasswordChecker
{
	const std::string user;
	const std::string password;
	
public:
	SimplePasswordChecker(const std::string &user, const std::string &password);
	
	bool check(const std::string *user, const std::string *password);
};

#endif // SIMPLEPASSWORDCHECKER_HH
