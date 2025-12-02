#include "CampusCompass.h"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <functional>
#include <iostream>
#include <limits>
#include <queue>
#include <regex>
#include <sstream>
#include <unordered_set>

using namespace std;

CampusCompass::CampusCompass() {
    // initialize your object
}

bool CampusCompass::ParseCSV(const string &edges_filepath, const string &classes_filepath) {
    ifstream fe(edges_filepath);
    if (!fe.is_open()) {
        return false;
    }

    string line;
    if (!getline(fe, line)) {
        fe.close();
        return false;
    }

    while (getline(fe, line)) {
        if (line.empty()) continue;

        stringstream ss(line);
        string loc1s, loc2s, name1, name2, times;

        if (!getline(ss, loc1s, ',')) continue;
        if (!getline(ss, loc2s, ',')) continue;
        if (!getline(ss, name1, ',')) continue;
        if (!getline(ss, name2, ',')) continue;
        if (!getline(ss, times, ',')) continue;
        auto trim = [](string &s) {
            size_t a = s.find_first_not_of(" \t\r\n");
            size_t b = s.find_last_not_of(" \t\r\n");
            if (a == string::npos) {
                s.clear();
            } else {
                s = s.substr(a, b - a + 1);
            }
        };
        trim(name1);
        trim(name2);

        int loc1 = stoi(loc1s);
        int loc2 = stoi(loc2s);
        int t    = stoi(times);

        locationNames_[loc1] = name1;
        locationNames_[loc2] = name2;

        Edge e1{loc2, t, true};
        Edge e2{loc1, t, true};
        adj_[loc1].push_back(e1);
        adj_[loc2].push_back(e2);
    }
    fe.close();

    ifstream fc(classes_filepath);
    if (!fc.is_open()) {
        return false;
    }

    if (!getline(fc, line)) {
        fc.close();
        return false;
    }

    while (getline(fc, line)) {
        if (line.empty()) continue;

        stringstream ss(line);
        string code, locs, start, end;
        if (!getline(ss, code, ',')) continue;
        if (!getline(ss, locs, ',')) continue;
        getline(ss, start, ',');
        getline(ss, end, ',');

        auto trim = [](string &s) {
            size_t a = s.find_first_not_of(" \t\r\n");
            size_t b = s.find_last_not_of(" \t\r\n");
            if (a == string::npos) {
                s.clear();
            } else {
                s = s.substr(a, b - a + 1);
            }
        };
        trim(code);
        trim(locs);
        trim(start);
        trim(end);

        if (code.empty() || locs.empty()) continue;

        int loc  = stoi(locs);
        int smin = TimeToMins(start);
        int emin = TimeToMins(end);

        ClassInfo info{loc, smin, emin};
        classes_[code] = info;
    }
    fc.close();

    return true;
}

int CampusCompass::TimeToMins(const string &t) const {
    if (t.empty()) return -1;
    size_t pos = t.find(':');
    if (pos == string::npos) return -1;
    string hs = t.substr(0, pos);
    string ms = t.substr(pos + 1);
    if (hs.empty() || ms.empty()) return -1;
    int h = stoi(hs);
    int m = stoi(ms);
    if (h < 0 || h > 23 || m < 0 || m > 59) return -1;
    return h * 60 + m;
}

void CampusCompass::Dijkstra(int source, unordered_map<int, int> &dist, unordered_map<int, int> &parent) const {
    dist.clear();
    parent.clear();

    using P = pair<int, int>;
    priority_queue<P, vector<P>, greater<P>> pq;

    dist[source] = 0;
    pq.push({0, source});

    while (!pq.empty()) {
        auto [d, u] = pq.top();
        pq.pop();

        if (dist[u] != d) continue;

        auto it = adj_.find(u);
        if (it == adj_.end()) continue;

        for (const Edge &e : it->second) {
            if (!e.open) continue;

            int v  = e.to;
            int nd = d + e.weight;

            if (!dist.count(v) || nd < dist[v]) {
                dist[v] = nd;
                parent[v] = u;
                pq.push({nd, v});
            }
        }
    }
}

