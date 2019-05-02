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

bool SimplePasswordChecker::check(const string *user, const string *password)
{
	if (user == nullptr)
		return false;
	return this->user == *user && this->password == *password;
}
