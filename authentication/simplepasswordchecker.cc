#include "simplepasswordchecker.hh"

using namespace std;

SimplePasswordChecker::SimplePasswordChecker(const std::pair<string_view, string_view> &credentials)
	: user(credentials.first), password(credentials.second)
{
	for (auto s: { user, password })
	{
		if (s.size() < 1 || s.size() > 255)
			throw length_error("Bad username or password length");
	}
}

bool SimplePasswordChecker::check(const std::pair<string_view, string_view> &credentials)
{
	auto [user, password] = credentials;
	return this->user == user && this->password == password;
}
