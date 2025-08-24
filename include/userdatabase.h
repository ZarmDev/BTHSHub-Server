#pragma once

#include <sw/redis++/redis++.h>
#include <string>
#include "global.h"

using namespace std;

namespace UserDB {  
    bool createUser(const string &username, const string &password, const string &email);
    void addUserToTeam(long long user_id, long long team_id);
    string handleLogin(const string &username, const string &password);
    bool grantAdminLevel(const string& username, string level);
    void storeScheduleInRedis(const vector<Day>& schedule);
    void printUserHash(const string& user_id);
};