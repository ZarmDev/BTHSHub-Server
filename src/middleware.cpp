#include "jwt.h"
#include "lib.h"
#include "pdf.h"
#include "userdatabase.h"
#include "teamdatabase.h"
#include "utils.h"
#include <nlohmann/json.hpp>
#include <sw/redis++/utils.h>
#include <iostream>

#define redis Global::db

using namespace std;

enum class AccessLevel {
  ANY_USER,      // Any authenticated user
  MODERATOR,     // Moderator only
  ADMIN,         // Admin only
  MOD_OR_ADMIN   // Either moderator or admin
};

bool protectRoute(HttpRequest &req, AccessLevel level) {
  // Always allow OPTIONS for CORS preflight
  if (req.method == "OPTIONS") {
    return true;
  }
  
  // Check for Authorization header
  auto it = req.headers.find("Authorization");
  if (it == req.headers.end()) {
    return false; // Header not found
  }
  
  const string& authHeader = it->second;
  string userID;
  
  // For ANY_USER level, verify token
  if (level == AccessLevel::ANY_USER) {
    userID = JWT::verifyJWTToken(authHeader);
    if (userID.empty()) return false;
    req.extra["userID"] = userID;
    return true;
  }
  
  // For other levels, get admin level
  userID = JWT::getUserIdFromToken(authHeader);
  if (userID.empty()) return false;
  
  string adminLevel = redis.hget("user:" + userID, "adminLevel").value();
  req.extra["userID"] = userID;
  
  // Check permissions based on level
  switch (level) {
    case AccessLevel::MODERATOR:
      return adminLevel == "1";
    case AccessLevel::ADMIN:
      return adminLevel == "2";
    case AccessLevel::MOD_OR_ADMIN:
      return adminLevel == "1" || adminLevel == "2";
    default:
      return false;
  }
}

bool protectJWT(HttpRequest &req) {
  return protectRoute(req, AccessLevel::ANY_USER);
}

bool protectModerator(HttpRequest &req) {
  return protectRoute(req, AccessLevel::MODERATOR);
}

bool protectAdmin(HttpRequest &req) {
  return protectRoute(req, AccessLevel::ADMIN);
}

bool protectModeratorOrAdmin(HttpRequest &req) {
  return protectRoute(req, AccessLevel::MOD_OR_ADMIN);
}

bool protectTeamMember(HttpRequest &req) {
  const string userID = getValueFromMiddleware(req, "userID");
  const string username = UserDB::getUsernameFromUserId(userID);
  cout << userID << " " << username << "" << req.data << '\n';
  if (TeamDB::userIsOnTeam(req.data, username) || UserDB::isUserAdmin(userID)) {
    OptionalString teamID = TeamDB::getTeamIDFromName(req.data);
    if (teamID) {
      cout << "teamID found " << teamID.value() << '\n';
      req.extra["teamID"] = teamID.value();
    }
    return true;
  } else {
    return false;
  }
}