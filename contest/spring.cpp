#pragma GCC optimize("O3")
#pragma GCC optimize("omit-frame-pointer")         // good optimization
#pragma GCC optimize("unsafe-math-optimizations")  // not really useful it seems
#pragma GCC optimize("unroll-all-loops")           // good optimization
#pragma GCC optimize("inline")                     // killer optimization with O3

#include <algorithm>
#include <array>
#include <chrono>
#include <climits>
#include <cmath>
#include <iostream>
#include <map>
#include <memory>
#include <queue>
#include <random>
#include <set>
#include <stack>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

using namespace std;

struct Point {
  int i, x, y;

  inline Point() : i(-1), x(0), y(0) {}
  inline Point(int i, int x, int y) : i(i), x(x), y(y) {}
  inline Point(int x, int y) : i(-1), x(x), y(y) {}
  inline Point(Point const &) = default;
  inline Point(Point &&) = default;

  inline Point &operator=(Point const &) = default;
  inline Point &operator=(Point &&) = default;
  inline bool operator==(const Point &c) const { return this->x == c.x and this->y == c.y; }
  inline bool operator!=(const Point &c) const { return !((*this) == c); }
  inline bool operator<(const Point &c) const { return this->x + 8 * this->y < c.x + 8 * c.y; }
  inline Point operator+(const Point &c) const { return Point(this->x + c.x, this->y + c.y); }
  inline Point operator-(const Point &c) const { return Point(this->x - c.x, this->y - c.y); }

  int dist(const Point &p) { return abs(this->x - p.x) + abs(this->y - p.y); }
  int dist2(const Point &p) { return (this->x - p.x) * (this->x - p.x) + (this->y - p.y) * (this->y - p.y); }

  void norm() {
    x = x / sqrt(x * x + y * y);
    y = y / sqrt(x * x + y * y);
  }

  int scalar(const Point &p) { return this->x * p.x + this->y * p.y; }

  void shift(const Point &p) {
    x = p.x;
    y = p.y;
  }

  virtual void serialize(ostream &os) const { os << "#" << i << " [" << x << "," << y << "]"; }
};
ostream &operator<<(ostream &os, const Point &c) {
  c.serialize(os);
  return os;
}

typedef enum item { NONE = -1, RADAR = 2, TRAP = 3, ORE = 4 } item_t;
struct Cell : Point {
  int value;
  inline Cell() : value(-1), Point{} {}
  virtual void serialize(ostream &os) const {
    Point::serialize(os);
    os << ", value:" << value;
  }
};
ostream &operator<<(ostream &os, const Cell &c) {
  c.serialize(os);
  return os;
}

namespace std {
template <>
struct hash<Point> {
  inline size_t operator()(const Point &p) const { return p.x * 1812433253 + p.y; }
};
}  // namespace std

struct Pac : Point {
  int speed, ability;
  string type, action;
  Point target;

  inline Pac() : Point{}, speed(0), ability(0), type("WAIT"), action("WAIT"), target(Point{-1, 0}) {}

  void move() { action = "MOVE " + to_string(i) + " " + to_string(target.x) + " " + to_string(target.y); }
};

template <typename T, typename priority_t>
struct PriorityQueue {
  typedef pair<priority_t, T> PQElement;
  priority_queue<PQElement, vector<PQElement>, std::greater<PQElement>> elements;

  inline bool empty() const { return elements.empty(); }

  inline void put(T item, priority_t priority) { elements.emplace(priority, item); }

  inline T get() {
    T best_item = elements.top().second;
    elements.pop();
    return best_item;
  }
};

struct Board {
  int width, height, myScore, hisScore, gamePac, gameBullet;
  Cell grid[40][20];
  array<Pac, 5> moi, lui;
  vector<Point> dirs = {Point{0, 0}, Point{0, 1}, Point{0, -1}, Point{1, 0}, Point{-1, 0}};
  unordered_set<Point> walls;
  array<multimap<int, Point>, 5> bullets;

  void init() {
    cin >> width >> height;
    cerr << "init " + to_string(width) + " " + to_string(height) << endl;
    cin.ignore();
    for (int i = 0; i < height; i++) {
      string row;
      getline(cin, row);  // one line of the grid: space " " is floor, pound "#" is wall
      for (int j(0); j < row.size(); ++j) {
        if (row[j] == '#') {
          walls.insert(Point{j, i});
        }
      }
    }
  }

