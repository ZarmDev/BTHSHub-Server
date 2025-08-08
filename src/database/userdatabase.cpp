#include "../server.h"
#define redis Global::db

using namespace std;

void create_user(const string& username, const string& email) {
  long long user_id = redis.incr("user:id:counter");

  string user_key = "user:" + to_string(user_id);
  unordered_map<string, string> hash = {
    {"username", username},
    {"email", email},
    {"created_at", to_string(time(nullptr))}
  };
  redis.hmset(user_key, hash.begin(), hash.end());

  redis.sadd("users:all", to_string(user_id));
  redis.zadd("users:by_created", to_string(user_id), time(nullptr));

  cout << "Created user '" << username << "' with ID " << user_id << endl;
}

// Add an existing user to an existing team
void add_user_to_team(long long user_id, long long team_id) {
  string team_members_key = "team:" + to_string(team_id) + ":members";
  string user_teams_key    = "user:" + to_string(user_id) + ":teams";

  // Add user to team's member set
  redis.sadd(team_members_key, to_string(user_id));

  // Add team to user's team set
  redis.sadd(user_teams_key, to_string(team_id));

  // Optionally, track join time
  double now = time(nullptr);
  redis.zadd(team_members_key + ":by_joined", to_string(user_id), now);
  redis.zadd(user_teams_key + ":by_joined", to_string(user_id), now);

  cout << "Added user " << user_id
            << " to team " << team_id << endl;
}