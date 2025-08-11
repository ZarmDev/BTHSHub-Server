#include "global.h"
#include "jwt.h"
#include "hash.h"
#define redis Global::db

using namespace std;

/* How the database SHOULD look like: (say we just created alice)
// Redis keys and values
// Redis keys and values
{
  "user:42": {
    "username": "alice",
    "email": "alice@example.com",
    "created_at": "1723228800",
    "password_hash": "$2b$12$..."
  },
  "username:alice": "42",
  "email:alice@example.com": "42",
  "users:all": Set(["42", "43", ...]),
  "users:by_created": SortedSet({ "42": 1723228800, ... })
}
*/

namespace UserDB {
void createUser(const string &username, const string &password, const string &email) {
  string username_key = "username:" + username;
  string email_key = "email:" + email;

  // Check if username or email already exists
  if (redis.exists(username_key) || redis.exists(email_key)) {
    cout << "Error: Username or email already exists." << endl;
    return;
  }

  // Generate a new user ID (you can use an atomic counter or UUID)
  string user_id = to_string(redis.incr("user:id:counter"));
  string user_key = "user:" + user_id;

  // Hash the password securely
  string password_hash = Hash::hashPassword(password);

  // Store user metadata in a flat hash
  unordered_map<string, string> hash = {
      {"username", username},
      {"email", email},
      {"created_at", to_string(time(nullptr))},
      {"password_hash", password_hash},
      {"status", "offline"},
      {"profile_picture_url", ""},
      {"isAdmin", "false"}
  };
  redis.hmset(user_key, hash.begin(), hash.end());

  // Store user_tags (like "taking 3 APS" or "Stuyvesant reject")
  // string tags_key = user_key + ":tags";
  // This can be done later since we don't know what the user wants...

  // Index by username and email
  redis.set(username_key, user_id);
  redis.set(email_key, user_id);

  // Add to global user sets
  redis.sadd("users:all", user_id);
  // redis.zadd("users:by_created", user_id, time(nullptr));

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
  OptionalString user_id_opt = redis.get("user:" + username);
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