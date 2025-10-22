
// Standard library
#include <cstring>
#include <functional>
#include <iostream>
#include <string>
#include <thread>
#include <unordered_map>
#include <utility>
#include <vector>
// ?
#include <sys/types.h>
#include <unistd.h>
// Socket/linux networking
#include "global.h"
#include "lib.h"
#include "utils.h"
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>

using namespace std;

// using ResponseFunc = const string (*)(HttpRequest &req);
// using RequestFunc = function<void(HttpRequest &req)>;

// Extract the path from a request (example: localhost:4221/test) -> returns
// test

// const char* stateToString(int state) {
//   switch (state) {
//     case 0: return "TYPE";
//     case 1: return "URL";
//     case 2: return "PROTO";
//     case 3: return "FIRST";
//     case 4: return "SECOND";
//     case 5: return "DATA";
//     default: return "UNKNOWN";
//   }
// }

// char *getURLPath(char *buffer, int n) {
//   // Use a static char so it doesn't get deallocated when the function exits
//   // (even though the return type is pointer)
//   static char url[1024];
//   int index = 0;
//   bool isURL = false;

//   for (int i = 0; i < n; i++) {
//     if (isURL && buffer[i] == ' ') {
//       break;
//     } else if (buffer[i] == ' ') {
//       isURL = true;
//     } else if (isURL == true) {
//       url[index++] = buffer[i];
//     }
//   }

//   url[index] = '\0'; // Null-terminate the string, this signals the end of C
//                      // style strings
//   return url;
// }
bool Server::init(const string &port) {
  // Flush after every cout / cerr
  cout << unitbuf;
  cerr << unitbuf;

  // int socket(int domain, int type, int protocol);
  // SOCK_DGRAM - Doesn't require connection to be established, no guarentee for
  // delivery/order/error checking, fixed size. Suitable for zoom meetings,
  // real-time applications, games. SOCK_STREAM - Provides reliable, sequenced
  // packets and error checked packets. It is suitable for applications where
  // data integrity and order are critical, such as HTTP, FTP, and SSH.
  // SOCK_SEQPACKET - Same as SOCK_STREAM but tells you when messages start and
  // end instead of just sending and sending data in one stream. (Maybe?)
  server_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (server_fd < 0) {
    cerr << "Failed to create server socket\n";
    return false;
  }

  // Since the tester restarts your program quite often, setting SO_REUSEADDR
  // ensures that we don't run into 'Address already in use' errors
  int reuse = 1;
  if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) <
      0) {
    cerr << "setsockopt failed\n";
    return false;
  }

  struct sockaddr_in server_addr;
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_port = htons(4221);

  if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) !=
      0) {
    cerr << "Failed to bind to port " + port + "\n";
    return false;
  }

  int connection_backlog = 5;
  if (listen(server_fd, connection_backlog) != 0) {
    cerr << "listen failed\n";
    return false;
  }

  cout << "Binded server on port: " + port + "\n";

  return true;
}
bool Server::start() {
  // This holds the IP address, port number, address type (IPV4/6)
  struct sockaddr_in client_addr;
  // This tells the accept function that the struct is x bytes. It doesn't
  // change on new requests, this is just for the compiler to know
  int client_addr_len = sizeof(client_addr);

  while (true) {
    int client_fd = accept(server_fd, (struct sockaddr *)&client_addr,
                           (socklen_t *)&client_addr_len);
    cout << "Client connected\n";
    try {
      // Create a thread, to handle it and then just run it seperately so the
      // program can continue accepting requests
      // Non static functions need to accept this when creating threads
      thread(&Server::handleClient, this, client_fd).detach();
    } catch (int err) {
      cout << "An error occured with handleClient. Error code: " << err << "\n";
    }
  }
  close(server_fd);
  return true;
}

