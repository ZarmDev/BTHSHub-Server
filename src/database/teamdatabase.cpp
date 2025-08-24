#include "teamdatabase.h"
#include "global.h"
#include <ctime>
#include <iostream>
#define redis Global::db

using namespace std;
/*
Basics
{
  "team:id:counter": "1",
  "team:1": {
    "name": "FRC 334",
    "created_at": "1781281",
  },
  "teamname:FRC 334": "1",
  "team:1:annoucements": ("1", "2", "3")
  "teams:all": ("FRC 334"),
  "teams:by_created": ("FRC 334"),
  "team:1:members": ("alice"),
  "team:1:coaches": ("Ms.D", "Mr.C"),
  "user:42:teams": ("FRC 334"),
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

// HELP OF AI
namespace TeamDB {
long long createTeam(const string &team_name) {
  // Step 1: Generate a new team ID by incrementing the value at the key
  // "team:id:counter"
  long long team_id = redis.incr("team:id:counter");

  // Step 2: Store team metadata
  const string teamHashKey = "team:" + to_string(team_id);
  const string teamPointerKey =  "teamname:" + team_name;

  /* redis.hset("user:100", "name", "Alice"); */
  /* Equivalent to: user["name"] = "Alice"; */
  unordered_map<string, string> m = {{"name", team_name},
                                     {"created_at", to_string(time(nullptr))}};
  redis.hmset(teamHashKey, m.begin(), m.end());
  redis.set(teamPointerKey, to_string(team_id));

  // Step 3: Add to tracking set
  // Use teams:all for unordered, fast set operations. Ex: Find team exists, getting list of all teams
  redis.sadd("teams:all", to_string(team_id));

  // Step 4: Add to sorted set by timestamp (less useful than above)
  // Use teams:by_created for ordered queries (by creation time). Ex: Show new teams
  redis.zadd("teams:by_created", to_string(team_id), time(nullptr));

  cout << "Created team '" << team_name << "' with ID " << team_id << endl;

  return team_id;
}

bool teamExists(long long team_id) {
  string team_key = "team:" + to_string(team_id);
  return redis.exists(team_key) == 1;
}

bool teamExistsByName(const string& team_name) {
  // redis.exists will return 1 if the key exists
  return redis.exists("teamname:" + team_name) == 1;
}

unordered_set<string> getAllTeams() {
  unordered_set<string> set;
  redis.smembers("teams:all", inserter(set, set.begin()));
  return set;
}

unordered_map<string, string> getTeamInfo(long long team_id) {
  string team_key = "team:" + to_string(team_id);
  unordered_map<string, string> hash;
  redis.hgetall("hash", inserter(hash, hash.end()));
  return hash;
}

// void register_team_name(Redis& redis, const string& team_name, long long
// team_id) {
//     redis.set("team:name:" + team_name, to_string(team_id));
// }

optional<string> getTeamIdByName(const string &team_name) {
  auto val = redis.get("team:name:" + team_name);
  if (val)
    return *val;
  return nullopt;
}

/*
content: content in markdown form
userIDOwner: the owner of the annoucement (the corresponding user ID)
mentions: an array of the people who are mentioned in the post (get notifications). If empty, it's assumed that it's a default annoucement where EVERYONE will be notified
*/
void postAnnoucement(const string& teamName, const string& content, const string& userIDOwner, vector<string> mentions) {
  const string teamKey = teamName + ":annoucement";
  long long annoucement_id = redis.incr(teamKey + ":id:counter");
  unordered_map<string, string> m = {{"content", content},
                                          {"owner", userIDOwner},
                                     {"created_at", to_string(time(nullptr))}};
  redis.hmset("hash", m.begin(), m.end());
  redis.sadd(teamKey + ":mentions", mentions.begin(), mentions.end());
}
} // namespace TeamDB

void getAnnoucementLength() {

}

/*
start: put the first annoucement you want to get
*/
void getRangeOfAnnoucements(int start, int end) {

}