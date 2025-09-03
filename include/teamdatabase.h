#pragma once

#include <sw/redis++/redis++.h>
#include <string>
#include <optional>
#include <unordered_map>
#include <unordered_set>
#include <nlohmann/json.hpp>

using namespace std;
using namespace sw::redis;

namespace TeamDB {
    const string& createTeam(const string &teamName, const string& isPrivate, const string& userID);
    bool teamExistsById(const string& teamId);
    unordered_set<string> getAllTeams();
    unordered_map<string, string> getTeamInfo(const string& teamId);
    bool teamExistsByName(const string& team_name);
    int getNumOfAnnoucements(string& teamName);
    nlohmann::json getRangeOfAnnoucements(const string& teamName, int start, int end);
    void postAnnoucement(const string& teamName, const string& content, const string& userIDOwner, vector<string> mentions);
    OptionalString getTeamIDFromName(const string& teamName);
    bool addUserToTeam(const string& user_id, const string& team_id, bool bypassPrivate);
    bool addOtherUserToTeam(const string& userInvitingID, const string& userBeingInvitedID, const string& team_id);
    bool userIsOnTeam(const string& teamName, const string& username);
    unordered_set<string> getUserTeams(const string& userID);
    unordered_set<string> getTeamMembers(const string& teamID);
    unordered_set<string> getTeamOwners(const string& teamID);
};