string replace_all(string str, const string &from, const string &to) {
  size_t start_pos = 0; // Initialize starting position for search
  // Loop as long as the 'from' substring is found
  while ((start_pos = str.find(from, start_pos)) != string::npos) {
    // Replace the found occurrence
    str.replace(start_pos, from.length(), to);
    // Advance the starting position to avoid replacing the newly inserted 'to'
    // string if it contains the 'from' substring itself (e.g., replacing "x"
    // with "yx")
    start_pos += to.length();
  }
  return str; // Return the modified string
}

// I saw an example on Codecrafters doing it this way and I have to say they did
// a good job
void Server::handleClient(int client_fd) {
  const int CHUNK_SIZE = 8192; // 8 KB chunks
  char buffer[CHUNK_SIZE];
  // Parse the HTTP request (Not created by AI, created by me)
  string curr;
  string httpMethod;
  string url;
  string protocol;
  string data;
  unordered_map<string, string> headers;
  enum STATE { TYPE, URL, PROTO, FIRST, SECOND, DATA };
  STATE state = STATE::TYPE;
  pair<string, string> headerData;
  bool shouldContinue = true;
  int totalChars = 0;
  int contentLengthHeader = 0;
  int bodyBytes = 0;
  int leftOver = 0;
  // bool waitForReal = false;

  while (shouldContinue) {
    int bytes_received = recv(client_fd, buffer, CHUNK_SIZE, 0);
    if (bytes_received == 0) {
      cout << "Client closed connection" << endl;
      close(client_fd);
      break;
    } else if (bytes_received == -1) {
      cout << "No more bytes received\n";
      string response = sendString("400 Bad Request", "Socket not recived");
      send(client_fd, response.c_str(), response.length(), 0);
      close(client_fd);
      return;
    }
    for (int i = 0; i < bytes_received; i++) {
      if (leftOver != 0) {
        i += leftOver;
        leftOver = 0;

        // Handle edge cases
        if (i >= bytes_received) {
          break;
        }
      }
      char c = buffer[i];
      if (state == STATE::TYPE) {
        if (c == ' ') {
          httpMethod = curr;
          curr = "";
          state = STATE::URL;
        } else {
          curr += c;
        }
      } else if (state == STATE::URL) {
        if (c == ' ') {
          url = curr;
          curr = "";
          state = STATE::PROTO;
        } else {
          curr += c;
        }
      } else if (state == STATE::PROTO) {
        if (c == '\r') {
          protocol = curr;
          curr = "";
          leftOver++;
          state = STATE::FIRST;
        } else {
          curr += c;
        }
      } else if (state == STATE::FIRST) {
        if (c == '\r') {
          curr = "";
          state = STATE::DATA;
          leftOver++;
          // THIS IS THE CHECKS BEFORE IT READS THE BODY
          if (httpMethod == "GET") {
            shouldContinue = false;
            break;
          }
          // Find content-length header
          auto it = headers.find("Content-Length");
          if (it != headers.end()) {
            contentLengthHeader = stoi(it->second);
            if (contentLengthHeader == 0) {
              data = "";
              shouldContinue = false;
              break;
            }
            cout << "Found content-length: " << contentLengthHeader << '\n';
          }
          // This does add an extra .find() call but we need it for max
          // character checks on specfiic urls
          auto it2 = routeMaxCharacters.find(url);
          if (it2 != routeMaxCharacters.end()) {
            // Check if body is too long based on specified max characters
            if (contentLengthHeader > it2->second) {
              cout << "body too long based on given max characters!\n";
              string response =
                  sendString("413 Payload Too Large",
                             "body too long based on given max characters!");
              send(client_fd, response.c_str(), response.length(), 0);
              close(client_fd);
              return;
            }
          } else {
            // Check if body is too long based on defaultMaxCharacters
            if (contentLengthHeader > defaultMaxCharacters) {
              cout << "body too long!\n";
              string response = sendString("413 Payload Too Large",
                                           "content length is too high!");
              send(client_fd, response.c_str(), response.length(), 0);
              close(client_fd);
              return;
            }
          }
        } else if (c == ':') {
          headerData.first = curr;
          curr = "";
          state = STATE::SECOND;
          leftOver++;
        } else {
          curr += c;
        }
      } else if (state == STATE::SECOND) {
        if (c == '\r') {
          headerData.second = curr;
          headers.insert(headerData);
          curr = "";
          state = STATE::FIRST;
          leftOver++;
        } else {
          curr += c;
        }
      } else if (state == STATE::DATA) {
        // cout << c << '\n';
        curr += c;
        bodyBytes++;
        // printRawString(curr);
        // cout << '\n';
        // cout << bodyBytes << ' ' << contentLengthHeader << '\n';
        // May seem like it can create an infinite loop but there is a check in
        // place if too many characters are read
        if (bodyBytes >= contentLengthHeader) {
          shouldContinue = false;
          break;
        }
      }
    }

    // Just as a precaution, headers shouldn't be longer than 400 characters 90%
    // of the time. So we will reject those because otherwise they could just
    // spam header data
    if (state != STATE::DATA) {
      totalChars += bytes_received;
      totalChars += leftOver;
      if (totalChars > 400) {
        cout << "Read too many chars\n";
        string response = sendString("413 Payload Too Large",
                                     "Too many characters in your request!");
        send(client_fd, response.c_str(), response.length(), 0);
        close(client_fd);
        return;
      }
    }
  }
  // cout << "Outside of loop\n";
  // printRawString(curr);
  data = curr;

  // if (n >= maximumCharacters - 1) {
  //   cout << "Request too large, dropping connection\n";
  //   close(client_fd);
  //   return;
  // }

  cout << "INFO: Request to " << url << '\n';
  cout << httpMethod << " " << protocol << '\n';
  for (const auto &pair : headers) {
    cout << "[" << pair.first << "] = [" << pair.second << "]\n";
  }
  // cout << "Data:\n";
  // cout << data << '\n';

  unordered_map<string, string> extra;
  HttpRequest req = {httpMethod, url, protocol, headers, data, extra};
  try {
    string response = this->handleRequest(req);
    send(client_fd, response.c_str(), response.length(), 0);
    close(client_fd);
  } catch (const exception &e) {
    cerr << "Error " << e.what() << '\n';
    string response =
        sendString("500 Internal Server Error",
                   "An error occurred while handling the request");
    send(client_fd, response.c_str(), response.length(), 0);
    close(client_fd);
  } catch (...) {
    cerr << "Serious error trying to handle request\n";
    string response =
        sendString("500 Internal Server Error",
                   "An error occurred while handling the request");
    send(client_fd, response.c_str(), response.length(), 0);
    close(client_fd);
  }
}

