#include <string>
#include <fstream>
#include <sstream>
#include <vector>

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