#include "lib.h"
#include "utils.h"
#include <iostream>
#include <string>
#include <cstring>
#include <fstream>
#include <sw/redis++/redis++.h>
#include <jwt-cpp/jwt.h>

using namespace sw::redis;
using namespace std;

// Global variables
string dir = "";
string JWT_SECRET = "";
Redis redis("tcp://127.0.0.1:6379");

void readEnv()
{
  ifstream f("../.env");

  if (!f.is_open())
  {
    cerr << "You did not properly set the .env file. See README.md for more information.";
  }
  string s;

  while (getline(f, s))
  {
    vector<string> vals = split(s, '=');
    if (vals[0] == "JWT_SECRET")
    {
      JWT_SECRET = vals[1];
    }
  }
}

// HELP OF AI
void create_team(const string &team_name)
{
  // Step 1: Generate a new team ID by incrementing the value at the key "team:id:counter"
  long long team_id = redis.incr("team:id:counter");

  // Step 2: Store team metadata
  string team_key = "team:" + to_string(team_id);
  /* redis.hset("user:100", "name", "Alice"); */
  /* Equivalent to: user["name"] = "Alice"; */
  unordered_map<string, string> m = {
      {"name", team_name},
      {"created_at", to_string(time(nullptr))}};
  redis.hmset("hash", m.begin(), m.end());

  // Step 3: Add to tracking set
  redis.sadd("teams:all", to_string(team_id));

  // Add to sorted set by timestamp
  redis.zadd("teams:by_created", to_string(team_id), time(nullptr));

  cout << "Created team '" << team_name << "' with ID " << team_id << endl;
}

bool team_exists(long long team_id)
{
  string team_key = "team:" + to_string(team_id);
  return redis.exists(team_key) == 1;
}

unordered_set<string> get_all_teams()
{
  unordered_set<string> set;
  redis.smembers("teams:all", inserter(set, set.begin()));
  return set;
}

unordered_map<string, string> get_team_info(Redis &redis, long long team_id)
{
  string team_key = "team:" + to_string(team_id);
  unordered_map<string, string> hash;
  redis.hgetall("hash", inserter(hash, hash.end()));
  return hash;
}

// void register_team_name(Redis& redis, const string& team_name, long long team_id) {
//     redis.set("team:name:" + team_name, to_string(team_id));
// }

optional<string> get_team_id_by_name(Redis &redis, const string &team_name)
{
  auto val = redis.get("team:name:" + team_name);
  if (val)
    return *val;
  return nullopt;
}

void printSet(unordered_set<string> const &s)
{
    copy(s.begin(),
            s.end(),
            ostream_iterator<string>(cout, " "));
}

const string handleGETResponse(const HttpRequest &req)
{
  if (req.url == "/")
  {
    const string body = "";

    const string response = sendString("200 OK", "");
    return response;
  }
  // Else condition
  return sendString("404 Not Found", "");
}

const string handlePOSTResponse(const HttpRequest &req)
{
  if (startsWith(req.url, "/createteam")) {
    create_team("test");
    unordered_set<string> teams = get_all_teams();
    printSet(teams);
  } else if (startsWith(req.url, "/createaccount"))
  {
    try
    {
      // auto token = jwt::create()
      //                  .set_type("JWS")
      //                  .set_issuer("auth0")
      //                  .sign(jwt::algorithm::hs256{JWT_SECRET});
      // cout << token << '\n';

      // redis.set("session:" + token, "user42", chrono::seconds(3600));
      // auto val = redis.get("session:abc123");
      // if (val)
      // {
      //   cout << "Session belongs to: " << *val << endl;
      // }
      // redis.hset("session:abc123", {{"user_id", "user42"},
      //                               {"login_time", "2025-08-07T13:45:00Z"}});
      // redis.expire("session:abc123", chrono::seconds(3600));
    }
    catch (const Error &err)
    {
      cerr << "Redis error: " << err.what() << endl;
    }
  }
  // Else condition
  return sendString("404 Not Found", "");
}

const string middleware(const HttpRequest &req)
{
  if (req.method == "GET")
  {
    return handleGETResponse(req);
  }
  else if (req.method == "POST")
  {
    return handlePOSTResponse(req);
  }
  // Else condition
  const string response = sendString("404 Not Found", "");
  return response;
}

int main(int argc, char **argv)
{
  Server server;
  // server.setMaxCharLength(int);
  // https://github.com/varunarya002/codecrafters-http-server-cpp/blob/472d238d47d555645dc8d15081c45fbee8061006/src/server.cpp
  if (argc == 3 && strcmp(argv[1], "--directory") == 0)
  {
    dir = argv[2];
  }

  cout << "Assuming dir is " << (dir == "" ? "Empty" : dir) << '\n';
  server.init();
  server.start(middleware);
  return 0;
}