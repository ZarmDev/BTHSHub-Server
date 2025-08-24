#include "global.h"
#include "lib.h"
#include "pdf.h"
#include "teamdatabase.h"
#include "userdatabase.h"
#include "utils.h"
#include <iostream>
#include <sodium.h>
#include <stdexcept>
#include <string>
#include <vector>
#include <nlohmann/json.hpp>

using namespace std;

string createTeamRoute(const HttpRequest &req) {
  cout << "Creating team...\n";
  // Check max team name length
  if (req.data.length() > 50) {
    return sendString("404 Not Found", "Team name too long!");
  }
  // Ensure no duplicates
  if (TeamDB::teamExistsByName(req.data)) {
    return sendString("404 Not Found", "Team already exists!");
  }

  TeamDB::createTeam(req.data);
  unordered_set<string> teams = TeamDB::getAllTeams();
  printContainer(teams);
  printContainer(TeamDB::getTeamInfo(1));
  return sendString("200 Success", "Team " + req.data + " successfully created!");
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

string getAllTeams(const HttpRequest &req) {
  unordered_set teams = TeamDB::getAllTeams();

  nlohmann::json j = teams;
  return sendString("200 OK", j.dump());
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

// TODO: Check if admin level is 3
string setDailyAnnoucement(const HttpRequest &req) {
  const string body = "";

  const string response = sendString("200 OK", "");
  return response;
}

// Receive the data as binary and ensure client has header of 'Content-Type': 'multipart/form-data',
string uploadSchedule(const HttpRequest &req) {
  cout << req.data << '\n';
  return "";
  // TODO: Set the maximum amount of characters a client can send. Fix lib.cpp to somehow be able to change the amount it reads
  // Remember to check the file size because if it's too big they clearly are slowing down the server
  const string text = PDF::getPDFText("../src/ProgramCard.pdf");
  vector<Day> parsed = PDF::parseSchedule(text);
}