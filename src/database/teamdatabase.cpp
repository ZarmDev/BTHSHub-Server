#include "teamdatabase.h"
#include "userdatabase.h"
#include "global.h"
#include "utils.h"
#include <ctime>
#include <iostream>
#include <nlohmann/json.hpp>
#include <string>
#include <sw/redis++/utils.h>

#define redis Global::db

using namespace std;

/*
Basics
{
  "team:id:counter": "1",
  // Everything here should be public information (in the hash)
  "team:1": {
    "name": "FRC 334",
    "created_at": "1781281",
    "private": "false"
  },
  "teamname:FRC 334": "1",
  "team:1:annoucements": ("1", "2", "3")
  "teams:all": ("FRC 334"),
  "teams:by_created": ("FRC 334"),
  // members includes the coaches as well. so, on client side you have to check owners and members
  "team:1:members": ("alice", "Ms.D", "Mr.C"),
  "team:1:members:by_joined": ("alice", "Mr.C", "Ms.D"),
  "team:1:owners": ("Ms.D", "Mr.C"),
  "user:42:teams": ("FRC 334"),
  "user:42:teams:by_joined": ("FRC 334")
}
Annoucements
{
  "FRC 334:annoucement:id:counter": "1",
  "FRC 334:annoucement:1": {
    "content": "Hey everyone! Remember to come to the event on Saturday.",
    // the mentions will be specific groups with labels. Like say FRC 334 has the programmers, mechanics, then it will
    // send it to those with the label
    "owner": "42",
    "created_at": "1781281"
  }
  // store it seperately, because i dont want to serialize/parse
  "FRC 334:annoucement:mentions": ("programmers"),
}
*/

// This function ACTUALLY adds a user to the team. The other functions named similarly are just to check if there is valid permission
bool addUserToTeamHelper(const string& user_id, const string& team_id) {
  string team_members_key = "team:" + team_id + ":members";
  string user_teams_key = "user:" + user_id + ":teams";

  // Add user to team's member set
  redis.sadd(team_members_key, UserDB::getUsernameFromUserId(user_id));

  // Add team to user's team set
  redis.sadd(user_teams_key, team_id);

  // Optionally, track join time
  double now = time(nullptr);
  redis.zadd(team_members_key + ":by_joined", user_id, now);
  redis.zadd(user_teams_key + ":by_joined", user_id, now);

  cout << "Added user " << user_id << " to team " << team_id << endl;
  return true;
}

bool addOwnerToTeamHelper(const string& userID, const string& teamID) {
  redis.sadd("team:" + teamID + ":owners", UserDB::getUsernameFromUserId(userID));

  return true;
}

