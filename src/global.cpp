#include "global.h"
#include <sw/redis++/redis++.h>

using namespace sw::redis;

namespace Global {
string JWT_SECRET;
Redis *db = nullptr;
string serverOrigin;
// To prevent "race conditions". Prevent two threads from simultaneously
// accessing data
mutex redisMutex;

void initializeRedis() {
  // Check if REDIS_URL environment variable exists (Heroku sets this)
  const char *redisUrl = std::getenv("REDIS_URL");

  if (redisUrl) {
    // We're on Heroku, use the provided Redis URL
    std::cout << "Using Heroku Redis configuration" << std::endl;
    db = new Redis(redisUrl);
  } else {
    // Local development
    std::cout << "Using local Redis configuration" << std::endl;
    db = new Redis("tcp://127.0.0.1:6379");
  }
}
} // namespace Global