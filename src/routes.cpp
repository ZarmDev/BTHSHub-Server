#include "global.h"
#include "lib.h"
#include "pdf.h"
#include "teamdatabase.h"
#include "userdatabase.h"
#include "scheduledatabase.h"
#include "utils.h"
#include <iostream>
#include <nlohmann/json_fwd.hpp>
#include <sodium.h>
#include <string>
#include <vector>
#include <nlohmann/json.hpp>
#include <filesystem>
#include <fstream>

using namespace std;

/*
Expect req.data to be:
teamname\n
iftheteamisprivate (0 for false, 1 for true)
*/
string createTeamRoute(HttpRequest &req) {
  vector<string> parsed = split(req.data, "\n");
  const string teamName = parsed.at(0);
  cout << "Creating " << teamName << "...\n";
  const string isPrivate = parsed.at(1);
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
  if (TeamDB::createTeam(teamName, isPrivate == "0" ? "false" : "true", getValueFromMiddleware(req, "userID")) == "") {
    return sendString("404 Not Found", "Error creating " + req.data);
  }
  return sendString("200 OK", "Team " + parsed.at(0) + " successfully created!");
}

/*Expect req.data to be:
*/
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
  if (content.empty()) {
    return sendString("404 Not Found", "Content cannot be empty!");
  }
  vector<string> mentions = j["mentions"].get<vector<string>>();
  const string userID = getValueFromMiddleware(req, "userID");
  TeamDB::postAnnoucement(teamName, content, UserDB::getUsernameFromUserId(userID), mentions);
  return sendString("200 OK", "Announcement created successfully");
}

/*
Expect req.data to be:
teamName
*/
string getTeamAnnoucements(HttpRequest &req) {
  const string userID = getValueFromMiddleware(req, "userID");
  int numOfAnnoucements = TeamDB::getNumOfAnnoucements(req.data);
  int past20 = (numOfAnnoucements - 20);
  // Get the past 20 unless there is less than 20, then just get all of them
  nlohmann::json annoucements = TeamDB::getRangeOfAnnoucements(req.data, past20 > 0 ? past20 : 0, numOfAnnoucements);
  return sendString("200 OK", annoucements.dump());
}

/*
Expect req.data to be:
teamname
*/
string addUserToTeam(HttpRequest &req) {
  cout << "1\n";
  const string userID = getValueFromMiddleware(req, "userID");
  if (TeamDB::userIsOnTeam(req.data, UserDB::getUsernameFromUserId(userID))) {
    return sendString("404 Not Found", "You are already on the team: " + req.data);
  }
  cout << "2\n";
  OptionalString teamID = TeamDB::getTeamIDFromName(req.data);
  if (!teamID) {
    return sendString("404 Not Found", "No teamID found for " + req.data);
  }
  cout << "3\n";
  bool addToTeam = TeamDB::addUserToTeam(userID, teamID.value());
  if (addToTeam) {
    return sendString("200 OK", "Added user to " + req.data);
  } else {
    return sendString("404 Not Found", "There was an issue adding you to " + req.data);
  }
}

/*
Expect req.data to be:
teamname
usertoadd (in username form)
*/
string addOtherUserToTeam(HttpRequest &req) {
  vector<string> parsed = split(req.data, "\n");
  const string teamName = parsed.at(0);
  const string username = parsed.at(1);
  OptionalString teamID = TeamDB::getTeamIDFromName(teamName);
  if (!teamID) {
    return sendString("404 Not Found", "Your team ID is invalid");
  }
  if (TeamDB::userIsOnTeam(teamName, username)) {
    return sendString("404 Not Found", "There was an issue adding you to " + req.data);
  }
  const string userInviting = getValueFromMiddleware(req, "userID");
  const string userBeingInvited = UserDB::getUserIdByUsername(username);
  cout << userInviting << " is inviting " << userBeingInvited << " to " << teamName << '\n';
  bool addToTeam = TeamDB::addOtherUserToTeam(userInviting, userBeingInvited, teamID.value());
  if (addToTeam) {
    return sendString("200 OK", "Added user to " + teamName);
  } else {
    return sendString("404 Not Found", "There was an issue adding the user to " + teamName);
  }
}

string createUserRoute(HttpRequest &req) {
  try {
    vector<string> parsed = split(req.data, "\n");
    printContainer(parsed);
    const string createUserAttempt =
        UserDB::createUser(parsed.at(0), parsed.at(1), parsed.at(2));
    const bool isError = createUserAttempt.starts_with("Error");
    return sendString(isError ? "404 Not Found" : "200 OK", createUserAttempt);
  } catch (const std::exception &e) {
    cerr << "Error at createUserRoute: " << e.what() << endl;
    return sendString("404 Not Found", "An error occured on our side");
  }
}

