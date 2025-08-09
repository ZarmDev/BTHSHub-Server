#include "../global.h"
#include "./jwt.h"
#define redis Global::db

using namespace std;

namespace UserDB {
void createUser(const string &username, const string &email) {
  string user_key = "user:" + username;
  string email_key = "email:" + email;
  // Before creating a user
  if (redis.exists(user_key) || redis.exists(email_key)) {
    cout << "Error: Username or email already exists." << endl;
    return;
  }

  unordered_map<string, string> hash = {
      {"username", username},
      {"email", email},
      {"created_at", to_string(time(nullptr))}};
  redis.hmset(user_key, hash.begin(), hash.end());

  redis.set("username:" + username, to_string(user_id));
  redis.set("email:" + email, to_string(user_id));

  redis.sadd("users:all", to_string(user_id));
  redis.zadd("users:by_created", to_string(user_id), time(nullptr));

  cout << "Created user '" << username << "' with ID " << user_id << endl;
}

// Add an existing user to an existing team
void addUserToTeam(long long user_id, long long team_id) {
  string team_members_key = "team:" + to_string(team_id) + ":members";
  string user_teams_key = "user:" + to_string(user_id) + ":teams";

  // Add user to team's member set
  redis.sadd(team_members_key, to_string(user_id));

  // Add team to user's team set
  redis.sadd(user_teams_key, to_string(team_id));

  // Optionally, track join time
  double now = time(nullptr);
  redis.zadd(team_members_key + ":by_joined", to_string(user_id), now);
  redis.zadd(user_teams_key + ":by_joined", to_string(user_id), now);

  cout << "Added user " << user_id << " to team " << team_id << endl;
}

string handle_login(const string &username, const string &password) {
  // Step 1: Lookup user by username
  OptionalString user_id_opt = redis.get("user:username:" + username);
  if (!user_id_opt) {
    // empty string means invalid username or password
    return "";
  }

  string user_id = *user_id_opt;
  cout << user_id << '\n';
  unordered_map<string, string> user_data;
  redis.hgetall("user:" + user_id, inserter(user_data, user_data.begin()));
  string stored_hash = user_data["password_hash"];

  // Step 2: Verify password
  if (!JWT::verify_password(password, stored_hash)) {
    return "";
  }

  // Step 3: Generate JWT
  string token = JWT::generate_token(user_id);

  // Step 4: Return token
  return token;
}
} // namespace UserDB