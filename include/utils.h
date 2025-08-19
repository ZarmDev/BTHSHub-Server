// Use header guard to prevent linker issues in server.cpp
#pragma once

#include <string>
#include <vector>
using namespace std;

string readFile(const string &path);
void writeToFile(const string &path, const string &data);
vector<string> split(string str,string sep);
void printAllRedisKeys();
string removeWhitespace(const string& str);
string trim(const string& str);

// void printSet(unordered_set<string> const &s);
// void printMap(const unordered_map<string, string>& m);
// All the below template/other code is generated using AI. I have no clue how this works
// Check if a type is a map-like container
template <typename T>
concept MapLike = requires(T t) {
    typename T::key_type;
    typename T::mapped_type;
    { t.begin()->first };
    { t.begin()->second };
};

// Check if a type is a sequence container
template <typename T>
concept SequenceContainer = requires(T t) {
    typename T::value_type;
    { t.begin() };
    { t.end() };
} && !MapLike<T>;
// Generic print function that handles any container type
template <typename Container>
void printContainer(const Container& container) {
    std::cout << "[ ";
    
    if constexpr (MapLike<Container>) {
        // Map-like container (unordered_map, map)
        for (const auto& [key, value] : container) {
            std::cout << key << ": " << value << ", ";
        }
    } else {
        // Sequence-like container (vector, set, list)
        bool first = true;
        for (const auto& item : container) {
            if (!first) std::cout << ", ";
            std::cout << item;
            first = false;
        }
    }
    
    std::cout << " ]" << std::endl;
}

// For efficient checking of the first x characters (generated using AI)
// This is declared in the header file because C++ doesn't let you do it inline otherwise
inline bool startsWith(const string &str, const string &prefix)
{
  return str.size() >= prefix.size() && str.compare(0, prefix.size(), prefix) == 0;
}