int CampusCompass::ShortestTime(int start, int goal) const {
    unordered_map<int, int> dist, parent;
    Dijkstra(start, dist, parent);

    auto it = dist.find(goal);
    return (it == dist.end()) ? -1 : it->second;
}

bool CampusCompass::InsertStudent(const string &name, const string &ufid, int residence, const vector<string> &classCodes) {
    if (students_.find(ufid) != students_.end()) {
        cout << "unsuccessful" << endl;
        return false;
    }
    if (!locationNames_.count(residence)) {
        cout << "unsuccessful" << endl;
        return false;
    }
    for (const string &c : classCodes) {
        if (!classes_.count(c)) {
            cout << "unsuccessful" << endl;
            return false;
        }
    }
    students_[ufid] = {name, ufid, residence, classCodes};
    cout << "successful" << endl;
    return true;
}

bool CampusCompass::RemoveStudent(const string &ufid) {
    if (!students_.count(ufid)) {
        cout << "unsuccessful" << endl;
        return false;
    }
    students_.erase(ufid);
    cout << "successful" << endl;
    return true;
}

bool CampusCompass::DropClass(const string &ufid, const string &code) {
    auto it = students_.find(ufid);
    if (it == students_.end()) {
        cout << "unsuccessful" << endl;
        return false;
    }

    Student &s = it->second;

    bool found = false;
    vector<string> newClasses;
    for (auto &c : s.classes) {
        if (c == code) found = true;
        else newClasses.push_back(c);
    }

    if (!found) {
        cout << "unsuccessful" << endl;
        return false;
    }

    s.classes = move(newClasses);
    if (s.classes.empty()) students_.erase(it);

    cout << "successful" << endl;
    return true;
}

bool CampusCompass::ReplaceClass(const string &ufid, const string &oldCode, const string &newCode) {
    auto it = students_.find(ufid);
    if (it == students_.end()) {
        cout << "unsuccessful" << endl;
        return false;
    }
    if (!classes_.count(oldCode) || !classes_.count(newCode)) {
        cout << "unsuccessful" << endl;
        return false;
    }

    Student &s = it->second;

    bool hasOld = false, hasNew = false;
    for (auto &c : s.classes) {
        if (c == oldCode) hasOld = true;
        if (c == newCode) hasNew = true;
    }

    if (!hasOld || hasNew) {
        cout << "unsuccessful" << endl;
        return false;
    }

    for (auto &c : s.classes) {
        if (c == oldCode) c = newCode;
    }

    cout << "successful" << endl;
    return true;
}

bool CampusCompass::RemoveClass(const string &code) {
    int count = 0;

    for (auto it = students_.begin(); it != students_.end();) {
        Student &s = it->second;
        bool dropped = false;

        vector<string> updated;
        for (auto &c : s.classes) {
            if (c == code) {
                dropped = true;
            } else {
                updated.push_back(c);
            }
        }

        if (dropped) {
            count++;
            s.classes = move(updated);
        }

        if (s.classes.empty()) it = students_.erase(it);
        else ++it;
    }

    cout << count << endl;
    return true;
}

bool CampusCompass::ToggleEdgesClosure(int N, const vector<pair<int,int>> &pairs) {
    for (auto &p : pairs) {
        int u = p.first, v = p.second;

        auto itU = adj_.find(u);
        if (itU != adj_.end()) {
            for (auto &e : itU->second)
                if (e.to == v) e.open = !e.open;
        }
        auto itV = adj_.find(v);
        if (itV != adj_.end()) {
            for (auto &e : itV->second)
                if (e.to == u) e.open = !e.open;
        }
    }
    cout << "successful" << endl;
    return true;
}

bool CampusCompass::CheckEdgeStatus(int u, int v) {
    if (!adj_.count(u)) {
        cout << "DNE" << endl;
        return true;
    }
    for (const auto &e : adj_[u]) {
        if (e.to == v) {
            cout << (e.open ? "open" : "closed") << endl;
            return true;
        }
    }
    cout << "DNE" << endl;
    return true;
}

