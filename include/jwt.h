#pragma once

#include <string>
using namespace std;

namespace JWT {
const string generateToken(const string& user_id);
bool verifyPassword(const std::string& password, const std::string& stored_hash);
}