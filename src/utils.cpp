#include <cstring>
#include <fstream>
#include <ostream>
#include <sstream>
#include <stdio.h>
#include <string>
#include <vector>
#include "global.h"
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
  char *cstr = const_cast<char *>(str.c_str());
  char *current;
  vector<string> arr;
  current = strtok(cstr, sep.c_str());
  while (current != NULL) {
    arr.push_back(current);
    current = strtok(NULL, sep.c_str());
  }
  return arr;
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

// void printSet(unordered_set<string> const &s)
// {
//     copy(s.begin(),
//             s.end(),
//             ostream_iterator<string>(cout, " "));
// }

// void printMap(const unordered_map<string, string>& m)
// {
//     for (const auto& [key, value] : m) {
//         cout << key << ": " << value << "\n";
//     }
// }

// void printVector(const vector<string>& v) {

// }