bool CampusCompass::IsConnected(int u, int v) {
    int t = ShortestTime(u, v);
    cout << ((t == -1) ? "unsuccessful\n": "successful\n");
    return true;
}

bool CampusCompass::PrintShortestEdges(const string &ufid) {
    auto it = students_.find(ufid);
    if (it == students_.end()) {
        cout << "unsuccessful" << endl;
        return false;
    }

    Student &s = it->second;
    unordered_map<int, int> dist, parent;
    Dijkstra(s.residence, dist, parent);

    vector<string> codes = s.classes;
    sort(codes.begin(), codes.end());

    cout << "Name: " << s.name << endl;
    for (auto &code : codes) {
        int time = -1;
        int loc = classes_[code].location;
        if (dist.count(loc)) time = dist[loc];
        cout << code << " | Total Time: " << time << endl;
    }
    return true;
}

bool CampusCompass::PrintStudentZone(const string &ufid) {
    auto it = students_.find(ufid);
    if (it == students_.end()) {
        cout << "unsuccessful" << endl;
        return false;
    }

    Student &s = it->second;
    unordered_map<int, int> dist, parent;
    Dijkstra(s.residence, dist, parent);

    unordered_set<int> verts;
    verts.insert(s.residence);

    for (auto &code : s.classes) {
        int loc = classes_[code].location;
        if (!dist.count(loc)) continue;

        int cur = loc;
        verts.insert(cur);
        while (cur != s.residence) {
            cur = parent[cur];
            verts.insert(cur);
        }
    }

    struct E { int u, v, w; };
    vector<E> edges;

    for (int u : verts) {
        for (auto &e : adj_[u]) {
            int v = e.to;
            if (u < v && verts.count(v) && e.open) {
                edges.push_back({u, v, e.weight});
            }
        }
    }

    vector<int> nodes(verts.begin(), verts.end());
    unordered_map<int, int> idx;
    for (size_t i = 0; i < nodes.size(); ++i) idx[nodes[i]] = i;

    vector<int> parentDSU(nodes.size()), rank(nodes.size());
    for (size_t i = 0; i < parentDSU.size(); ++i) parentDSU[i] = i;

    function<int(int)> find = [&](int x) {
        return parentDSU[x] == x ? x : parentDSU[x] = find(parentDSU[x]);
    };

    auto unite = [&](int a, int b) {
        a = find(a); b = find(b);
        if (a != b) {
            if (rank[a] < rank[b]) swap(a, b);
            parentDSU[b] = a;
            if (rank[a] == rank[b]) rank[a]++;
        }
    };

    sort(edges.begin(), edges.end(),
         [](auto &a, auto &b){ return a.w < b.w; });

    int total = 0;
    for (auto &e : edges) {
        int iu = idx[e.u], iv = idx[e.v];
        if (find(iu) != find(iv)) {
            unite(iu, iv);
            total += e.w;
        }
    }

    cout << "Student Zone Cost For " << s.name << ": " << total << endl;
    return true;
}

bool CampusCompass::VerifySchedule(const string &ufid) {
    auto it = students_.find(ufid);
    if (it == students_.end()) {
        cout << "unsuccessful" << endl;
        return false;
    }

    Student &s = it->second;

    struct S { string code; int loc, start, end; };
    vector<S> arr;

    for (auto &code : s.classes) {
        ClassInfo &ci = classes_[code];
        if (ci.start != -1 && ci.end != -1)
            arr.push_back({code, ci.location, ci.start, ci.end});
    }

    if (arr.size() < 2) {
        cout << "unsuccessful" << endl;
        return false;
    }

    sort(arr.begin(), arr.end(),
         [](auto &a, auto &b){ return a.start < b.start; });

    cout << "Schedule Check for " << s.name << ":" << endl;

    for (size_t i = 0; i < arr.size() - 1; ++i) {
        auto &A = arr[i];
        auto &B = arr[i+1];

        int gap = B.start - A.end;
        int shortest = ShortestTime(A.loc, B.loc);

        cout << A.code << " - " << B.code << " ";
        if (shortest == -1 || gap < shortest)
            cout << "\"Cannot make it!\"" << endl;
        else
            cout << "\"Can make it!\"" << endl;
    }

    return true;
}

