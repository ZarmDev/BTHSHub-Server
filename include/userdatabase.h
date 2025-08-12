#pragma once

#include <sw/redis++/redis++.h>
#include <string>

using namespace std;

namespace UserDB {  
    bool createUser(const string &username, const string &password, const string &email);
    void addUserToTeam(long long user_id, long long team_id);
    string handle_login(const string &username, const string &password);
};