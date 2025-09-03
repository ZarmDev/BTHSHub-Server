#pragma once

#include <string>

using namespace std;

struct HttpRequest;

string createTeamRoute(HttpRequest &req);
string createUserRoute(HttpRequest &req);
string loginRoute(HttpRequest &req);
string defaultRoute(HttpRequest &req);
string getDailyAnnoucement(HttpRequest &req);
string setDailyAnnoucement(HttpRequest &req);
string uploadSchedule(HttpRequest &req);
string getAllTeams(HttpRequest &req);
string createTeamAnnoucement(HttpRequest &req);
string getTeamAnnoucements(HttpRequest &req);
string addUserToTeam(HttpRequest &req);
string addOtherUserToTeam(HttpRequest &req);
string getTeamInfo(HttpRequest &req);
string updateOtherUserAdminLevel(HttpRequest &req);
string getUserTeams(HttpRequest &req);
string getPermissionLevel(HttpRequest &req);
string getTeamMembers(HttpRequest &req);
string getTeamCoaches(HttpRequest &req);