  void turnInit() {
    cerr << "turn init" << endl;
    for (auto &b : bullets) b.clear();
    cin >> myScore >> hisScore;
    cin.ignore();
    cin >> gamePac;
    cin.ignore();
    cerr << "turn init pac" << endl;
    for (int i = 0; i < gamePac; ++i) {
      int id, x, y, s, a;
      bool mine;  // true if this pac is yours
      string t;
      cin >> id >> mine >> x >> y >> t >> s >> a;
      cin.ignore();
      if (mine) {
        moi[id].x = x;
        moi[id].y = y;
        moi[id].i = id;
        moi[id].speed = s;
        moi[id].ability = a;
        moi[id].type = t;
      } else {
        lui[id].x = x;
        lui[id].y = y;
        lui[id].i = id;
        lui[id].speed = s;
        lui[id].ability = a;
        lui[id].type = t;
      }
    }
    cin >> gameBullet;
    cin.ignore();
    for (int i = 0; i < gameBullet; ++i) {
      int x, y, value;
      cin >> x >> y >> value;
      cin.ignore();
      grid[x][y].x = x;
      grid[x][y].y = y;
      grid[x][y].value = value;
      // cerr << "turn init bullet" << endl;
      for (auto m : moi) {
        if (m.i == -1) break;
        cerr << grid[x][y] << " " << m.dist(grid[x][y]) + value << endl;
        bullets[m.i].insert({m.dist(grid[x][y]) - value, Point{x, y}});
      }
    }
    cerr << "turn init end" << endl;
  }

  inline double heuristic(Point a, Point b) { return abs(a.x - b.x) + abs(a.y - b.y); }

  bool passable(const Point &p) { return p.x >= 0 and p.x < width and p.y >= 0 and p.y < height and !walls.count(p); }

  vector<Point> neighbors(const Point &p) {
    vector<Point> res;
    for (auto d : dirs) {
      Point n(p.x + d.x, p.y + d.y);
      if (passable(n)) res.push_back(n);
    }
    return res;
  }

  template <typename Location>
  std::unordered_map<Location, Location> bfs(Location start) {
    std::queue<Location> frontier;
    frontier.push(start);

    std::unordered_map<Location, Location> came_from;
    came_from[start] = start;

    while (!frontier.empty()) {
      Location current = frontier.front();
      frontier.pop();

      if (grid[current.x][current.y].value > 0) break;

      for (Location next : neighbors(current)) {
        if (came_from.find(next) == came_from.end()) {
          frontier.push(next);
          came_from[next] = current;
        }
      }
    }
    return came_from;
  }

  template <typename Location>
  void a_star(const Location &start,
              const Location &goal,
              unordered_map<Location, Location> &came_from,
              unordered_map<Location, double> &cost) {
    PriorityQueue<Location, double> frontier;
    frontier.put(start, 0);
    came_from[start] = start;
    cost[start] = 0;
    while (!frontier.empty()) {
      auto current = frontier.get();
      if (current == goal) {
        break;
      }
      for (auto next : neighbors(current)) {
        double new_cost = cost[current] + 1;
        if (!cost.count(next) || new_cost < cost[next]) {
          cost[next] = new_cost;
          double priority = new_cost + heuristic(next, goal);
          frontier.put(next, priority);
          came_from[next] = current;
        }
      }
    }
  }

  template <typename Location>
  vector<Location> construct(const Location &start, const Location &goal, unordered_map<Location, Location> from) {
    vector<Location> path;
    Location current = goal;
    while (current != start) {
      path.push_back(current);
      current = from[current];
    }
    path.push_back(start);  // optional
    for (auto p : path) {
      cerr << " *" << p << endl;
    }
    // reverse(path.begin(), path.end());
    vector<Location> v(path.end() - min(5, static_cast<int>(path.size())), path.end());
    return v;
  }

  void findNearest() {
    cerr << "find nearest" << endl;
    for (auto &m : moi) {
      if (m.i == -1) break;
      cerr << m << endl;
      m.target = bullets[m.i].begin()->second;
      m.move();
    }
  }

  void bestAction() {
    cerr << "best action" << endl;
    findNearest();
  }

  void playActions() {
    cerr << "play action" << endl;
    for (auto m : moi) {
      if (m.i == -1) break;
      cout << m.action << endl;
    }
  }
};

int main() {
  Board theBoard;
  theBoard.init();
  while (1) {
    theBoard.turnInit();
    theBoard.bestAction();
    theBoard.playActions();
  }
}
