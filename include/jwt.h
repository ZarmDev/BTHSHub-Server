#pragma once

#include <string>
using namespace std;

namespace JWT {
const string generateToken(const string& user_id);
bool verifyJWTToken(const string &token);
const string getUserIdFromToken(const string& token);
}