#include "global.h"
#include "jwt.h"
#include "lib.h"
#include "pdf.h"
#include "teamdatabase.h"
#include "userdatabase.h"
#include "utils.h"
#include <cstring>
#include <fstream>
#include <iostream>
#include <sodium.h>
#include <stdexcept>
#include <string>
#include <vector>

#define redis Global::db

using namespace std;

// Global in this file
string dir;
string adminPassword;

void readEnv() {
  ifstream f("../.env");

  if (!f.is_open()) {
    cerr << "You did not properly set the .env file. See README.md for more "
            "information.\n";
  }
  string s;

  while (getline(f, s)) {
    vector<string> vals = split(s, "=");
    if (vals[0] == "JWT_SECRET") {
      Global::JWT_SECRET = vals[1];
    } else if (vals[0] == "ADMINPASS") {
      adminPassword = vals[1];
    }
  }
}

string createTeamRoute(const HttpRequest &req) {
  TeamDB::createTeam("test");
  unordered_set<string> teams = TeamDB::getAllTeams();
  printContainer(teams);
  printContainer(TeamDB::getTeamInfo(1));
}

string createUserRoute(const HttpRequest &req) {
  try {
    vector<string> parsed = split(req.data, "\n");
    printContainer(parsed);
    bool createUserAttempt =
        UserDB::createUser(parsed[0], parsed[1], parsed[2]);
    if (createUserAttempt) {
      return sendString("200 Success", "Successfully created the user!");
    } else {
      return sendString("404 Not Found", "Unable to create user");
    }
  } catch (const std::runtime_error &e) {
    cerr << "Redis error: " << e.what() << endl;
    return sendString("404 Not Found", "An error occured on our side");
  }
}

string loginRoute(const HttpRequest &req) {
  vector<string> parsed = split(req.data, "\n");
  const string token = UserDB::handleLogin(parsed[0], parsed[1]);
  if (token == "") {
    return sendString("404 Not Found", "Invalid password or username");
  } else {
    return sendString("200 Success", token);
  }
}

string defaultRoute(const HttpRequest &req) {
  const string body = "";

  const string response = sendString("200 OK", "");
  return response;
}

string getDailyAnnoucement(const HttpRequest &req) {
  const string body = "";

  const string response = sendString("200 OK", "");
  return response;
}

bool protectJWT(const HttpRequest &req) {
  return JWT::verifyJWTToken(req.data);
}

// TODO: Check if admin level is 3
string setDailyAnnoucement(const HttpRequest &req) {
  const string body = "";

  const string response = sendString("200 OK", "");
  return response;
}

string parsePDF(const HttpRequest &req) {

}


int main(int argc, char **argv) {
  // Remember to check the file size because if it's too big they clearly are slowing down the server
  const string text = PDF::getPDFText("../src/ProgramCard.pdf");
  vector<Day> parsed = PDF::parseSchedule(text);

  return 1;
  readEnv();
  Server server;
  // server.setMaxCharLength(int);

  // https://github.com/varunarya002/codecrafters-http-server-cpp/blob/472d238d47d555645dc8d15081c45fbee8061006/src/server.cpp
  if (argc == 3 && strcmp(argv[1], "--directory") == 0) {
    dir = argv[2];
  }
  cout << "Assuming dir is " << (dir == "" ? "Empty" : dir) << '\n';
  // Initalize libsodium in order to hash passwords
  if (sodium_init() < 0) {
    cerr << "Failed to initialize libsodium" << endl;
    return 1;
  }
  // Initalize server on 4221
  server.init("4221");

  // Create an admin account
  bool createAdmin = UserDB::createUser("admin", adminPassword, "");
  if (!createAdmin) {
    cout << "Failed to create admin. The DB is not working correctly.\n";
  }

  // not protected
  server.get("/", defaultRoute);
  server.post("/login", loginRoute);
  server.post("/createuser", createUserRoute);

  server.use(protectJWT);
  // protected with JWT token
  server.post("/createteam", createTeamRoute);
  server.get("/getdailyannoucement", getDailyAnnoucement);
  server.post("/setdailyannoucement", setDailyAnnoucement);

  server.start();

  return 0;
}