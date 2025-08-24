#include "lib.h"
#include "pdf.h"
#include <nlohmann/json.hpp>

#define redis Global::db

using namespace std;

bool protectJWT(const HttpRequest &req);
bool protectModerator(const HttpRequest &req);
bool protectAdmin(const HttpRequest &req);
bool protectModeratorOrAdmin(const HttpRequest &req);
