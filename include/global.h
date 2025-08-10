#pragma once
#include <sw/redis++/redis++.h>
#include <string>

using namespace sw::redis;
using namespace std;

namespace Global {
    extern string JWT_SECRET;
    extern Redis db;
}