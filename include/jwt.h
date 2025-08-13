#pragma once

#include <string>
using namespace std;

namespace JWT {
const string generateToken(const string& user_id);
bool verifyJWTToken(const std::string &token);
}