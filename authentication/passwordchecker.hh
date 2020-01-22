#ifndef PASSWORDCHECKER_HH
#define PASSWORDCHECKER_HH

#include <string>
#include <memory>

class PasswordChecker
{
public:
	virtual bool check(const std::pair<std::string_view, std::string_view> &credentials) = 0;
	
	virtual ~PasswordChecker() = default;
};

#endif // PASSWORDCHECKER_HH
