#include "global.h"
#include "hash.h"
#include "jwt.h"
#include "pdf.h"
#include "utils.h"
#include <nlohmann/json.hpp>
#include <iostream>
#include <sw/redis++/utils.h>

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
  "users:by_created": SortedSet({ "42": 1723228800, ... }),
  "team:1:members": ("alice"),
  "team:1:coaches": ("Ms.D", "Mr.C")
  "user:42:teams": ("FRC 334")
}
NOTES:
adminLevel - 0 means no permissions, 1 means an app moderator, 2 means all permission admin
*/

namespace UserDB {
// Add an existing user to an existing team
const string createUser(const string &username, const string &password,
                const string &email) {
  lock_guard<mutex> lock(Global::redisMutex);
  // Backslashes in team names, usernames will screw with the places where I used split to get data from req.data
  // Could have parsed the JSON, but I prefer something easy to make and use
  if (username.contains("\\") || password.contains("\\") || email.contains("\\")) {
    return "Error: Any of the fields cannot contain backslashes (\\)";
  }
  // Used for the flat key
  string user_key = "username:" + username;
  string email_key = "email:" + email;

  // Check if username or email already exists
  if (redis.exists(user_key) || redis.exists(email_key)) {
    return "Error: Username or email already exists.";
  }

  // Generate a new user ID (you can use an atomic counter or UUID)
  string user_id = to_string(redis.incr("user:id:counter"));

  // Used for the hash
  string userhash_key = "user:" + user_id;

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

  return "Created user '" + username + "' with ID " + user_id;
}

string getUserIdByUsername(const string& username) {
  OptionalString user_id_opt = redis.get("username:" + username);
  if (!user_id_opt) {
    cout << "Username " << username << " not found" << endl;
    return "";
  }
  return *user_id_opt;
}

string getUsernameFromUserId(const string& userID) {
  OptionalString user_id_opt = redis.hget("user:" + userID, "username");
  if (!user_id_opt) {
    cout << userID << " not found\n";
    return "";
  }

  return user_id_opt.value();
}

bool grantAdminLevel(const string& username, string level) {
  try {
    const string user_key = "user:" + UserDB::getUserIdByUsername(username);
    if (redis.exists(user_key)) {
      return false;
    }
    // Set adminLevel to 2 (admin)
    redis.hset(user_key, "adminLevel", level);
  } catch (...) {
    cout << "Unable to grant admin level " << username << " " << level;
    return false;
  }
  return true;
};

void printUserHash(const string& user_id) {
    unordered_map<string, string> user_data;
    redis.hgetall("user:" + user_id, inserter(user_data, user_data.begin()));
    cout << "user:" << user_id << " hash fields:\n";
    for (const auto& [key, value] : user_data) {
        cout << key << ": " << value << '\n';
    }
}

string handleLogin(const string &username, const string &password) {
  // printContainer(users);
  // printAllRedisKeys();

  const string user_key = "username:" + username;

  // Step 1: Lookup user by username
  const string user_id = getUserIdByUsername(username);
  
  const string userhash_key = "user:" + user_id;
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

bool isUserAdmin(const string& userID) {
  string adminLevel = redis.hget("user:" + userID, "adminLevel").value();
  if (adminLevel == "2") {
    return true;
  }
  return false;
}

OptionalString getPermissionLevel(const string& userID)
{
  OptionalString adminLevel = redis.hget("user:" + userID, "adminLevel");
  return adminLevel;
}
} // namespace UserDB