#include "global.h"
#include "pdf.h"
#include <nlohmann/json.hpp>
#include <sw/redis++/utils.h>

#define redis Global::db

using namespace std;
/*
{
    "schedule:42": {
        {
            "dayNumber": "1",
            "courses": {
                "courseId": "MXW71238/17",
                "courseName": "Digital Electronics",
                "room": "2W4",
                "teacher": "Jane Smith",
                "timeSlot": "1:30-2:25"
            }
        },
        {
            "dayNumber": "2",
            "courses": [...]
        },
        [...]
    }
}
*/

namespace ScheduleDB {
    // Store in Redis (written by AI)
    void storeScheduleInRedis(const vector<Day>& schedule, const string& userId) {
        nlohmann::json j = PDF::scheduleToJson(schedule);
        string jsonString = j.dump();
        
        // It may seem inefficient, but the schedule only has to be stored once per year for each user. The most common thing the user will do is look at what courses they have every day, not set the schedule all the time.

        // Store the entire schedule as one key
        redis.set("schedule:" + userId, jsonString);
        
        // AND store each day separately for faster individual day access. It's likely that the entire schedule will be used when there is sharing schedules, but, the per day will be used on a daily basis
        for (const auto& day : schedule) {
            nlohmann::json dayJson = PDF::scheduleToJson({day});
            redis.set("schedule:day:" + to_string(day.dayNumber), dayJson.dump());
        }
    }

    // returns json dump (string)
    OptionalString getSchedule(const string& userId) {
        return redis.get("schedule:" + userId);
    }
}