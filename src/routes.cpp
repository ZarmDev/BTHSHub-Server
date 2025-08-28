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

const string getUserIdFromJWT(HttpRequest &req) {
  auto it = req.extra.find("userID");
  if (it == req.extra.end()) {
    cerr << "No userID supplied! Something is not right here.\n";
  }
  const string& userID = it->second;
  return userID;
}

/*
Expect req.data to be:
teamname\n
iftheteamisprivate (0 for false, 1 for true)
*/
string createTeamRoute(HttpRequest &req) {
  vector<string> parsed = split(req.data, "\n");
  const string teamName = parsed[0];
  cout << "Creating " << teamName << "...\n";
  const string isPrivate = parsed[1];
  // Ensure there is no backslashes (will mess up the split)
  if (req.data.contains("\\")) {
    return sendString("404 Not Found", "Backslashes cannot be used in the team name!");
  }
  // Check max team name length
  if (req.data.length() > 50) {
    return sendString("404 Not Found", "Team name too long!");
  }
  // Ensure no duplicates
  if (TeamDB::teamExistsByName(req.data)) {
    return sendString("404 Not Found", "Team already exists!");
  }
  // To prevent it from defaulting to a private team if the client is messed up
  if (isPrivate != "0" && isPrivate != "1") {
    return sendString("404 Not Found", "Malformed request!");
  }
  TeamDB::createTeam(teamName, isPrivate == "0" ? "false" : "true", getUserIdFromJWT(req));
  unordered_set<string> teams = TeamDB::getAllTeams();
  printContainer(teams);
  printContainer(TeamDB::getTeamInfo(1));
  return sendString("200 OK", "Team " + parsed[0] + " successfully created!");
}

// Using JSON here because it's much more convenient/readable in this case
string createTeamAnnoucement(HttpRequest &req) {
  // Parse the JSON string from req.data
  nlohmann::json j = nlohmann::json::parse(req.data);

  // Access fields
  const string teamName = j["teamName"];
  unordered_set teams = TeamDB::getAllTeams();
  // Make sure teamName exists as a team
  if (!teams.contains(teamName)) {
    return sendString("404 Not Found", teamName + " does not exist!");
  }
  const string content = j["content"];
  vector<string> mentions = j["mentions"].get<vector<string>>();
  const string userID = getUserIdFromJWT(req);
  TeamDB::postAnnoucement(teamName, content, userID, mentions);
  
  return sendString("200 OK", "Announcement created successfully");
}

string getTeamAnnoucements(HttpRequest &req) {
  int numOfAnnoucements = TeamDB::getNumOfAnnoucements(req.data);
  int past50 = (numOfAnnoucements - 50);
  // Get the past 50 unless there is less than 50, then just get all of them
  nlohmann::json annoucements = TeamDB::getRangeOfAnnoucements(req.data, past50 > 0 ? past50 : 0, numOfAnnoucements);
  return sendString("200 OK", annoucements.dump());
}

/*
Expect req.data to be:
teamname
*/
string addUserToTeam(HttpRequest &req) {
  const string userID = getUserIdFromJWT(req);
  const string teamID = TeamDB::getTeamIDFromName(req.data);
  bool addToTeam = TeamDB::addUserToTeam(userID, teamID, false);
  if (addToTeam) {
    return sendString("200 OK", "Added user to " + req.data);
  } else {
    return sendString("404 Not Found", "There was an issue adding you to " + req.data);
  }
}

/*
Expect req.data to be:
teamname
usertoadd
*/
string addOtherUserToTeam(HttpRequest &req) {
  const string verifiedUserID = getUserIdFromJWT(req);
  vector<string> parsed = split(req.data, "\n");
  const string teamID = TeamDB::getTeamIDFromName(parsed[0]);
  const string userID = getUserIdFromJWT(req);
  bool addToTeam = TeamDB::addOtherUserToTeam(userID, teamID);
  if (addToTeam) {
    return sendString("200 OK", "Added user to " + req.data);
  } else {
    return sendString("404 Not Found", "There was an issue adding you to " + req.data);
  }
}

string createUserRoute(HttpRequest &req) {
  try {
    vector<string> parsed = split(req.data, "\n");
    printContainer(parsed);
    const string createUserAttempt =
        UserDB::createUser(parsed[0], parsed[1], parsed[2]);
    return sendString("200 OK", createUserAttempt);
  } catch (const runtime_error &e) {
    cerr << "Redis error: " << e.what() << endl;
    return sendString("404 Not Found", "An error occured on our side");
  }
}

string loginRoute(HttpRequest &req) {
  vector<string> parsed = split(req.data, "\n");
  const string token = UserDB::handleLogin(parsed[0], parsed[1]);
  if (token == "") {
    return sendString("404 Not Found", "Invalid password or username");
  } else {
    return sendString("200 OK", token);
  }
}

string getAllTeams(HttpRequest &req) {
  unordered_set teams = TeamDB::getAllTeams();

  nlohmann::json j = teams;
  return sendString("200 OK", j.dump());
}

string defaultRoute(HttpRequest &req) {
  const string body = "";

  const string response = sendString("200 OK", "");
  return response;
}

string getDailyAnnoucement(HttpRequest &req) {
  const string body = "";

  const string response = sendString("200 OK", "");
  return response;
}

// TODO: Check if admin level is 3
string setDailyAnnoucement(HttpRequest &req) {
  const string body = "";

  const string response = sendString("200 OK", "");
  return response;
}

// Receive the data as binary and ensure client has header of 'Content-Type': 'multipart/form-data',
string uploadSchedule(HttpRequest &req) {
  cout << req.data << '\n';
  return "";
  // TODO: Set the maximum amount of characters a client can send. Fix lib.cpp to somehow be able to change the amount it reads
  // Remember to check the file size because if it's too big they clearly are slowing down the server
  const string text = PDF::getPDFText("../src/ProgramCard.pdf");
  vector<Day> parsed = PDF::parseSchedule(text);
}

/*
Expect req.data to be:
teamname
*/
string getTeamInfo(HttpRequest &req) {
  const string teamID = TeamDB::getTeamIDFromName(req.data);
   nlohmann::json j = TeamDB::getTeamInfo(stoll(teamID));
  return sendString("200 OK", j.dump());
}