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
#include <sw/redis++/errors.h>
#include <cstdlib>

using namespace std;

namespace fs = filesystem;

// Global in this file
string dir;
string adminPassword;

optional<fs::path> find_env_file(const vector<fs::path>& search_dirs) {
  for (const auto& dir : search_dirs) {
    fs::path candidate = dir / ".env";
    if (fs::exists(candidate) && fs::is_regular_file(candidate)) {
      return candidate;
    }
  }
  return nullopt;
}

void readEnv() {
  vector<fs::path> search_paths = {
    fs::current_path(),
    fs::current_path().parent_path()
  };
  optional<fs::path> env_path = find_env_file(search_paths);

  if (!env_path.has_value())
  {
    cerr << "You did not create an .env file. See README.md for more "
            "information.\n";
  }
  ifstream f(env_path.value());
  cout << "Current working directory: "
              << filesystem::current_path() << filesystem::exists(".env") << endl;
  // if (!f.is_open()) {
  //   cerr << "You did not create an .env file. See README.md for more "
  //           "information.\n";
  //   exit(1);
  // }
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
  // Maximum of 250KB
  server.setMaxCharacters(1024 * 250);
  // NOT FOR PRODUCTION. Change to server url during production.
  Global::serverOrigin = "*";


  // NOT FOR PRODUCTION
  server.post("/adminsetup", [](HttpRequest &req) -> string {
    try {
      const string createAdmin = UserDB::createUser("admin", adminPassword, "");
      UserDB::grantAdminLevel("admin", "2");
    } catch (...) {
      cerr
          << "Your redis DB is not online.\n Run redis-server & to start it.\n";
      exit(1);
    }
    return sendString("200 OK", "created admin. NOT FOR PRODUCTION");
  });

  // not protected. DOES NOT send userID since it doesn't exist here
  server.get("/", defaultRoute);
  server.post("/login", loginRoute);
  server.post("/createuser", createUserRoute);

  server.use(protectJWT);
  // protected for regular users. sends userID in req.extra
  server.get("/api/getallteams", getAllTeams);
  server.get("/api/getpermissionlevel", getPermissionLevel);
  server.get("/api/getdailyannoucement", getDailyAnnoucement);
  server.post("/api/uploadschedule", uploadSchedule);
  server.post("/api/addusertoteam", addUserToTeam);
  server.get("/api/getmyteams", getUserTeams);
  // server.post("/api/updateuserlogo", updateUserLogo);
  server.post("/api/uploadpdf", uploadPDF);
  server.get("/api/getschedule", getSchedule);

  // only allow real users who are also members of the team, an admin or a coach/moderator
  server.use({protectJWT, protectTeamMember});
  server.get("/api/getteammembers", getTeamMembers);
  server.get("/api/getteamcoaches", getTeamCoaches);
  server.post("/api/createannoucement", createTeamAnnoucement);
  server.post("/api/getannoucements", getTeamAnnoucements);
  server.post("/api/getteaminfo", getTeamInfo);

  server.use(protectModeratorOrAdmin);
  // protected only for moderators (coaches, club execs) or admins. sends userID in req.extra
  server.post("/mod/createteam", createTeamRoute);
  server.post("/mod/addotherusertoteam", addOtherUserToTeam);

  server.use(protectAdmin);
  // protected only for admins. sends userID in req.extra
  server.post("/admin/setdailyannoucement", setDailyAnnoucement);
  server.post("/admin/updateotheruseradminlevel", updateOtherUserAdminLevel);

  server.start();

  return 0;
}