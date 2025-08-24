#include "global.h"
#include "lib.h"
#include "middleware.h"
#include "routes.h"
#include "userdatabase.h"
#include "utils.h"
#include <cstring>
#include <fstream>
#include <iostream>
#include <sodium.h>
#include <string>
#include <sw/redis++/errors.h>

#define redis Global::db

using namespace std;

// Global in this file
string dir;
string adminPassword;

void readEnv() {
  ifstream f("../.env");

  if (!f.is_open()) {
    cerr << "You did not create an .env file. See README.md for more "
            "information.\n";
    exit(1);
  }
  string s;

  try {
    while (getline(f, s)) {
      vector<string> vals = split(s, "=");
      if (vals.at(0) == "JWT_SECRET") {
        Global::JWT_SECRET = vals.at(1);
      } else if (vals.at(0) == "ADMINPASS") {
        adminPassword = vals.at(1);
      }
    }
  } catch (...) {
    cerr << "Invalid .env file!\n";
    exit(1);
  }
}

int main(int argc, char **argv) {
  readEnv();
  Server server;

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

  // NOT FOR PRODUCTION. TODO
  try {
    bool createAdmin = UserDB::createUser("admin", adminPassword, "");
  } catch (...) {
    cerr << "Your redis DB is not online.\n Run redis-server & to start it.\n";
    exit(1);
  }
  // not protected
  server.get("/", defaultRoute);
  server.post("/login", loginRoute);
  server.post("/createuser", createUserRoute);

  server.use(protectJWT);
  // protected with JWT token
  server.post("/api/createteam", createTeamRoute);
  server.get("/api/getdailyannoucement", getDailyAnnoucement);
  server.post("/api/setdailyannoucement", setDailyAnnoucement);
  server.post("/api/uploadschedule", uploadSchedule);

  server.start();

  return 0;
}