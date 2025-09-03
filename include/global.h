#pragma once
#include <sw/redis++/redis++.h>
#include <string>
#include <mutex>

using namespace sw::redis;
using namespace std;

struct Course {
    string courseId;
    string courseName;
    string room;
    string teacher;
    string timeSlot;
};

struct Day {
    int dayNumber;
    vector<Course> courses;
};

namespace Global {
    extern string JWT_SECRET;
    extern Redis db;
    extern string serverOrigin;
    extern mutex redisMutex;
}