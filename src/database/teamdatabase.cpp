#include "teamdatabase.h"
#include "global.h"
#include <ctime>
#include <iostream>
#define redis Global::db

using namespace std;
/*
{
  "team:id:counter": "1",
  "team:1": {
    "name": "FRC 334",
    "created_at": "1781281",
  },
  "teams:all": ("FRC 334"),
  "teams:by_created": ("FRC 334"),
  "team:name:FRC 334": "1",
  "team:1:members": ("alice"),
  "user:42:teams": ("FRC 334"),
  "team:1:coaches": ("Ms.D", "Mr.C")
}
*/

// HELP OF AI
namespace TeamDB {
long long createTeam(const string &team_name) {
  // Step 1: Generate a new team ID by incrementing the value at the key
  // "team:id:counter"
  long long team_id = redis.incr("team:id:counter");

  // Step 2: Store team metadata
  string team_key = "team:" + to_string(team_id);
  string team_pointer =  "team:name:" + team_name;

  /* redis.hset("user:100", "name", "Alice"); */
  /* Equivalent to: user["name"] = "Alice"; */
  unordered_map<string, string> m = {{"name", team_name},
                                     {"created_at", to_string(time(nullptr))}};
  redis.hmset("hash", m.begin(), m.end());
  redis.set(team_pointer, to_string(team_id));

  // Step 3: Add to tracking set
  redis.sadd("teams:all", to_string(team_id));

  // Add to sorted set by timestamp
  redis.zadd("teams:by_created", to_string(team_id), time(nullptr));

  cout << "Created team '" << team_name << "' with ID " << team_id << endl;

  return team_id;
}

bool teamExists(long long team_id) {
  string team_key = "team:" + to_string(team_id);
  return redis.exists(team_key) == 1;
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
} // namespace TeamDB