void Server::setDefaultMaxCharacters(int num) { defaultMaxCharacters = num; }

void Server::use(MiddlewareFunc func) {
  currentMiddlewareRoutes = vector<MiddlewareFunc>{func};
}

void Server::use(const vector<MiddlewareFunc> &funcs) {
  currentMiddlewareRoutes = funcs;
}

void Server::get(const string &route, RequestFunc handler, int maxCharacters) {
  getRoutes[route] = handler;
  /* If this exists in the map, tell developer not to do this for more
  performance gain The reason is because we don't want to create more maps and
  more .find() calls for GET, POST and all the methods This check also does not
  matter in performance because this is when the server is SETTING UP not
  actually responding to calls*/
  if (routeMaxCharacters.find(route) != routeMaxCharacters.end()) {
    cerr << "Please do not use the same url for both a GET and POST. It will "
            "help the server run faster and you will run into some issues if "
            "you do not change this!\n";
  }
  // If not specified, use default max characters
  if (maxCharacters == -1) {
    routeMaxCharacters[route] = defaultMaxCharacters;
  } else {
    routeMaxCharacters[route] = maxCharacters;
  }
  updateRouteMap(route);
}

void Server::post(const string &route, RequestFunc handler, int maxCharacters) {
  postRoutes[route] = handler;
  /* If this exists in the map, tell developer not to do this for more
  performance gain The reason is because we don't want to create more maps and
  more .find() calls for GET, POST and all the methods This check also does not
  matter in performance because this is when the server is SETTING UP not
  actually responding to calls*/
  if (routeMaxCharacters.find(route) != routeMaxCharacters.end()) {
    cerr << "Please do not use the same url for both a GET and POST. It will "
            "help the server run faster and you will run into some issues if "
            "you do not change this!\n";
  }
  // If not specified, use default max characters
  if (maxCharacters == -1) {
    routeMaxCharacters[route] = defaultMaxCharacters;
  } else {
    routeMaxCharacters[route] = maxCharacters;
  }
  updateRouteMap(route);
}

