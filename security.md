# Security according to AI
### Redis Security

1. **Enable Authentication**:
   ```cpp
   // filepath: /home/d/projects/codecrafters-challenges/BTHSHub/src/global.cpp
   Redis db{"tcp://127.0.0.1:6379", "strong_password_here"};
   ```

2. **Enable TLS for Redis** (if deployed in production):
   ```cpp
   ConnectionOptions opts;
   opts.host = "127.0.0.1";
   opts.port = 6379;
   opts.password = "strong_password";
   opts.tls.enabled = true;
   Redis db(opts);
   ```

3. **Restrict Redis Network Access**:
   ```bash
   # In redis.conf
   bind 127.0.0.1
   protected-mode yes
   ```

### JWT Security

1. **Add Expiration Checking** in your `JWTTokenMiddleware` function:
   ```cpp
   bool JWTTokenMiddleware(const HttpRequest &req) {
       // Extract token from Authorization header
       auto it = req.headers.find("Authorization");
       if (it == req.headers.end()) return false;
       
       string token = it->second.substr(7); // Remove "Bearer "
       return JWT::verifyToken(token);
   }
   ```

2. **Rotate JWT Secret** periodically as you mentioned in your README

### Password & Data Security

1. **Add CSRF Protection**:
   ```cpp
   // Generate a token on form pages
   string generateCSRFToken(const string& session_id) {
       return Hash::hashPassword(session_id + to_string(time(nullptr))).substr(0, 32);
   }
   ```

2. **Sanitize Inputs** to prevent injection attacks:
   ```cpp
   string sanitizeInput(const string& input) {
       string sanitized = input;
       // Remove dangerous characters
       return sanitized;
   }
   ```

3. **Implement Rate Limiting** for login attempts:
   ```cpp
   bool checkRateLimit(const string& ip_address, const string& endpoint) {
       string key = "ratelimit:" + endpoint + ":" + ip_address;
       long long count = redis.incr(key);
       
       if (count == 1) {
           redis.expire(key, 60); // Set TTL for the key (1 minute)
       }
       
       // Return true if within limit, false otherwise
       return (endpoint == "login" && count <= 3) || (count <= 60);
   }
   ```

### Server Hardening

1. **Add HTTPS Support** - Update your server to support HTTPS with OpenSSL

2. **Security Headers** in your responses:
   ```cpp
   string sendString(const string &status, const string &body) {
       // ...existing code...
       response += "X-Content-Type-Options: nosniff\r\n";
       response += "X-Frame-Options: DENY\r\n";
       response += "Content-Security-Policy: default-src 'self'\r\n";
       // ...rest of the function...
   }
   ```

3. **Input Validation** for all client data:
   ```cpp
   string createUserRoute(const HttpRequest &req) {
       // Validate email format
       if (!isValidEmail(parsed[2])) {
           return sendString("400 Bad Request", "Invalid email format");
       }
       
       // Validate password strength
       if (!isStrongPassword(parsed[1])) {
           return sendString("400 Bad Request", "Password too weak");
       }
       // ...existing code...
   }
   ```

4. **Implement Proper Logging**:
   ```cpp
   void logSecurityEvent(const string& event_type, const string& user, const string& details) {
       string log_entry = to_string(time(nullptr)) + "|" + event_type + "|" + user + "|" + details;
       writeToFile("security_log.txt", log_entry + "\n");
       
       // For critical events, also log to Redis
       if (event_type == "LOGIN_FAILURE" || event_type == "UNAUTHORIZED_ACCESS") {
           redis.rpush("security:events", log_entry);
       }
   }
   ```

These improvements will help make your application more secure against common threats while maintaining its functionality.

# My own notes
## Add testing
Use burpsuite or a script to test with random characters, try to poke at different routes
## Add captcha/prevent bots
## Rate limiting of course
## DDOS test
## Hypothetical scenario on how to contain your server if it is compromised