#include "global.h"
#include "jwt-cpp/traits/nlohmann-json/traits.h"
#include <jwt-cpp/jwt.h>
#include <iostream>

using namespace std;

// auto token = jwt::create()
//                  .set_type("JWS")
//                  .set_issuer("auth0")
//                  .sign(jwt::algorithm::hs256{JWT_SECRET});
// cout << token << '\n';

// redis.set("session:" + token, "user42", chrono::seconds(3600));
// auto val = redis.get("se23", {{"user_id", "user42"},
//                               {"login_time", "2025-08-07T13:45:00Z"}});
// redis.expire("session:abc123", chrono::seconds(3600));ssion:abc123");
// if (val)
// {
//   cout << "Session belongs to: " << *val << endl;
// }
// redis.hset("session:abc1
using traits = jwt::traits::nlohmann_json;
namespace JWT {
const string generateToken(const string &user_id) {
  return jwt::create<traits>()
      .set_type("JWS")
      .set_issuer("auth0")
      .set_subject(user_id)
      .set_issued_at(chrono::system_clock::now())
      .set_expires_at(chrono::system_clock::now() + chrono::minutes{30})
      .sign(jwt::algorithm::hs256{Global::JWT_SECRET});
}

// returns userID if success, otherwise an empty string
const string verifyJWTToken(const string &token) {
  if (token.length() > 1000) {
    cout << "Attempt to send huge token to server\n";
    return "";
  }
  try {
    // Parse/decode the JWT token
    auto decoded = jwt::decode<traits>(token);

    // Create a verifier
    auto verifier = jwt::verify<traits>().with_issuer("auth0").allow_algorithm(
        jwt::algorithm::hs256{Global::JWT_SECRET});

    // Verify the token
    verifier.verify(decoded);
    return decoded.get_subject();
  } catch (const jwt::error::token_verification_exception &e) {
    std::cout << "Token verification failed: " << e.what() << std::endl;
    return "";
  } catch (const std::exception &e) {
    std::cout << "Error: " << e.what() << std::endl;
    return "";
  }
}

const string getUserIdFromToken(const string& token) {
  try {
    // Parse/decode the JWT token
    auto decoded = jwt::decode<traits>(token);

    // Create a verifier
    auto verifier = jwt::verify<traits>().with_issuer("auth0").allow_algorithm(
        jwt::algorithm::hs256{Global::JWT_SECRET});

    // Verify the token
    verifier.verify(decoded);

    const string sub = decoded.get_subject();
    return sub;
  } catch (const jwt::error::token_verification_exception &e) {
    std::cout << "Token verification failed: " << e.what() << std::endl;
    return "";
  } catch (const std::exception &e) {
    std::cout << "Error: " << e.what() << std::endl;
    return "";
  }
}
} // namespace JWT