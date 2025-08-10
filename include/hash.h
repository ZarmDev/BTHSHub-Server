#pragma once;

using namespace std;

namespace Hash {
string hashPassword(const string& password);
bool verifyPassword(const string& inputPassword, const string& hashedPassword);
}