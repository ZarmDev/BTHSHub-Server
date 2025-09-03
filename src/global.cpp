#include "global.h"
#include <sw/redis++/redis++.h>

using namespace sw::redis;

namespace Global {
    string JWT_SECRET;
    Redis db{"tcp://127.0.0.1:6379"};
    string serverOrigin;
    // To prevent "race conditions". Prevent two threads from simultaneously accessing data
    mutex redisMutex;
}