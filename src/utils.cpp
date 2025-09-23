#include "utils.h"
#include "global.h"
#include "lib.h"
#include <cstring>
#include <fstream>
#include <iostream>
#include <ostream>
#include <sstream>
#include <stdio.h>
#include <string>
#include <vector>

#define redis Global::db

using namespace std;

string readFile(const string &path) {
  ifstream file(path, ios::binary);
  if (!file.is_open())
    return "";
  ostringstream ss;
  ss << file.rdbuf();
  return ss.str();
}

void writeToFile(const string &path, const string &data) {
  ofstream outFile(path, ios::out | ios::binary);
  if (outFile.is_open()) {
    outFile << data;
    outFile.close();
  }
}

// https://stackoverflow.com/questions/16286095/similar-function-to-javas-string-split-in-c
vector<string> split(string str, string sep) {
  try {
    char *cstr = const_cast<char *>(str.c_str());
    char *current;
    vector<string> arr;
    current = strtok(cstr, sep.c_str());
    while (current != NULL) {
      arr.push_back(current);
      current = strtok(NULL, sep.c_str());
    }
    return arr;
  } catch (...) {
    return {str};
  }
}

void printAllRedisKeys() {
  // Initialize cursor and key container
  long long cursor = 0;
  vector<string> keys;

  // Scan loop
  do {
    cursor = redis->scan(cursor, "*", 100, back_inserter(keys));
  } while (cursor != 0);

  // Print all keys
  cout << "Redis Keys:" << endl;
  for (const auto &key : keys) {
    cout << key << endl;
  }
}

string removeWhitespace(const string &str) {
  string newStr;
  for (int i = 0; i < str.length(); i++) {
    if (str.at(i) != ' ') {
      newStr += str.at(i);
    }
  }
  return newStr;
}

// only for the surrounding whitespace
string trim(const string &str) {
  string newStr;
  int i = 0;
  while (str.at(i) == ' ') {
    i++;
  }
  int startIdx = i;

  i = str.length() - 1;
  while (str.at(i) == ' ') {
    i--;
  }
  return str.substr(startIdx, i - startIdx + 1);
}

// example: 5admin8password
// vector<string> parseLengthPrefixed(const string& data) {
//   vector<string> result;
//   enum STATE {
//     LENGTH,
//     CONTENT
//   };
//   STATE state = STATE::LENGTH;
//   int length;
//   string content;
//   for (int i = 0; i < data.length(); i++) {
//     if (state == STATE::LENGTH) {
//       length = stoi(data);
//     }
//   }
// }

const string getValueFromMiddleware(HttpRequest &req, const string &value) {
  auto it = req.extra.find(value);
  if (it == req.extra.end()) {
    cerr << "Could not locate \"" << value << "\"\n";
    return "";
  }
  const string &found = it->second;
  return found;
}

// Generated with AI
optional<UploadedFile> extractPdfFromRequest(const HttpRequest &req) {
  writeToFile("output.txt", req.data);
  // Check if it's multipart/form-data
  auto it = req.headers.find("Content-Type");
  if (it == req.headers.end() ||
      it->second.find("multipart/form-data") == string::npos) {
        cout << "No multipart...\n";
    return nullopt;
  }

  // Get boundary
  string contentType = it->second;
  size_t pos = contentType.find("boundary=");
  if (pos == string::npos) {
    cout << "2\n";
    return nullopt;
  }

  string boundary = "--" + contentType.substr(pos + 9);

  // Find PDF content in the multipart data
  pos = req.data.find(boundary);
  if (pos == string::npos) {
    cout << "3\n";
    return nullopt;
  }

  // Find content disposition header
  pos = req.data.find("Content-Disposition:", pos);
  if (pos == string::npos) {
    // Fallback to the lower casing (WHY JAVASCRIPT!!!!)
    pos = req.data.find("content-disposition:");
    if (pos == string::npos) {
      cout << "4\n";
      return nullopt;
    }
  }

  // Get filename
  pos = req.data.find("filename=\"", pos);
  if (pos == string::npos) {
    cout << "5\n";
    return nullopt;
  }
  pos += 10; // Skip "filename=\""
  size_t end_pos = req.data.find("\"", pos);
  string filename = req.data.substr(pos, end_pos - pos);

  // Find PDF content type
  pos = req.data.find("Content-Type:", end_pos);
  if (pos == string::npos) {
    cout << "6\n";
    return nullopt;
  }

  pos += 13;                                  // Skip "Content-Type:"
  pos = req.data.find_first_not_of(" ", pos); // Skip whitespace
  end_pos = req.data.find("\r\n", pos);
  string content_type = req.data.substr(pos, end_pos - pos);

  // Find actual PDF data
  pos = req.data.find("\r\n\r\n", end_pos);
  if (pos == string::npos) {
    cout << "8\n";
    return nullopt;
  }
  pos += 4; // Skip "\r\n\r\n"

  // Find end of data (next boundary)
  end_pos = req.data.find(boundary, pos);
  if (end_pos == string::npos)
    return nullopt;
  end_pos -= 2; // Account for \r\n before boundary

  // Extract the PDF data
  vector<unsigned char> pdf_data(req.data.begin() + pos,
                                 req.data.begin() + end_pos);

  return UploadedFile{filename, content_type, pdf_data};
}