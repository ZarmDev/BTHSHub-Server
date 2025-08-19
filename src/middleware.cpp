#include "jwt.h"
#include "pdf.h"
#include "lib.h"
#include <nlohmann/json.hpp>

#define redis Global::db

using namespace std;

bool protectJWT(const HttpRequest &req) {
  return JWT::verifyJWTToken(req.data);
}