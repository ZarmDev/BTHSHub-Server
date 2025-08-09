#pragma once

#include <string>
using namespace std;

namespace JWT {
const string generate_token(const string& user_id);
bool verify_password(const std::string& password, const std::string& stored_hash);
}