#include <iterator>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <ostream>

using namespace std;

string readFile(const string &path)
{
  ifstream file(path, ios::binary);
  if (!file.is_open())
    return "";
  ostringstream ss;
  ss << file.rdbuf();
  return ss.str();
}

void writeToFile(const string &path, const string &data)
{
  ofstream outFile(path, ios::out | ios::binary);
  if (outFile.is_open())
  {
    outFile << data;
    outFile.close();
  }
}

vector<string> split(const string& input, char delimiter) {
    vector<string> result;
    stringstream ss(input);
    string item;

    while (getline(ss, item, delimiter)) {
      result.push_back(item);
    }

    return result;
}


void printSet(unordered_set<string> const &s)
{
    copy(s.begin(),
            s.end(),
            ostream_iterator<string>(cout, " "));
}

void printMap(const unordered_map<string, string>& m) 
{
    for (const auto& [key, value] : m) {
        cout << key << ": " << value << "\n";
    }
}