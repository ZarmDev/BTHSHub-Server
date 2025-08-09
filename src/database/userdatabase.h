#pragma once

#include <sw/redis++/redis++.h>
#include <string>
#include <optional>
#include <unordered_map>
#include <unordered_set>

using namespace std;

namespace UserDB {  
    void createUser(const string &username, const string &email);
    void addUserToTeam(long long user_id, long long team_id);
    string handle_login(const string &username, const string &password);
};