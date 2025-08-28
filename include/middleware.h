#include "lib.h"
#include "pdf.h"
#include <nlohmann/json.hpp>

#define redis Global::db

using namespace std;

bool protectJWT(HttpRequest &req);
bool protectModerator(HttpRequest &req);
bool protectAdmin(HttpRequest &req);
bool protectModeratorOrAdmin(HttpRequest &req);