// HELP OF AI
namespace TeamDB {
bool addUserToTeam(const string& user_id, const string& team_id, bool bypassPrivate) {
  string team_key = "team:" + team_id;
  // First, check the level of permission the user has. A user can only add themselves to private teams if they are an admin
  string adminLevel = *redis.hget("user:" + user_id, "adminLevel");

  if (adminLevel != "2") {
    return false;
  }
  // Second, make sure that the team is public, if not you must be invited
  string isPrivate = *redis.hget(team_key, "private");
  if (isPrivate == "true" && !bypassPrivate) {
    return false;
  }

  addUserToTeamHelper(user_id, team_id);
  return true;
}

bool addOtherUserToTeam(const string& userInvitingID, const string& userBeingInvitedID, const string& team_id) {
  lock_guard<mutex> lock(Global::redisMutex);
  // Regardless of everything, we know the user exists because of the middleware but the team may not exist so we should make sure it exists
  if (!teamExistsById(team_id)) {
    cout << "Team does not EXIST!\n";
    return false;
  }
  if (userBeingInvitedID == userInvitingID) {
    cout << "Cannot invite yourself...\n";
    return false;
  }
  const string adminLevel = redis.hget("user:" + userInvitingID, "adminLevel").value();
  bool isTeamOwner = redis.sismember("team:" + team_id + ":owners", UserDB::getUsernameFromUserId(userInvitingID));
  // First case, regardless of the case, if you are an admin you can invite anyone
  bool firstCase = adminLevel == "2";
  // Second case, if you are a moderator, you are allowed to invite anyone as long as you own the team
  bool secondCase = adminLevel == "1" && isTeamOwner;
  // Only here, userBeingInvitedID should be used. If not, there is a problem with the function
  if (firstCase || secondCase) {
    bool addUserToTeamAttempt = addUserToTeamHelper(userBeingInvitedID, team_id);
    cout << addUserToTeamAttempt << " -> addUserToTeamAttempt\n";
    return addUserToTeamAttempt;
  }
  return false;
}

const string& createTeam(const string &teamName, const string& isPrivate, const string& userID) {
  if (teamName.contains("\\")) {
    return "";
  }

  lock_guard<mutex> lock(Global::redisMutex);

  // Step 1: Generate a new team ID by incrementing the value at the key
  // "team:id:counter"
  const string teamId = to_string(redis.incr("team:id:counter"));

  // Step 2: Store team metadata
  const string teamHashKey = "team:" + teamId;
  const string teamPointerKey =  "teamname:" + teamName;

  /* redis.hset("user:100", "name", "Alice"); */
  /* Equivalent to: user["name"] = "Alice"; */
  unordered_map<string, string> m = {{"name", teamName},
                                     {"created_at", to_string(time(nullptr))},
                                     {"private", isPrivate}};
  redis.hmset(teamHashKey, m.begin(), m.end());
  redis.set(teamPointerKey, teamId);

  // Step 3: Add to tracking set
  // Use teams:all for unordered, fast set operations. Ex: Find team exists, getting list of all teams
  redis.sadd("teams:all", teamName);

  // Step 4: Add to sorted set by timestamp (less useful than above)
  // Use teams:by_created for ordered queries (by creation time). Ex: Show new teams
  redis.zadd("teams:by_created", teamId, time(nullptr));

  cout << "Created team '" << teamName << "' with ID " << teamId << endl;

  // Step 5: Add user to the created team as a COACH AND MEMBER and bypass the private check
  addUserToTeamHelper(userID, teamId);
  addOwnerToTeamHelper(userID, teamId);

  auto t = getTeamInfo(teamId);
  printContainer(t);

  return teamId;
}

bool teamExistsById(const string& teamId) {
  string team_key = "team:" + teamId;
  return redis.exists(team_key) == 1;
}

bool teamExistsByName(const string& teamName) {
  // redis.exists will return 1 if the key exists
  return redis.exists("teamname:" + teamName) == 1;
}

// does not check if owner is on the team
bool userIsOnTeam(const string& teamName, const string& username) {
  OptionalString teamID = getTeamIDFromName(teamName);
  string teamKey = "team:" + teamID.value() + ":members";
  return redis.sismember(teamKey, username);
}

unordered_set<string> getAllTeams() {
  unordered_set<string> set;
  redis.smembers("teams:all", inserter(set, set.begin()));
  return set;
}

// get the teams OF a user
unordered_set<string> getUserTeams(const string& userID) {
  unordered_set<string> set;
  redis.smembers("user:" + userID + ":teams", inserter(set, set.begin()));
  return set;
}

unordered_set<string> getTeamMembers(const string& teamID) {
  unordered_set<string> set;
  redis.smembers("team:" + teamID + ":members", inserter(set, set.begin()));
  return set;
}

unordered_set<string> getTeamOwners(const string& teamID) {
  unordered_set<string> set;
  redis.smembers("team:" + teamID + ":owners", inserter(set, set.begin()));
  return set;
}

unordered_map<string, string> getTeamInfo(const string& teamId) {
  string team_key = "team:" + teamId;
  unordered_map<string, string> hash;
  redis.hgetall(team_key, inserter(hash, hash.end()));
  return hash;
}

OptionalString getTeamIDFromName(const string& teamName) {
  return redis.get("teamname:" + teamName);
}

// void register_teamName(Redis& redis, const string& teamName, long long
// teamId) {
//     redis.set("team:name:" + teamName, teamId);
// }

/*
content: content in markdown form
userIDOwner: the owner of the annoucement (the corresponding user ID)
mentions: an array of the people who are mentioned in the post (get notifications). If empty, it's assumed that it's a default annoucement where EVERYONE will be notified
*/
void postAnnoucement(const string& teamName, const string& content, const string& userIDOwner, vector<string> mentions) {
  lock_guard<mutex> lock(Global::redisMutex); // Lock during Redis operations
  
  const string teamKey = teamName + ":annoucement";
  long long annoucement_id = redis.incr(teamKey + ":id:counter");
  unordered_map<string, string> m = {{"content", content},
                                          {"owner", userIDOwner},
                                     {"created_at", to_string(time(nullptr))}};
  redis.hmset(teamKey + ":" + to_string(annoucement_id), m.begin(), m.end());
  redis.sadd(teamKey + ":mentions", mentions.begin(), mentions.end());
}

int getNumOfAnnoucements(string& teamName) {
  Optional counter = redis.get(teamName+":annoucement:id:counter");
  if (!counter) {
    return 0;
  }
  return stoi(*counter);
}

/*
start: put the first annoucement you want to get
*/
nlohmann::json getRangeOfAnnoucements(const string& teamName, int start, int end) {
  nlohmann::json arr = nlohmann::json::array();
  cout << teamName << " " << start << " " << end << '\n';
  for (int i = end; i >= start; i--) {
    string annoucementKey = teamName + ":annoucement:" + to_string(i);
    unordered_map<string, string> obj;
    redis.hgetall(annoucementKey, inserter(obj, obj.begin()));
    if (!obj.empty()) {
      // Convert to JSON object
      nlohmann::json j = obj;
      // Add to array
      arr.push_back(j);
    }
  }
  return arr;
}
} // namespace TeamDB