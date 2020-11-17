#pragma GCC target("avx")
#pragma GCC optimize("O3")
#pragma GCC optimize("omit-frame-pointer")
#pragma GCC optimize("unsafe-math-optimizations")
#pragma GCC optimize("unroll-all-loops")
#pragma GCC optimize("inline")

#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <iostream>
#include <map>
#include <queue>
#include <set>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

using namespace std;

struct Point {
  int x, y;
  inline Point() = default;
  inline Point(int x, int y) : x(x), y(y) {}
  inline Point(Point const &) = default;
  inline Point(Point &&) = default;

  inline bool adjacent(const Point &p) {
    if (p.x == x and p.y == y) return false;
    return (p.x == x - 1 or p.x == x or p.x == x + 1) and (p.y == y - 1 or p.y == y or p.y == y + 1);
  }

  inline Point &operator=(Point const &) = default;
  inline Point &operator=(Point &&) = default;
  inline bool operator==(const Point &c) const { return this->x == c.x && this->y == c.y; }
  inline bool operator!=(const Point &c) const { return !((*this) == c); }
  inline bool operator<(const Point &c) const { return this->x + 8 * this->y < c.x + 8 * c.y; }
  inline Point operator+(const Point &c) const { return Point(this->x + c.x, this->y + c.y); }
  inline Point operator-(const Point &c) const { return Point(this->x - c.x, this->y - c.y); }
  virtual void serialize(ostream &os) const { os << " [" << x << "," << y << "]"; }
};
ostream &operator<<(ostream &os, const Point &c) {
  c.serialize(os);
  return os;
}

namespace std {
template <>
struct hash<Point> {
  inline size_t operator()(const Point &p) const { return p.x * 1812433253 + p.y; }
};
}  // namespace std

struct Query {
  int award;
  string item;
  vector<string> items;

  inline Query() = default;

  inline void split() {
    string token;
    istringstream tokenStream(item);
    while (getline(tokenStream, token, '-')) {
      items.push_back(token);
    }
  }
  inline bool contains(const string &name) { return find(items.begin(), items.end(), name) != items.end(); }
  inline void remove(const string &name) { items.erase(find(items.begin(), items.end(), name)); }
};

static inline bool q_compare(Query q1, Query q2) { return q1.award < q2.award; }

struct Unit : public Point, public Query {
  bool carries, dropped;
  Point precious;
  inline Unit() : carries(false), dropped(false) {}
};

struct Action {
  string action;
  Point pos;
  string message;

  inline Action() : action("WAIT"), pos(-1, -1), message("") {}

  inline void play() const {
    string out(action);
    if (pos.x >= 0) {
      out += " " + to_string(pos.x) + " " + to_string(pos.y);
    }
    out += ";" + message;
    cout << out << endl;
  }
};

struct Board {
  int width, height, nbCustomers, turns, nbTables, timer;
  string oven;
  vector<Query> customerQueries;
  array<Unit, 2> cooks;
  array<Unit, 10> tables;
  Point window, chop;
  Unit dishwasher;
  unordered_set<Point> walls, etables;
  map<char, Point> crates;
  Query current, order;
  Action action;
  const array<Point, 4> cross{Point{1, 0}, Point{0, -1}, Point{-1, 0}, Point{0, 1}};
  const array<Point, 8> rect{
      Point{1, 0}, Point{1, 1}, Point{0, 1}, Point{-1, 1}, Point{-1, 0}, Point{-1, -1}, Point{0, -1}, Point{1, -1}};
  map<string, char> reserve{{"BLUEBERRIES", 'B'}, {"ICE_CREAM", 'I'}, {"CHOPPED_STRAWBERRIES", 'S'}};

  Board() {
    width = 11;
    height = 7;
    cin >> nbCustomers;
    cin.ignore();
    for (int i(0); i < nbCustomers; ++i) {
      Query q;
      cin >> q.item >> q.award;
      cin.ignore();
      q.split();
      customerQueries.push_back(q);
    }
    for (int j(0); j < 7; j++) {
      string row;
      getline(cin, row);
      for (int i(0); i < row.size(); ++i) {
        switch (row[i]) {
          case 'D':
            dishwasher.x = i;
            dishwasher.y = j;
            break;
          case 'W':
            window.x = i;
            window.y = j;
            break;
          case 'C':
            chop.x = i;
            chop.y = j;
            break;
          case 'I':
          case 'B':
          case 'S':
            crates[row[i]] = Point(i, j);
            break;
          case '#':
            etables.insert(Point(i, j));
            break;
          case '.':
          case '0':
          case '1':
            continue;
          default:
            break;
        }
        walls.insert(Point(i, j));
      }
    }
    current.award = 0;
  }