/*
  Given:
  > /api/createTeam
  >

  currentMiddlewareRoutesMap: {
    "/api/createteam": protectJWT*,
    "/api/postannoucement": protectJWT*
  }
*/
void Server::updateRouteMap(const string &route) {
  middlewareRoutesMap[route] = currentMiddlewareRoutes;
}

string Server::handleRequest(HttpRequest &req) {
  const string errorMsg = req.url + " not found!";
  // Always allow OPTIONS for CORS preflight
  if (req.method == "OPTIONS") {
    return sendString("200 OK", "");
  }
  // First, handle middleware. This should be efficient because it's just a
  // .find() call which is usually O(1)
  auto it = middlewareRoutesMap.find(req.url);
  if (it != middlewareRoutesMap.end()) {
    const vector<MiddlewareFunc> mfs = it->second;
    for (MiddlewareFunc mf : mfs) {
      if (!mf(req)) {
        return sendString("401 Unauthorized", "Unauthorized");
      }
    }
  }
  // Then handle each request
  if (req.method == "GET") {
    // This just finds the requested URL in the map with routes
    auto it = getRoutes.find(req.url);
    if (it != getRoutes.end()) {
      // Access the function using it->second and call it with the request
      return it->second(req);
    } else {
      return sendString("404 Not Found", errorMsg);
    }
  } else if (req.method == "POST") {
    // This just finds the requested URL in the map with routes
    auto it = postRoutes.find(req.url);
    if (it != postRoutes.end()) {
      // Access the function using it->second and call it with the request
      return it->second(req);
    } else {
      return sendString("404 Not Found", errorMsg);
    }
  } else if (req.method == "OPTIONS") {
    return sendString("200 OK", "");
  }
  return sendString("404 Not Found", errorMsg);
}

string Response::toString() const {
  string response = "HTTP/1.1 " + status + "\r\n";
  response += "Content-Type: " + contentType + "\r\n";

  // Implement CORS
  if (!Global::serverOrigin.empty()) {
    response.append("Access-Control-Allow-Origin: " + Global::serverOrigin +
                    "\r\n");
    response.append("Access-Control-Allow-Methods: GET, POST, OPTIONS\r\n");
    response.append(
        "Access-Control-Allow-Headers: Content-Type, Authorization\r\n");
    response.append("Access-Control-Allow-Credentials: true\r\n");
  }

  response += "Connection: close\r\n";
  response += "Content-Length: " + to_string(body.length()) + "\r\n\r\n";
  response += body;
  return response;
}

// Just for quick sending
string sendString(const string &status, const string &body) {
  string response = "HTTP/1.1 " + status + "\r\n";
  response.append("Content-Type: application/json\r\n");

  // Implement CORS
  if (!Global::serverOrigin.empty()) {
    response.append("Access-Control-Allow-Origin: " + Global::serverOrigin +
                    "\r\n");
    response.append("Access-Control-Allow-Methods: GET, POST, OPTIONS\r\n");
    response.append(
        "Access-Control-Allow-Headers: Content-Type, Authorization\r\n");
    response.append("Access-Control-Allow-Credentials: true\r\n");
  }

  response.append("Connection: close\r\n");
  response.append("Content-Length: " + to_string(body.length()) + "\r\n" +
                  "\r\n");
  response.append(body);

  return response;
}