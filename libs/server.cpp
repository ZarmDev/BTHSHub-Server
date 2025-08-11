#include "global.h"
#include "lib.h"
#include "utils.h"
#include "teamdatabase.h"
#include "userdatabase.h"
#include <iostream>
#include <string>
#include <cstring>
#include <stdexcept>
#include <fstream>
#define redis Global::db

using namespace std;

// Global variables
string dir;

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
    vector<string> vals = split(s, "=");
    if (vals[0] == "JWT_SECRET")
    {
      Global::JWT_SECRET = vals[1];
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
  if (startsWith(req.url, "/createteam")) {
    TeamDB::createTeam("test");
    unordered_set<string> teams = TeamDB::getAllTeams();
    printSet(teams);
    printMap(TeamDB::getTeamInfo(1));
  } else if (startsWith(req.url, "/createuser"))
  {
    try
    {
      vector<string> parsed = split(req.data, "\\n");
      UserDB::createUser(parsed[0], parsed[1], parsed[2]);
    }
    catch (const std::runtime_error& e)
    {
      cerr << "Redis error: " << e.what() << endl;
      return sendString("404 Not Found", "An error occured on our side");
    }
  } else if (startsWith(req.url, "/login")) {
    vector<string> parsed = split(req.data, "\\n");
    UserDB::handle_login(parsed[0], parsed[1]);
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
  readEnv();
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