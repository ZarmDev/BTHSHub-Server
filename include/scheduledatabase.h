#include "global.h"
using namespace std;

namespace ScheduleDB {
    void storeScheduleInRedis(const vector<Day>& schedule, const string& userId);
    OptionalString getSchedule(const string& userId);
}