string loginRoute(HttpRequest &req) {
  vector<string> parsed = split(req.data, "\n");
  const string token = UserDB::handleLogin(parsed.at(0), parsed.at(1));
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

/*
Expect req.data to be:
teamname
*/
string getTeamInfo(HttpRequest &req) {
  // Only give the team information if you are on the team
  const string teamID = getValueFromMiddleware(req, "teamID");
  cout << "Passed teamID check " << teamID << '\n';
  nlohmann::json j = TeamDB::getTeamInfo(teamID);
  return sendString("200 OK", j.dump());
}

constexpr bool isValidAdminLevel(const string& level) {
    constexpr char validLevels[] = {'0', '1', '2'};
    return level.size() == 1 && 
           find(begin(validLevels), end(validLevels), level[0]) != end(validLevels);
}

// Since this is protected with the admin middleware, we can assume we have all permissions to do anything
/*
Expect req.data to be:
targetusername\n
adminlevel
*/
string updateOtherUserAdminLevel(HttpRequest &req) {
  vector<string> parsed = split(req.data, "\n");
  const string adminLevel = parsed.at(1);
  // No need to sanitize input since level is sanitized by middleware
  UserDB::grantAdminLevel(parsed.at(0), parsed.at(1));
  return sendString("200 OK", "Changed permission successfully!");
}

/*
Expect req.data to be:
*/
string getUserTeams(HttpRequest &req) {
  unordered_set<string> userTeams = TeamDB::getUserTeams(getValueFromMiddleware(req, "userID"));
  nlohmann::json j = userTeams;
  return sendString("200 OK", j.dump());
}

/*
Expect req.data to be:
*/
string getPermissionLevel(HttpRequest &req) {
  OptionalString permissionLevel = UserDB::getPermissionLevel(getValueFromMiddleware(req, "userID"));
  if (!permissionLevel) {
    return sendString("404 Not Found", "Unable to get user's permission level...");
  }
  return sendString("200 OK", permissionLevel.value());
}

/*
Expect req.data to be:
teamname
*/
string getTeamMembers(HttpRequest &req) {
  unordered_set<string> teamMembers = TeamDB::getTeamMembers(getValueFromMiddleware(req, "teamID"));
  nlohmann::json j = teamMembers;
  return sendString("404 Not Found", j.dump());
}

/*
Expect req.data to be:
teamname
*/
string getTeamCoaches(HttpRequest &req) {
  unordered_set<string> teamOwners = TeamDB::getTeamOwners(getValueFromMiddleware(req, "teamID"));
  nlohmann::json j = teamOwners;
  return sendString("404 Not Found", j.dump());
}

/*
Expect req.data to be:
TODO: fill this out
*/
string updateUserLogo(HttpRequest &req) {
  auto pdf = extractPdfFromRequest(req);
    if (!pdf) {
        return sendString("400 Bad Request", "{\"error\":\"Invalid PDF upload\"}");
    }
    
    // Validate file type
    if (pdf->content_type != "application/pdf") {
        return sendString("400 Bad Request", "{\"error\":\"File must be a PDF\"}");
    }
    
    // Before saving the file
    if (!filesystem::exists("uploads")) {
      filesystem::create_directory("uploads");
    }

    // Save PDF to disk
    string filename = "uploads/" + pdf->filename;
    ofstream outfile(filename, ios::binary);
    outfile.write((char*)pdf->data.data(), pdf->data.size());
    outfile.close();
    
    return sendString("200 OK", "{\"success\":true,\"filename\":\"" + pdf->filename + "\"}");
}

/*
Expect req.data to be:
TODO: FILL THIS OUT
*/
// Usage of AI/me
string uploadPDF(HttpRequest &req) {
  cout << "extracting...\n";
  auto pdf = extractPdfFromRequest(req);
    if (!pdf) {
        return sendString("400 Bad Request", "{\"error\":\"Invalid PDF upload\"}");
    }
    
    // Validate file type
    if (pdf->content_type != "application/pdf") {
        return sendString("400 Bad Request", "{\"error\":\"File must be a PDF\"}");
    }
    
    // Process PDF directly from memory
    string extractedText = PDF::getPDFText(pdf->data);
    
    vector<Day> schedule = PDF::parseSchedule(extractedText);

    const string j = ScheduleDB::storeScheduleInRedis(schedule, getValueFromMiddleware(req, "userID"));
  
    return sendString("200 OK", j);
}

/*
Expect req.data to be:
*/
string getSchedule(HttpRequest &req) {
  OptionalString j = ScheduleDB::getSchedule(getValueFromMiddleware(req, "userID"));
  if (j) {
    return sendString("200 OK", j.value());
  } else {
    return sendString("404 Not Found", "");
  }
}