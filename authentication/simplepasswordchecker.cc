#include "simplepasswordchecker.hh"

using namespace std;

SimplePasswordChecker::SimplePasswordChecker(const std::string &user, const std::string &password)
	: user(user), password(password)
{
	if (    user.size()     < 1 || user.size()     > 255 ||
		password.size() < 1 || password.size() > 255)
	{
		//TODO: proper exception
		throw length_error("Bad username or password length");
	}
}

bool SimplePasswordChecker::check(std::shared_ptr<string> user, std::shared_ptr<string> password)
{
	if (!user)
		return false;
	return this->user == *user && this->password == *password;
}
