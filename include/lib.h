// Use header guard to prevent linker issues in server.cpp
#pragma once

using namespace std;

#include <functional>
#include <unordered_map>
#include <string>
#include <vector>

// Request struct
struct HttpRequest {
  string method;
  string url;
  string protocol;
  unordered_map<string, string> headers;
  string data;
  // For adding extra data (like next() in Next.js)
  unordered_map<string, string>& extra;
};

// Response function type
using ResponseFunc = const string (*)(HttpRequest &req);
using RequestFunc = function<string(HttpRequest &req)>;
using MiddlewareFunc = function<bool(HttpRequest &req)>;

// Server class declaration
class Server {
public:
  bool init(const string &port);
  bool start();
  void get(const string &route, RequestFunc handler, int maxCharacters = -1);
  void post(const string &route, RequestFunc handler, int maxCharacters = -1);
  void use(MiddlewareFunc func);
  void use(const vector<MiddlewareFunc> &funcs);
  string handleRequest(HttpRequest &req);
  void setDefaultMaxCharacters(int num);
  void updateRouteMap(const string& route);

private:
  int server_fd;
  int defaultMaxCharacters = 8192;
  vector<MiddlewareFunc> currentMiddlewareRoutes;
  unordered_map<string, vector<MiddlewareFunc>> middlewareRoutesMap;
  unordered_map<string, RequestFunc> postRoutes;
  unordered_map<string, RequestFunc> getRoutes;
  unordered_map<string, int> routeMaxCharacters;

  void handleClient(int client_fd);
};

// Utility functions
string sendString(const string &status, const string &body);

// Response struct (Help of AI)
struct Response {
  string status = "200 OK";
  string contentType = "text/plain";
  string body = "";

  string toString() const;
};