#include "global.h"
#include <nlohmann/json.hpp>
using namespace std;

namespace ScheduleDB {
    const string storeScheduleInRedis(const vector<Day>& schedule, const string& userId);
    OptionalString getSchedule(const string& userId);
}