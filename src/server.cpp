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

const string readEnv()
{
  ifstream f("../.env");

  if (!f.is_open())
  {
    cerr << "You did not properly set the .env file. See README.md for more information.";
    return "";
  }
  string s;

  while (getline(f, s)) {
    vector<string> vals = split(s, '=');
    if (vals[0] == "JWT_SECRET") {
      JWT_SECRET = vals[1];
    }
  }
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
  if (startsWith(req.url, "/createaccount"))
  {
    try
    {
      auto token = jwt::create()
                       .set_type("JWS")
                       .set_issuer("auth0")
                       .set_payload_claim("sample", jwt::claim(std::string("test")))
                       .sign(jwt::algorithm::hs256{JWT_SECRET});
      cout << token << '\n';
      redis.set("session:abc123", "user42", std::chrono::seconds(3600));
      auto val = redis.get("session:abc123");
      if (val)
      {
        std::cout << "Session belongs to: " << *val << std::endl;
      }
      redis.hset("session:abc123", {{"user_id", "user42"},
                                    {"login_time", "2025-08-07T13:45:00Z"}});
      redis.expire("session:abc123", std::chrono::seconds(3600));
    }
    catch (const Error &err)
    {
      std::cerr << "Redis error: " << err.what() << std::endl;
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