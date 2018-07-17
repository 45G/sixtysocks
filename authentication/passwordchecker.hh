#ifndef PASSWORDCHECKER_HH
#define PASSWORDCHECKER_HH

#include <string>

class PasswordChecker
{
public:
	virtual bool check(const std::string &user, const std::string &password) = 0;
	
	virtual ~PasswordChecker();
};

#endif // PASSWORDCHECKER_HH
