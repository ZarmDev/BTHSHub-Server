#pragma once

#include <sw/redis++/redis++.h>
#include <string>
#include "global.h"

using namespace std;

namespace UserDB {  
    const string createUser(const string &username, const string &password, const string &email);
    void addUserToTeam(long long user_id, long long team_id);
    string handleLogin(const string &username, const string &password);
    bool grantAdminLevel(const string& username, string level);
    void storeScheduleInRedis(const vector<Day>& schedule);
    void printUserHash(const string& user_id);
    string getUserIdByUsername(const string& username);
    string getUsernameFromUserId(const string& userID);
    bool isUserAdmin(const string& userID);
    const string& getPermissionLevel(const string& userID);
};