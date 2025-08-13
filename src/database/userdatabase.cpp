#include "global.h"
#include "hash.h"
#include "jwt.h"
#include "utils.h"
#define redis Global::db

using namespace std;

/* How the database SHOULD look like: (say we just created alice)
{
  "user:42": {
    "username": "alice",
    "email": "alice@example.com",
    "created_at": "1723228800",
    "password_hash": "$2b$12$...",
    "status": "offline",
    "profile_picture_url": "",
    "adminLevel": "0"
  },
  "username:alice": "42",
  "email:alice@example.com": "42",
  "users:all": Set(["42", "43", ...]),
  "users:by_created": SortedSet({ "42": 1723228800, ... })
}
NOTES:
adminLevel - 0 means no permissions, 1 means an app moderator, 2 means all permission admin
*/

namespace UserDB {
bool createUser(const string &username, const string &password,
                const string &email) {
  cout << password << '\n';
  // Used for the flat key
  string user_key = "user:" + username;
  string email_key = "email:" + email;

  // Check if username or email already exists
  if (redis.exists(user_key) || redis.exists(email_key)) {
    cout << "Error: Username or email already exists." << endl;
    return false;
  }

  // Generate a new user ID (you can use an atomic counter or UUID)
  string user_id = to_string(redis.incr("user:id:counter"));

  // Used for the hash
  string userhash_key = "userhash:" + user_id;

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
      {"adminLevel", "0"}};
  redis.hmset(userhash_key, hash.begin(), hash.end());

  // Store user_tags (like "taking 3 APS" or "Stuyvesant reject")
  // string tags_key = user_key + ":tags";
  // This can be done later since we don't know what the user wants...

  // Index by username and email
  redis.set(user_key, user_id);
  redis.set(email_key, user_id);

  // Add to global user sets
  redis.sadd("users:all", user_id);
  // redis.zadd("users:by_created", user_id, time(nullptr));

  cout << "Created user '" << username << "' with ID " << user_id << endl;
  return true;
}

string getUserId(const string& user_key) {
  OptionalString user_id_opt = redis.get(user_key);
  if (!user_id_opt) {
    // empty string means invalid username or password
    cout << user_key << " not found\n";
    return "";
  }

  string user_id = *user_id_opt;
  return user_id;
}

bool grantAdminLevel(const string& username, string level) {
  string user_key = "user:" + username;
  string user_id = getUserId(user_key);

  string userhash_key = "userhash:" + user_id;

  // Will update only adminLevel
  unordered_map<string, string> update = {
    {"adminLevel", level}
  };
  redis.hmset(userhash_key, update.begin(), update.end());
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
  // printContainer(users);
  // printAllRedisKeys();
  // cout << password << '\n';

  string user_key = "user:" + username;

  // Step 1: Lookup user by username
  string user_id = getUserId(user_key);
  
  string userhash_key = "userhash:" + user_id;
  cout << user_id << '\n';
  unordered_map<string, string> user_data;
  redis.hgetall(userhash_key, inserter(user_data, user_data.begin()));
  cout << "Verifying password\n";

  // Step 2: Verify password
  if (!Hash::verifyPassword(password, user_data["password_hash"])) {
    return ""; // Return empty string if password is
  }

  // Step 3: Generate JWT
  string token = JWT::generateToken(user_id);

  // Step 4: Return token
  return token;
}
} // namespace UserDB