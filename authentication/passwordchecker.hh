#ifndef PASSWORDCHECKER_HH
#define PASSWORDCHECKER_HH

#include <string>
#include <memory>

class PasswordChecker
{
public:
	virtual bool check(const std::string *user, const std::string *password) = 0;
	
	virtual ~PasswordChecker() = default;
};

#endif // PASSWORDCHECKER_HH
