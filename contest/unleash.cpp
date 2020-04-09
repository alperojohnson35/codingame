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

  inline Point() : i(0), x(0), y(0) {}
  inline explicit Point(int r) : i(0), x(0), y(0) {}
  inline Point(int i, int x, int y) : i(i), x(x), y(y) {}
  inline Point(int x, int y) : i(0), x(x), y(y) {}
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

  // bool inZone() { return x >= 0 and x <= X_MAX and y >= 0 and y <= Y_MAX; }

  virtual void serialize(ostream &os) const { os << "#" << i << " [" << x << "," << y << "]"; }
};
ostream &operator<<(ostream &os, const Point &c) {
  c.serialize(os);
  return os;
}

typedef enum item { NONE = -1, RADAR = 2, TRAP = 3, ORE = 4 } item_t;
struct Cell : Point {
  int ore, gold;
  bool hole, trou;
  inline Cell() : ore(-1), hole(false), trou(false), Point{} {}
  virtual void serialize(ostream &os) const {
    Point::serialize(os);
    os << ", or:" << ore << ", hole:" << hole << ";" << trou << " gold " << gold;
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

struct Entity : Point {
  int item, nhole, ngold;
  string action;
  Point target;
  vector<Cell> autour;
  bool pause, feinte;

  inline Entity() : item(NONE), action("WAIT"), target(Point{-1, 0}), pause(false), feinte(false) {}
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
  int width, height, myScore, hisScore, nextRadar, nextTrap, nbEntity;
  int questionMark, money, closest, nturn, dead, moneyTop;
  Cell grid[30][15];
  vector<Point> radars, veins;
  array<Entity, 5> moi, lui;
  vector<Point> spots;
  unordered_set<Point> forbidden, eradars, traps;
  Point vision;
  vector<Point> dirs = {Point{0, 0}, Point{0, 1}, Point{0, -1}, Point{1, 0}, Point{-1, 0}};

  void init() {
    cin >> width >> height;
    cin.ignore();
    for (int i = 0; i < height; i++) {
      for (int j = 0; j < width; j++) {
        grid[j][i].x = j;
        grid[j][i].y = i;
      }
    }
    nturn = 0;

    // spots.push_back(Point{1, 1});
    // spots.push_back(Point{9, 0});
    // spots.push_back(Point{29, 0});
    // spots.push_back(Point{6, 14});
    // spots.push_back(Point{28, 7});
    // spots.push_back(Point{24, 12});
    // spots.push_back(Point{23, 3});
    // spots.push_back(Point{19, 8});
    // spots.push_back(Point{18, 0});
    // spots.push_back(Point{3, 11});
    // spots.push_back(Point{15, 13});
    // spots.push_back(Point{14, 4});
    // spots.push_back(Point{10, 9});
    // spots.push_back(Point{5, 5});

    spots.push_back(Point{27, 0});
    spots.push_back(Point{28, 9});
    spots.push_back(Point{25, 11});
    spots.push_back(Point{23, 5});
    spots.push_back(Point{18, 1});
    spots.push_back(Point{19, 10});
    spots.push_back(Point{15, 14});
    spots.push_back(Point{14, 6});
    spots.push_back(Point{10, 11});
    spots.push_back(Point{9, 2});
    // spots.push_back(Point{1, 12});
    spots.push_back(Point{5, 7});
  }

  void turnInit() {
    ++nturn;
    vector<Point> oldradar(radars);
    radars.clear();
    traps.clear();
    veins.clear();
    cin >> myScore >> hisScore;
    cin.ignore();
    money = 0;
    dead = 0;
    moneyTop = 6 + 2 * (nturn / 66);
    int x(7), y(12);
    // cerr << " Cell" << grid[x][y] << "forbid " << forbidden.count(Point{x, y}) << endl;
    for (int i = 0; i < height; i++) {
      for (int j = 0; j < width; j++) {
        string ore;
        int hole;
        cin >> ore >> hole;
        cin.ignore();
        if (j == x and i == y) {
          // cerr << "    ore " << ore << " cell " << grid[j][i] << endl;
        }
        grid[j][i].hole = hole;
        if (!(grid[j][i].ore >= 0 and ore == "?")) {
          grid[j][i].ore = ore == "?" ? -1 : stoi(ore);
        } else {
          if (grid[j][i].ore != grid[j][i].gold) {
            int close(0);
            Point p{j, i};
            for (auto m : moi) {
              if (m.dist(p) < 2) close++;
            }
            for (auto m : lui) {
              if (m.dist(p) < 2) close++;
            }
            grid[j][i].ore = grid[j][i].gold - close;
          }
        }
        grid[j][i].gold = grid[j][i].ore;
        if (grid[j][i].ore == -1) questionMark++;
        if (grid[j][i].ore > 0) {
          Point p = Point{j, i};
          if (!forbidden.count(p) and !traps.count(p)) {
            veins.push_back(p);
            money += grid[j][i].ore;
          }
        }
      }
    }

    cin >> nbEntity >> nextRadar >> nextTrap;
    cin.ignore();
    int far(30);
    for (int i = 0; i < nbEntity; i++) {
      int id, type, x, y, item;
      cin >> id >> type >> x >> y >> item;
      cin.ignore();
      if (x == -1 and y == -1) dead++;
      switch (type) {
        case (0):
          moi[id % 5].i = id;
          moi[id % 5].x = x;
          moi[id % 5].y = y;
          moi[id % 5].item = item;
          if (x == -1 and y == -1)
            moi[id % 5].action = "WAIT mort";
          else {
            moi[id % 5].action = "WAIT";
            if (x < far) {
              closest = i;
              far = x;
            }
          }
          // cerr << "#" << id << " target " << moi[id % 5].target << endl;
          break;
        case (1):
          lui[id % 5].i = id;
          // cerr << id << " prev " << lui[id % 5].x << ";" << lui[id % 5].y << " current " << x << ";" << y << endl;
          if (lui[id % 5].feinte and x >= lui[id % 5].x and x > 0) {
            lui[id % 5].item = RADAR;
          }
          if (lui[id % 5].x == x and lui[id % 5].y == y) {
            lui[id % 5].pause = true;
            if (x == 0) lui[i % 5].item = RADAR;
          } else {
            lui[id % 5].pause = false;
          }

          lui[id % 5].x = x;
          lui[id % 5].y = y;
          // lui[id % 5].action = "WAIT";
          // cerr << "lui " << id << " carries " << lui[id % 5].item << " pause " << lui[id % 5].pause << endl;
          break;
        case (2):
          radars.push_back(Point(x, y));
          break;
        case (3):
          traps.insert(Point(x, y));
          break;
        default:
          break;
      }
    }
    cerr << nturn << " delta " << myScore - hisScore << " nbE:" << nbEntity << " radar:" << radars.size()
         << " forbid:" << forbidden.size() << endl;
    cerr << spots.size() << " spots " << veins.size() << " veines " << money << " gold " << endl;
    for (auto f : forbidden) {
      cerr << f << endl;
    }
    if (nturn > 100 and forbidden.size() < 20) forbidden.clear();
  }

  inline double heuristic(Point a, Point b) { return abs(a.x - b.x) + abs(a.y - b.y); }

  void takeRadar() {
    if (nextRadar == 0 and !spots.empty() and money < moneyTop) {
      array<Entity, 5> copy = moi;
      sort(copy.begin(), copy.end(), [&spot = spots[spots.size() - 1]](Entity a, Entity b) {
        return a.dist(spot) < b.dist(spot);
      });
      for (auto &e : copy) {
        if (e.x == 0) {
          moi[e.i % 5].action = "REQUEST RADAR vision";
          moi[e.i % 5].target = spots[spots.size() - 1];
          vision = moi[e.i % 5].target;
          spots.pop_back();
          return;
        }
      }
    }
    for (auto m : moi) {
      if (m.item == RADAR) {
        vision = m.target;
        cerr << m.i << " porte radar " << vision << endl;
      }
    }
  }

  void placeRadar(Entity &e) {
    if (e.item == RADAR) {
      auto p = e.target;
      e.action = "DIG " + to_string(p.x) + " " + to_string(p.y) + " radar";
    }
    if (e.dist(e.target) == 1) {
      grid[e.target.x][e.target.y].trou = true;
      e.target.x = -1;
      radars.push_back(e.target);
      if (e.item == TRAP) traps.insert(e.target);
    }
  }
  void fetchRadar(Entity &e) {
    if (e.i == closest and e.x > 0 and money < moneyTop and (e.item == ORE or e.item == NONE) and
        nextRadar - (e.x / 4) <= 0) {
      e.action = "MOVE 0 " + to_string(e.y) + " back radar";
    }
  }

  void straigth() {
    for (auto &e : moi) {
      if (e.action == "WAIT" and e.target.x != -1 and !forbidden.count(e.target) and !traps.count(e.target) and
          e.item != RADAR) {
        // cerr << e.i << " continue " << endl;
        auto p = safe(e, e.target);
        e.action = "DIG " + to_string(e.target.x) + " " + to_string(e.target.y) + " straight";
        grid[e.target.x][e.target.y].ore = 0;
        if (e.dist(e.target) == 1) {
          e.target.x = -1;
          grid[e.target.x][e.target.y].trou = true;
          if (e.item == TRAP) traps.insert(e.target);
        }
      }
    }
  }
  void mine(Entity &e) {
    // sort(veins.begin(), veins.end(), [&e = e](Point a, Point b) { return a.dist(e) < b.dist(e); });
    sort(veins.begin(), veins.end(), [&e = e](Point a, Point b) {
      // if (a.x == b.x) {
      return a.dist(e) < b.dist(e);
      // } else {
      // return a.x < b.x;
      // }
    });
    if (!veins.empty() and e.action == "WAIT") {
      // cerr << "mine for " << e.i <<  endl;
      for (auto v : veins) {
        // cerr << grid[v.x][v.y].ore << " in " << v << endl;
        if (traps.count(v) or forbidden.count(v) or (grid[v.x][v.y].ore > 1 and e.item == TRAP)) {
          continue;
        }
        if (grid[v.x][v.y].ore > 0) {
          e.target = v;
          auto p = safe(e, e.target);
          e.action = "DIG " + to_string(e.target.x) + " " + to_string(e.target.y) + " creuse";
          grid[v.x][v.y].ore = 0;
          if (e.dist(e.target) == 1) {
            e.target.x = -1;
            grid[e.target.x][e.target.y].trou = true;
            if (e.item == TRAP) traps.insert(e.target);
          }
          break;
        }
      }
    }
  }

  void voisins(Entity &e) {
    bool unknown = false;
    e.autour.clear();
    e.nhole = 0;
    e.ngold = 0;
    for (auto d : dirs) {
      e.autour.push_back(grid[e.x + d.x][e.y + d.y]);
      e.nhole += static_cast<int>(grid[e.x + d.x][e.y + d.y].hole and !grid[e.x + d.x][e.y + d.y].trou);
      e.ngold += grid[e.x + d.x][e.y + d.y].ore;
      if (grid[e.x + d.x][e.y + d.y].ore == -1) unknown = true;
      // cerr << autour[i] << endl;
    }
    if (unknown) e.ngold = -1;
  }

  bool sameTable(const vector<Cell> &a, const vector<Cell> &b) {
    for (int i(0); i < 5; ++i) {
      if (a[i].x != b[i].x or a[i].y != a[i].y or a[i].hole != b[i].hole or a[i].ore != b[i].ore or a[i].ore == -1 or
          b[i].ore == -1)
        return false;
    }
    return true;
  }
  void voleur() {
    for (auto &l : lui) {
      auto nautour = l.autour;
      auto prev = l.nhole;
      auto sous = l.ngold;
      voisins(l);
      cerr << "enemy " << l.i << " item " << l.item << " pause " << l.pause << " feinte " << l.feinte << endl;
      if (l.item == RADAR and l.pause and l.x > 0) {
        cerr << "flairing " << l.i << " HhGg" << l.nhole << ":" << prev << "|" << l.ngold << ":" << sous << endl;
        if (l.nhole - prev == 1 or sous - l.ngold == 1 or l.x == 1) {
          for (int i(0); i < 5; ++i) {
            Point p = Point{l.x + dirs[i].x, l.y + dirs[i].y};
            cerr << "   new" << l.autour[i] << " old " << nautour[i] << endl;
            if ((l.autour[i].hole and !l.autour[i].trou and !nautour[i].hole) or
                (l.autour[i].ore >= 0 and l.autour[i].ore < nautour[i].ore) or l.x == 1) {
              bool bombe(false);
              for (auto r : eradars) {
                if (r.dist(p) < 5 and eradars.size() > 1) {
                  cerr << " IT'S A TRAP" << p << endl;
                  bombe = true;
                  break;
                }
              }
              if (bombe or eradars.empty())
                forbidden.insert(p);
              else
                eradars.insert(Point{l.x + 1, l.y});
            }
          }
        } else {
          for (auto a : l.autour) {
            if (a.hole) {
              forbidden.insert(Point{a.x, a.y});
              cerr << "FORBID " << Point{a.x, a.y} << endl;
            }
          }
          l.feinte = true;
        }
        if (sameTable(l.autour, nautour)) {
          cerr << "this is a WAIT" << endl;
          l.feinte = false;
        } else {
          l.item = NONE;
        }
      }
    }
  }

  vector<Point> zone(const Point &p) {
    int x = max(0, p.x - 4);
    int y = max(0, p.y - 4);
    vector<Point> res;
    for (int i = 0; i < 8; i++) {
      for (size_t j = 0; j < 8; j++) {
        Point m(x + i, y + j);
        if (m.dist(p) < 5 and m.dist(p) > 0) res.push_back(m);
      }
    }
    return res;
  }

  Point safe(const Point &from, const Point &to) {
    auto z = zone(from);
    sort(z.begin(), z.end(), [&to = to](Point a, Point b) { return a.x < b.x; });
    for (auto p : z) {
      bool ok(true), close(p.x > 0);
      for (auto f : forbidden) {
        if (p.dist(f) <= 1) {
          ok = false;
          break;
        }
      }
      for (auto l : lui) {
        if (p.dist(l) < 5) {
          close = true;
          break;
        }
      }
      if (ok and !close) return p;
    }
    return to;
  }

  vector<Point> neighbors(const Point &p, bool forbid = false) {
    vector<Point> res;
    for (auto d : dirs) {
      Point n(p.x + d.x, p.y + d.y);
      for (auto f : forbidden) {
        if (f.dist(n) <= 1) {
          forbid = true;
          break;
        }
      }
      if (n.x >= 0 and n.x < width and n.y >= 0 and n.y < height and !forbid) {
        res.push_back(n);
      }
    }
    return res;
  }

  void a_star(const Point &start,
              const Point &goal,
              unordered_map<Point, Point> &came_from,
              unordered_map<Point, double> &cost,
              bool forbid = false) {
    PriorityQueue<Point, double> frontier;
    frontier.put(start, 0);
    came_from[start] = start;
    cost[start] = 0;
    while (!frontier.empty()) {
      auto current = frontier.get();
      if (current == goal) {
        break;
      }
      for (auto next : neighbors(current, forbid)) {
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

  Point isSafe(const Point &from, const Point &to) {
    unordered_map<Point, Point> star;
    unordered_map<Point, double> cost;
    cerr << from.i << "star" << endl;
    a_star(from, to, star, cost);
    for (auto f : star) {
      cerr << f.first << ";" << f.second << endl;
    }
    if (star.size() == 1) {
      a_star(from, to, star, cost, true);
    }
    cerr << " new start " << star.size() << endl;
    auto path = construct(from, to, star);
    cerr << from.i << " path size " << path.size() << from << to << endl;
    auto r = from;
    for (auto p : path) {
      r = p;
      // cerr << "      ch " << r << endl;
      bool ok(true);
      for (auto f : forbidden) {
        if (f.dist(p) <= 1) {
          ok = false;
          break;
        }
      }
      if (ok) break;
    }
    return r;
  }

  void bank(Entity &e) {
    if (e.item == ORE) {
      auto p = safe(e, Point{0, e.y});
      cerr << " is safe " << p << endl;
      e.action = "MOVE " + to_string(p.x) + " " + to_string(p.y) + " cash money";
    }
  }

  void creuse(Entity &e, int t) {
    Point n = Point{e.x + dirs[t].x, e.y + dirs[t].y};
    if (!forbidden.count(n) and !traps.count(n)) {
      auto p = safe(e, e.target);
      e.action = "DIG " + to_string(n.x) + " " + to_string(n.y) + " dig dig";
      if (e.dist(n) == 1) {
        grid[n.x][n.y].trou = true;
        if (e.item == TRAP) traps.insert(n);
      }
    }
  }

  void bouge(Entity &e) {
    Point n = Point{e.x + 4, e.y};
    e.action = "MOVE " + to_string(n.x) + " " + to_string(n.y) + " move move";
  }

  void kami(Entity &e) {
    if (e.action == "WAIT") {
      for (auto d : dirs) {
        int eux(0), nous(0);
        for (auto d2 : dirs) {
          Point p = Point{e.x + d.x + d2.x, e.y + d.y + d2.y};
          for (auto m : moi) {
            if (p.x == m.x and p.y == m.y) nous++;
          }
          for (auto l : lui) {
            if (p.x == l.x and p.y == l.y) eux++;
          }
        }
        Point p = Point{e.x + d.x, e.y + d.y};
        if (eux >= nous and (forbidden.count(p) or traps.count(p)) and myScore >= hisScore + 2) {
          e.action = "DIG " + to_string(p.x) + " " + to_string(p.y) + " kami";
          grid[p.x][p.y].trou = true;
        }
      }
    }
  }

  void explore() {
    for (auto &e : moi) {
      if (e.action == "WAIT") {
        int t = (nturn < 3 or e.x < 5) ? 5 : nturn % 6;
        switch (t) {
          case 0:
          case 1:
          case 2:
          case 3:
          case 4:
            creuse(e, t);
            break;
          case 5:
            bouge(e);
            break;
        }
      }
    }
  }

  void takeTrap(Entity &e) {
    if (e.action == "WAIT" and nextTrap == 0 and e.x == 0 and (forbidden.size() > (50 * nturn) / 200 or nturn == 1)) {
      e.action = "REQUEST TRAP";
      e.item = TRAP;
    }
  }

  void bestAction() {
    takeRadar();
    voleur();
    straigth();
    for (auto &m : moi) {
      if (m.x == -1 and m.y == -1) continue;
      cerr << "  deal with " << m.i << endl;
      placeRadar(m);
      kami(m);
      bank(m);
      mine(m);
      fetchRadar(m);
      takeTrap(m);
    }
    explore();
  }

  void playActions() {
    for (auto m : moi) {
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
