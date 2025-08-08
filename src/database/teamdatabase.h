#pragma once

#include <sw/redis++/redis++.h>
#include <string>
#include <optional>
#include <unordered_map>
#include <unordered_set>

using namespace std;
using namespace sw::redis;

class TeamDatabase {
public:
    // Constructor that takes a Redis connection string
    TeamDatabase();
    
    // Team operations
    long long createTeam(const string& team_name);
    bool teamExists(long long team_id);
    unordered_set<string> getAllTeams();
    unordered_map<string, string> getTeamInfo(long long team_id);
    optional<string> getTeamIdByName(const string& team_name);
    
    // Session operations (for your JWT functionality)
    void createSession(const string& token, const string& user_id, int expiry_seconds = 3600);
    optional<string> getSession(const string& token);
};