  void init() {
    cin >> turns;
    cin.ignore();
    for (int i(0); i < 2; ++i) {
      cin >> cooks[i].x >> cooks[i].y >> cooks[i].item;
      cin.ignore();
      cooks[i].split();
    }
    cin >> nbTables;
    cin.ignore();
    for (int i = 0; i < nbTables; i++) {
      cin >> tables[i].x >> tables[i].y >> tables[i].item;
      cin.ignore();
      tables[i].split();
    }
    cin >> oven >> timer;
    cin.ignore();
    cin >> nbCustomers;
    cin.ignore();
    customerQueries.clear();
    for (int i = 0; i < nbCustomers; i++) {
      Query q;
      cin >> q.item >> q.award;
      cin.ignore();
      q.split();
      customerQueries.push_back(q);
    }
  }

  inline bool inKitchen(Point p) const { return 0 <= p.x && p.x < width && 0 <= p.y && p.y < height; }
  inline bool isWall(Point id) const { return walls.count(id); }
  vector<Point> neighbors(Point &p) const {
    vector<Point> results;
    for (auto d : cross) {
      Point next(p.x + d.x, p.y + d.y);
      if (inKitchen(next) and !isWall(next)) {
        results.push_back(next);
      }
    }
    return results;
  }
  Point emptyTableAround() {
    for (auto r : rect) {
      Point zone(cooks[0].x + r.x, cooks[0].y + r.y);
      if (etables.count(zone)) {
        return zone;
      }
    }
    return Point{-1, -1};
  }

  unordered_map<Point, int> bfs(Point &start) {
    queue<Point> frontier;
    frontier.push(start);

    unordered_map<Point, int> path;
    path[start] = 0;

    while (!frontier.empty()) {
      auto current = frontier.front();
      frontier.pop();

      for (auto next : neighbors(current)) {
        if (!path.count(next)) {
          frontier.push(next);
          path[next] = 1 + path[current];
        }
      }
    }
    path.erase(start);
    return path;
  }

  Query selectCustomer() const {
    auto res = max_element(customerQueries.begin(), customerQueries.end(), q_compare);
    cerr << "max " << (*res).item << endl;
    return (*res);
  }

  void go_simple(const Point &cible, const string &use, const string &move, const string &name = "none") {
    cerr << " go " << use << ";" << name << " " << cooks[0].carries << cooks[0].dropped << " " << cooks[0].precious
         << endl;
    action.pos = cible;
    if (cooks[0].adjacent(cible)) {
      action.action = "USE";
      action.message = use;
      if (name == "none") {
        order.items.clear();
        current.award = 0;
      } else {
        cerr << "insert " << name << " in order" << endl;
        order.items.push_back(name);
      }
      if (name == "DISH") {
        cooks[0].carries = true;
      }
    } else {
      action.action = "MOVE";
      action.message = move;
    }
  }
  void go_chop() {
    // porte un truc
    if (cooks[0].carries) {
      // pas dropped: porte une assiette
      if (!cooks[0].dropped) {
        if (!cooks[0].adjacent(crates['S'])) {
          action.pos = crates['S'];
        } else {
          Point libre = emptyTableAround();
          if (libre.x >= 0) {
            action.pos = libre;
            cooks[0].precious = libre;
            cooks[0].dropped = true;
            cooks[0].carries = false;
          }
          order.items.clear();
        }
      } else {
        action.pos = chop;
      }
    } else {
      if (cooks[0].adjacent(chop)) {
        action.pos = cooks[0].precious;
      } else {
        action.pos = crates['S'];
      }
    }
    if (cooks[0].adjacent(action.pos)) {
      action.action = "USE";
      action.message = " use";
      if (action.pos == crates['S']) {
        cooks[0].carries = true;
      }
    } else {
      action.action = "MOVE";
      action.message = " move";
    }
  }
  void moteur() {
    if (current.award == 0) {
      current = selectCustomer();
    }
    cerr << current.item << " " << current.items.size() << endl;
    cerr << " order size " << order.items.size() << endl;
    if (order.items.size() == current.items.size()) {
      go_simple(window, "service", "livraison");
    } else {
      if (order.items.size() == 0 and !cooks[0].dropped) {
        go_simple(dishwasher, "assiette", "parti", "DISH");
      } else {
        for (auto i : current.items) {
          if (!order.contains(i)) {
            if (i == "CHOPPED_STRAWBERRIES") {
              go_chop();
            } else {
              go_simple(crates[reserve[i]], i, i, i);
            }
            break;
          }
        }
      }
    }
  }

  inline void play() { action.play(); }
};
/**
 * Auto-generated code below aims at helping you parse
 * the standard input according to the problem statement.
 **/
int main() {
  Board board;

  // game loop
  while (1) {
    board.init();
    board.moteur();
    board.play();
  }
}