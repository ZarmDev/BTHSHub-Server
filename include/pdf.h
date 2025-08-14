#include <nlohmann/json.hpp>
#include "global.h"

using namespace std;

namespace PDF {
string getPDFText(const string &fileLocation);
nlohmann::json scheduleToJson(const vector<Day>& schedule);
vector<Day> parseSchedule(const string& text);
}