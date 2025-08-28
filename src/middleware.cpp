#include "jwt.h"
#include "lib.h"
#include "pdf.h"
#include "userdatabase.h"
#include <nlohmann/json.hpp>

#define redis Global::db

using namespace std;

bool protectJWT(HttpRequest &req) {
  cout << "protectJWT\n";
  auto it = req.headers.find("Authorization");
  if (it == req.headers.end()) {
    // Header not found
    return false;
  }
  const string& authHeader = it->second;
  string userID = JWT::verifyJWTToken(authHeader);
  if (userID == "") {
    return false;
  }
  // Pass userID to next routes
  req.extra["userID"] = userID;
  return true;
}

bool protectModerator(HttpRequest &req) {
  auto it = req.headers.find("Authorization");
  if (it == req.headers.end()) {
    // Header not found
    return false;
  }
  const string& authHeader = it->second;
  const string userID = JWT::getUserIdFromToken(authHeader);
  string adminLevel = *redis.hget("user:" + userID, "adminLevel");
  // Pass userID to next routes
  req.extra["userID"] = userID;
  if (adminLevel == "1") {
    return true;
  }
  return false;
}

bool protectAdmin(HttpRequest &req) {
  auto it = req.headers.find("Authorization");
  if (it == req.headers.end()) {
    // Header not found
    return false;
  }
  const string& authHeader = it->second;
  const string userID = JWT::getUserIdFromToken(authHeader);
  string adminLevel = *redis.hget("user:" + userID, "adminLevel");
  // Pass userID to next routes
  req.extra["userID"] = userID;
  if (adminLevel == "2") {
    return true;
  }
  return false;
}

bool protectModeratorOrAdmin(HttpRequest &req) {
  auto it = req.headers.find("Authorization");
  if (it == req.headers.end()) {
    // Header not found
    return false;
  }
  const string& authHeader = it->second;
  const string userID = JWT::getUserIdFromToken(authHeader);
  cout << userID << '\n';
  string adminLevel = *redis.hget("user:" + userID, "adminLevel");
  // Pass userID to next routes
  req.extra["userID"] = userID;
  cout << "adminLevel: " << adminLevel << '\n';
  UserDB::printUserHash(userID);
  if (adminLevel == "2" || adminLevel == "1") {
    return true;
  }
  return false;
}

