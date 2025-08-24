#include "jwt.h"
#include "lib.h"
#include "pdf.h"
#include "userdatabase.h"
#include <nlohmann/json.hpp>

#define redis Global::db

using namespace std;

bool protectJWT(const HttpRequest &req) {
  cout << "protectJWT\n";
  auto it = req.headers.find("Authorization");
  if (it == req.headers.end()) {
    // Header not found
    return false;
  }
  const string& authHeader = it->second;
  return JWT::verifyJWTToken(authHeader);
}

bool protectModerator(const HttpRequest &req) {
  auto it = req.headers.find("Authorization");
  if (it == req.headers.end()) {
    // Header not found
    return false;
  }
  const string& authHeader = it->second;
  const string user_id = JWT::getUserIdFromToken(authHeader);
  string adminLevel = *redis.hget("user:" + user_id, "adminLevel");
  if (adminLevel == "1") {
    return true;
  }
  return false;
}

bool protectAdmin(const HttpRequest &req) {
  auto it = req.headers.find("Authorization");
  if (it == req.headers.end()) {
    // Header not found
    return false;
  }
  const string& authHeader = it->second;
  const string user_id = JWT::getUserIdFromToken(authHeader);
  string adminLevel = *redis.hget("user:" + user_id, "adminLevel");
  if (adminLevel == "2") {
    return true;
  }
  return false;
}

bool protectModeratorOrAdmin(const HttpRequest &req) {
  auto it = req.headers.find("Authorization");
  if (it == req.headers.end()) {
    // Header not found
    return false;
  }
  const string& authHeader = it->second;
  const string user_id = JWT::getUserIdFromToken(authHeader);
  cout << user_id << '\n';
  string adminLevel = *redis.hget("user:" + user_id, "adminLevel");
  cout << "adminLevel: " << adminLevel << '\n';
  UserDB::printUserHash(user_id);
  if (adminLevel == "2" || adminLevel == "1") {
    return true;
  }
  return false;
}

