#pragma once
#include <string>
#include <sw/redis++/redis++.h>
#include <string>

using namespace std;
using namespace sw::redis;

namespace Global {
    extern string JWT_SECRET;
    extern Redis db;
}