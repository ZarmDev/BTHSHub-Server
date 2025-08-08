#include <jwt-cpp/jwt.h>
#include "jwt.h"
#include "../server.h"

using namespace std;

      // auto token = jwt::create()
      //                  .set_type("JWS")
      //                  .set_issuer("auth0")
      //                  .sign(jwt::algorithm::hs256{JWT_SECRET});
      // cout << token << '\n';

      // redis.set("session:" + token, "user42", chrono::seconds(3600));
      // auto val = redis.get("session:abc123");
      // if (val)
      // {
      //   cout << "Session belongs to: " << *val << endl;
      // }
      // redis.hset("session:abc123", {{"user_id", "user42"},
      //                               {"login_time", "2025-08-07T13:45:00Z"}});
      // redis.expire("session:abc123", chrono::seconds(3600));
const string generate_token(const string& user_id) {
    return jwt::create()
        .set_type("JWS")
        .set_issuer("auth0")
        .set_subject(user_id)
        .set_issued_at(chrono::system_clock::now())
        .set_expires_at(chrono::system_clock::now() + chrono::minutes{30})
        .sign(jwt::algorithm::hs256{Global::JWT_SECRET});
}
