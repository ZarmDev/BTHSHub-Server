#include <cstring>
#include <fstream>
#include <ostream>
#include <sstream>
#include <stdio.h>
#include <string>
#include <vector>
#include <iostream>
#include "global.h"
#include "lib.h"

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
    cursor = redis.scan(cursor, "*", 100, back_inserter(keys));
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

const string getValueFromMiddleware(HttpRequest &req, const string& value) {
  auto it = req.extra.find(value);
  if (it == req.extra.end()) {
    cerr << "No value found! Something is not right here.\n";
    return "";
  }
  const string& found = it->second;
  return found;
}