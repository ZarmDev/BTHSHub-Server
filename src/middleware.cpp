#include "jwt.h"
#include "lib.h"
#include "pdf.h"
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