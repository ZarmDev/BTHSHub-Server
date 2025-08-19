#pragma once

#include <string>
#include "global.h"

using namespace std;

struct HttpRequest;

string createTeamRoute(const HttpRequest &req);
string createUserRoute(const HttpRequest &req);
string loginRoute(const HttpRequest &req);
string defaultRoute(const HttpRequest &req);
string getDailyAnnoucement(const HttpRequest &req);
bool protectJWT(const HttpRequest &req);
string setDailyAnnoucement(const HttpRequest &req);
string uploadSchedule(const HttpRequest &req);