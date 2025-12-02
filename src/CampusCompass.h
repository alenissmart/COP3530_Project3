#pragma once
#include <string>
#include <unordered_map>
#include <vector>

using namespace std;

struct Edge {
    int to;
    int weight;
    bool open;
};

struct ClassInfo {
    int location;
    int start;
    int end;
};

struct Student {
    string name;
    string ufid;
    int residence;
    vector<string> classes;
};

class CampusCompass {
private:
    // Think about what member variables you need to initialize
    // perhaps some graph representation?
    unordered_map<int, vector<Edge>> adj_;
    unordered_map<int, string> locationNames_;
    unordered_map<string, ClassInfo> classes_;
    unordered_map<string, Student> students_;

    int TimeToMins(const string &t) const;

    void Dijkstra(int source, unordered_map<int, int> &dist, unordered_map<int, int> &parent) const;
    int ShortestTime(int start, int goal) const;

    bool InsertStudent(const string &name, const string &ufid, int residence, const vector<string> &classCodes);

    bool RemoveStudent(const string &ufid);

    bool DropClass(const string &ufid, const string &code);

    bool ReplaceClass(const string &ufid, const string &oldCode, const string &newCode);

    bool RemoveClass(const string &code);

    bool ToggleEdgesClosure(int N, const vector<pair<int,int>> &pairs);

    bool CheckEdgeStatus(int u, int v);

    bool IsConnected(int u, int v);

    bool PrintShortestEdges(const string &ufid);

    bool PrintStudentZone(const string &ufid);

    bool VerifySchedule(const string &ufid);
public:
    // Think about what helper functions you will need in the algorithm
    CampusCompass(); // constructor
    bool ParseCSV(const string &edges_filepath, const string &classes_filepath);
    bool ParseCommand(const string &command);
};
