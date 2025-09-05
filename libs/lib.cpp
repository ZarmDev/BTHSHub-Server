
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
#include <unistd.h>
#include <sys/types.h>
// Socket/linux networking
#include "lib.h"
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include "global.h"
#include "utils.h"

using namespace std;

// using ResponseFunc = const string (*)(HttpRequest &req);
// using RequestFunc = function<void(HttpRequest &req)>;

// Extract the path from a request (example: localhost:4221/test) -> returns
// test
char *getURLPath(char *buffer, int n) {
  // Use a static char so it doesn't get deallocated when the function exits
  // (even though the return type is pointer)
  static char url[1024];
  int index = 0;
  bool isURL = false;

  for (int i = 0; i < n; i++) {
    if (isURL && buffer[i] == ' ') {
      break;
    } else if (buffer[i] == ' ') {
      isURL = true;
    } else if (isURL == true) {
      url[index++] = buffer[i];
    }
  }

  url[index] = '\0'; // Null-terminate the string, this signals the end of C
                     // style strings
  return url;
}
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
  char buffer[maximumCharacters];
  memset(buffer, 0, maximumCharacters);

  // First read to get headers
  int n = read(client_fd, buffer, maximumCharacters - 1);
  if (n <= 0) {
    cout << "ERROR reading from socket\n";
    close(client_fd);
    return;
  }
  
  buffer[n] = '\0';
  
  // Parse headers to find Content-Length (created using AI after oroblems with reading a fixed length)
  string headers_str(buffer, n);
  size_t header_end = headers_str.find("\r\n\r\n");
  int content_length = 0;
  
  if (header_end != string::npos) {
    // Look for Content-Length header
    size_t cl_pos = headers_str.find("Content-Length:");
    if (cl_pos != string::npos && cl_pos < header_end) {
      size_t cl_start = cl_pos + 15; // "Content-Length:" length
      size_t cl_end = headers_str.find("\r\n", cl_start);
      if (cl_end != string::npos) {
        string cl_value = headers_str.substr(cl_start, cl_end - cl_start);
        // Trim whitespace
        cl_value.erase(0, cl_value.find_first_not_of(" \t"));
        cl_value.erase(cl_value.find_last_not_of(" \t") + 1);
        content_length = stoi(cl_value);
      }
    }
    
    // Calculate how much body data we already have
    int headers_length = header_end + 4; // +4 for "\r\n\r\n"
    int body_received = n - headers_length;
    
    // Read remaining body data if needed
    while (body_received < content_length && n < maximumCharacters - 1) {
      int additional = read(client_fd, buffer + n, maximumCharacters - 1 - n);
      if (additional <= 0) break;
      n += additional;
      body_received += additional;
    }
    
    buffer[n] = '\0';
  }

  string bufferStr(buffer, n);
  
  if (n >= maximumCharacters - 1) {
    cout << "Request too large, dropping connection\n";
    close(client_fd);
    return;
  }

  // Parse the HTTP request (Not created by AI, created by me)
  string curr;
  string httpMethod;
  string url;
  string protocol;
  string data;
  unordered_map<string, string> headers;
  enum STATE { TYPE, URL, PROTO, HEADER_NAME, HEADER_VALUE, BODY };
  STATE state = STATE::TYPE;
  string headerName;
  
  for (int i = 0; i < n; ++i) {
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
      if (c == '\r' && i + 1 < n && buffer[i + 1] == '\n') {
        protocol = curr;
        curr = "";
        i++; // Skip the \n
        state = STATE::HEADER_NAME;
      } else {
        curr += c;
      }
    } else if (state == STATE::HEADER_NAME) {
      if (c == '\r' && i + 1 < n && buffer[i + 1] == '\n') {
        // Empty line means end of headers
        i++; // Skip the \n
        state = STATE::BODY;
      } else if (c == ':') {
        headerName = curr;
        curr = "";
        state = STATE::HEADER_VALUE;
        // Skip the space after colon if present
        if (i + 1 < n && buffer[i + 1] == ' ') {
          i++;
        }
      } else {
        curr += c;
      }
    } else if (state == STATE::HEADER_VALUE) {
      if (c == '\r' && i + 1 < n && buffer[i + 1] == '\n') {
        headers[headerName] = curr;
        curr = "";
        i++; // Skip the \n
        state = STATE::HEADER_NAME;
      } else {
        curr += c;
      }
    } else if (state == STATE::BODY) {
      data += c;
    }
  }

  writeToFile("output.txt", bufferStr);

  cout << "INFO: Request to " << url << '\n';
  cout << httpMethod << " " << protocol << '\n';
  for (const auto &pair : headers) {
    cout << "[" << pair.first << "] = [" << pair.second << "]\n";
  }

  unordered_map<string, string> extra;
  HttpRequest req = {httpMethod, url, protocol, headers, data, extra};
  try {
    string response = this->handleRequest(req);
    send(client_fd, response.c_str(), response.length(), 0);
    close(client_fd);
  } catch (const exception& e) {
    cerr << "Error " << e.what() << '\n';
    string response = sendString("500 Internal Server Error", "An error occurred while handling the request");
    send(client_fd, response.c_str(), response.length(), 0);
    close(client_fd);
  } catch (...) {
    cerr << "Serious error trying to handle request\n";
    string response = sendString("500 Internal Server Error", "An error occurred while handling the request");
    send(client_fd, response.c_str(), response.length(), 0);
    close(client_fd);
  }
}