bool CampusCompass::ParseCommand(const string &command) {
    // do whatever regex you need to parse validity
    string line = command;
    size_t a = line.find_first_not_of(" \t\r\n");
    if (a == string::npos) {
        cout << "unsuccessful" << endl;
        return false;
    }
    size_t b = line.find_last_not_of(" \t\r\n");
    line = line.substr(a, b - a + 1);

    smatch m;

    {
        static regex r(R"regex(^insert\s+"([A-Za-z ]+)"\s+(\d{8})\s+(\d+)\s+([1-6])((\s+[A-Z]{3}\d{4}){1,6})\s*$)regex");
        if (regex_match(line, m, r)) {
            string name = m[1];
            string ufid = m[2];
            int residence = stoi(m[3]);
            int N = stoi(m[4]);
            string rest = m[5];

            vector<string> codes;
            stringstream ss(rest);
            string c;
            while (ss >> c) codes.push_back(c);

            if (static_cast<int>(codes.size()) != N) {
                cout << "unsuccessful" << endl;
                return false;
            }

            return InsertStudent(name, ufid, residence, codes);
        }
    }

    {
        static regex r(R"(^remove\s+(\d{8})\s*$)");
        if (regex_match(line, m, r))
            return RemoveStudent(m[1]);
    }

    {
        static regex r(R"(^dropClass\s+(\d{8})\s+([A-Z]{3}\d{4})\s*$)");
        if (regex_match(line, m, r))
            return DropClass(m[1], m[2]);
    }

    {
        static regex r(R"(^replaceClass\s+(\d{8})\s+([A-Z]{3}\d{4})\s+([A-Z]{3}\d{4})\s*$)");
        if (regex_match(line, m, r))
            return ReplaceClass(m[1], m[2], m[3]);
    }

    {
        static regex r(R"(^removeClass\s+([A-Z]{3}\d{4})\s*$)");
        if (regex_match(line, m, r))
            return RemoveClass(m[1]);
    }

    {
        static regex r(R"(^toggleEdgesClosure\s+(\d+)(.*)$)");
        if (regex_match(line, m, r)) {
            int N = stoi(m[1]);
            vector<pair<int,int>> pairs;
            stringstream ss(m[2].str());
            int a, b;
            while (ss >> a >> b) pairs.emplace_back(a, b);
            if (static_cast<int>(pairs.size()) != N) {
                cout << "unsuccessful" << endl;
                return false;
            }
            return ToggleEdgesClosure(N, pairs);
        }
    }

    {
        static regex r(R"(^checkEdgeStatus\s+(\d+)\s+(\d+)\s*$)");
        if (regex_match(line, m, r))
            return CheckEdgeStatus(stoi(m[1]), stoi(m[2]));
    }

    {
        static regex r(R"(^isConnected\s+(\d+)\s+(\d+)\s*$)");
        if (regex_match(line, m, r))
            return IsConnected(stoi(m[1]), stoi(m[2]));
    }

    {
        static regex r(R"(^printShortestEdges\s+(\d{8})\s*$)");
        if (regex_match(line, m, r))
            return PrintShortestEdges(m[1]);
    }

    {
        static regex r(R"(^printStudentZone\s+(\d{8})\s*$)");
        if (regex_match(line, m, r))
            return PrintStudentZone(m[1]);
    }

    {
        static regex r(R"(^verifySchedule\s+(\d{8})\s*$)");
        if (regex_match(line, m, r))
            return VerifySchedule(m[1]);
    }

    cout << "unsuccessful" << endl;
    return false;
}