void Server::setMaxCharacters(int num) {
  maximumCharacters = num;
}

void Server::use(MiddlewareFunc func) {
  currentMiddlewareRoutes = vector<MiddlewareFunc>{func};
}

void Server::use(const vector<MiddlewareFunc> &funcs) {
  currentMiddlewareRoutes = funcs;
}

void Server::get(const string &route, RequestFunc handler) {
  getRoutes[route] = handler;
  updateRouteMap(route);
}

void Server::post(const string &route, RequestFunc handler) {
  postRoutes[route] = handler;
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
void Server::updateRouteMap(const string& route) {
  middlewareRoutesMap[route] = currentMiddlewareRoutes;
}

string Server::handleRequest(HttpRequest &req) {
  const string errorMsg = req.url + " not found!";
  // Always allow OPTIONS for CORS preflight
  if (req.method == "OPTIONS") {
    return sendString("200 OK", "");;
  }
  // First, handle middleware. This should be efficient because it's just a .find() call which is usually O(1)
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

// TODO: parse multipart/form-data
/*
POST /upload HTTP/1.1
Content-Type: multipart/form-data; boundary=----WebKitFormBoundaryX

------WebKitFormBoundaryX
Content-Disposition: form-data; name="file"; filename="myfile.pdf"
Content-Type: application/pdf

(binary PDF data here)
------WebKitFormBoundaryX--
*/


string Response::toString() const {
  string response = "HTTP/1.1 " + status + "\r\n";
  response += "Content-Type: " + contentType + "\r\n";

  // Implement CORS
  if (!Global::serverOrigin.empty()) {
    response.append("Access-Control-Allow-Origin: " + Global::serverOrigin + "\r\n");
    response.append("Access-Control-Allow-Methods: GET, POST, OPTIONS\r\n");
    response.append("Access-Control-Allow-Headers: Content-Type, Authorization\r\n");
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
    response.append("Access-Control-Allow-Origin: " + Global::serverOrigin + "\r\n");
    response.append("Access-Control-Allow-Methods: GET, POST, OPTIONS\r\n");
    response.append("Access-Control-Allow-Headers: Content-Type, Authorization\r\n");
    response.append("Access-Control-Allow-Credentials: true\r\n");
  }

  response.append("Connection: close\r\n");
  response.append("Content-Length: " + to_string(body.length()) + "\r\n" +
                  "\r\n");
  response.append(body);
  
  return response;
}