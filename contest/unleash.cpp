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
  int ore;
  bool hole;
  inline Cell() : ore(0), hole(false), Point{} {}
  virtual void serialize(ostream &os) const {
    Point::serialize(os);
    os << ", or:" << ore << ", hole:" << hole;
  }
};
ostream &operator<<(ostream &os, const Cell &c) {
  c.serialize(os);
  return os;
}

struct Entity : Point {
  int item;
  string action;
  Point target;
  bool flaire, pause;

  inline Entity() : item(NONE), action("WAIT"), target(Point{}), flaire(false) {}
};

struct Board {
  int width, height, myScore, hisScore, nextRadar, nextTrap, nbEntity;
  int questionMark, money, closest, nhole, nturn, radarMax;
  Cell grid[30][15];
  vector<Point> radars, traps, veins;
  array<Entity, 5> moi, lui;
  vector<Point> spots;
  set<Point> forbidden, eradars;
  Point vision;
  array<Point, 5> dirs = {Point{0, 0}, Point{0, 1}, Point{0, -1}, Point{1, 0}, Point{-1, 0}};
  array<int, 10> atHome = {0};
  array<Cell, 5> autour;

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

    // spots.push_back(Point(28, pmin.y + 8));
    // spots.push_back(Point(28, pmin.y));
    // spots.push_back(Point(24, pmin.y + 4));
    // spots.push_back(Point(20, pmin.y + 8));
    // spots.push_back(Point(20, pmin.y));
    // spots.push_back(Point(16, pmin.y + 4));
    // spots.push_back(Point(12, pmin.y + 8));
    // spots.push_back(Point(12, pmin.y));
    // spots.push_back(Point(8, pmin.y + 4));

    spots.push_back(Point{1, 1});
    spots.push_back(Point{9, 0});
    spots.push_back(Point{29, 0});
    spots.push_back(Point{6, 14});
    spots.push_back(Point{28, 7});
    spots.push_back(Point{24, 12});
    spots.push_back(Point{23, 3});
    spots.push_back(Point{19, 8});
    spots.push_back(Point{18, 0});
    spots.push_back(Point{3, 11});
    spots.push_back(Point{15, 13});
    spots.push_back(Point{14, 4});
    spots.push_back(Point{10, 9});
    spots.push_back(Point{5, 5});

    // spots.push_back(Point{15, 14});
    // spots.push_back(Point{1, 3});
    // spots.push_back(Point{1, 12});
    // spots.push_back(Point{24, 14});
    // spots.push_back(Point{27, 0});
    // spots.push_back(Point{28, 9});
    // spots.push_back(Point{23, 5});
    // spots.push_back(Point{18, 1});
    // spots.push_back(Point{19, 10});
    // spots.push_back(Point{14, 6});
    // spots.push_back(Point{10, 11});
    // spots.push_back(Point{9, 2});
    // spots.push_back(Point{5, 7});
  }

  void turnInit() {
    ++nturn;
    radars.clear();
    traps.clear();
    veins.clear();
    cin >> myScore >> hisScore;
    cin.ignore();
    money = 0;
    for (int i = 0; i < height; i++) {
      for (int j = 0; j < width; j++) {
        string ore;  // amount of ore or "?" if unknown
        int hole;    // 1 if cell has a hole
        cin >> ore >> hole;
        cin.ignore();
        grid[j][i].hole = hole;
        if (!(grid[j][i].hole > 0 and ore == "?")) {
          grid[j][i].ore = ore == "?" ? -1 : stoi(ore);
        }
        if (grid[j][i].ore == -1) questionMark++;
        if (grid[j][i].ore > 0) {
          Point p = Point{j, i};
          if (find(forbidden.begin(), forbidden.end(), p) == forbidden.end()) {
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
      switch (type) {
        case (0):
          moi[id % 5].i = id;
          moi[id % 5].x = x;
          moi[id % 5].y = y;
          moi[id % 5].item = item;
          moi[id % 5].action = "WAIT";
          if (x < far) {
            closest = i;
            far = x;
          }
          if (x == 0)
            atHome[id]++;
          else
            atHome[id] = 0;
          // cerr << "#" << id << " target " << moi[id % 5].target << endl;
          break;
        case (1):
          lui[id % 5].i = id;
          cerr << id << " previous " << lui[id % 5].x << ";" << lui[id % 5].y << " current " << x << ";" << y << endl;
          if (lui[id % 5].x == x and lui[id % 5].y == y)
            lui[id % 5].pause = true;
          else
            lui[id % 5].pause = false;
          lui[id % 5].x = x;
          lui[id % 5].y = y;
          // lui[id % 5].action = "WAIT";
          // cerr << "lui " << id << " carries " << lui[id % 5].item << " pause " << lui[id % 5].pause << endl;
          if (x == 0)
            atHome[id]++;
          else
            atHome[id] = 0;
          break;
        case (2):
          radars.push_back(Point(x, y));
          break;
        case (3):
          traps.push_back(Point(x, y));
          break;
        default:
          break;
      }
    }
    radarMax = 0;
    for (auto e : eradars) {
      if (e.x > radarMax) radarMax = e.x;
    }
    cerr << "delta " << myScore - hisScore << " nbE:" << nbEntity << " radar:" << radars.size()
         << " forbid:" << forbidden.size() << endl;
    cerr << spots.size() << " spots " << veins.size() << " veines " << money << " gold " << radarMax << endl;
    for (int i(0); i < 10; ++i) {
      if (atHome[i] > 1) {
        cerr << i << " is requesting something" << endl;
        if (lui[i % 5].i == i) lui[i % 5].item = RADAR;
      }
    }
    // radarMax = max_element(eradars.begin(), eradars.end(), [](const Point &a, const Point &b) { return a.x < b.x; });
  }

  void takeRadar() {
    if (nextRadar == 0 and !spots.empty() and money < 8) {
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
          moi[e.i % 5].flaire = false;
          return;
        }
      }
    }
  }

  void placeRadar(Entity &e) {
    if (e.item == RADAR) {
      e.action = "DIG " + to_string(e.target.x) + " " + to_string(e.target.y) + " radar";
      vision = e.target;
      e.flaire = false;
    }
    if (e.dist(e.target) == 1) vision.x = -1;
  }
  void fetchRadar(Entity &e) {
    if (e.i == closest and e.x > 0 and e.action == "WAIT" and (e.item == ORE or e.item == NONE) and
        nextRadar - (e.x % 4) <= 0) {
      e.action = "MOVE 0 " + to_string(e.y) + " back radar";
      e.flaire = false;
    }
  }

  void mine(Entity &e) {
    // cerr << "mine for " << e.i << " flaire " << e.flaire << endl;
    if (e.flaire) return;
    sort(veins.begin(), veins.end(), [&e = e](Point a, Point b) { return a.dist(e) < b.dist(e); });
    if (!veins.empty() and e.action == "WAIT") {
      for (auto v : veins) {
        // cerr << grid[v.x][v.y].ore << " in " << v << endl;
        if (grid[v.x][v.y].ore > 0) {
          e.target = v;
          e.action = "DIG " + to_string(e.target.x) + " " + to_string(e.target.y) + " creuse";
          e.flaire = false;
          // grid[v.x][v.y].ore--;
          grid[v.x][v.y].ore = 0;
          break;
        }
      }
    }
  }

  void voisins(Point p) {
    nhole = 0;
    for (auto i(0); i < dirs.size(); ++i) {
      autour[i] = grid[p.x + dirs[i].x][p.y + dirs[i].y];
      nhole += static_cast<int>(autour[i].hole);
      // cerr << autour[i] << endl;
    }
  }

  void voleur() {
    for (auto &l : lui) {
      if (l.item == RADAR) {
        cerr << "flairing " << l.i << " pause " << l.pause << endl;
        array<Entity, 5> copy = moi;
        sort(copy.begin(), copy.end(), [&t = l](Entity a, Entity b) {
          if (a.dist(t) == b.dist(t)) {
            return a.flaire == true;
          } else {
            return a.dist(t) < b.dist(t);
          }
        });
        auto nautour = autour;
        auto prev = nhole;
        voisins(l);
        if (l.pause and l.x > 0) {
          eradars.insert(Point{l.x + 1, l.y});
          for (int i(0); i < dirs.size(); ++i) {
            Point p = Point{l.x + dirs[i].x, l.y + dirs[i].y};
            cerr << nhole << "vs" << prev << " new " << autour[i] << " old " << nautour[i] << endl;
            if ((autour[i].hole and !nautour[i].hole and nhole - prev <= 1) or
                (autour[i].hole and nautour[i].hole and nhole == 1)) {
              for (auto m : copy) {
                if (m.action == "WAIT" and m.item == NONE and moi[m.i % 5].dist(l) < 9) {
                  moi[m.i % 5].flaire = true;
                  moi[m.i % 5].action = "DIG " + to_string(p.x) + " " + to_string(p.y) + " vole";
                  if (moi[m.i % 5].dist(p) == 1) {
                    moi[m.i % 5].flaire = false;
                  }
                }
                break;
              }
              for (auto r : eradars) {
                if (r.dist(p) < 5) {
                  cerr << " IT'S A TRAP" << endl;
                  forbidden.insert(p);
                  break;
                }
              }
            } else if (autour[i].hole) {
              forbidden.insert(p);
              cerr << "FORBID " << p << endl;
            }
          }
          l.item = NONE;
        } else {
          for (auto m : copy) {
            if (m.action == "WAIT" and m.item == NONE and moi[m.i % 5].dist(l) < 9) {
              moi[m.i % 5].flaire = true;
              moi[m.i % 5].target = Point{l.x, l.y};
              moi[m.i % 5].action = "MOVE " + to_string(l.x) + " " + to_string(l.y) + " flaire " + to_string(l.i);
              break;
            }
          }
        }
      }
    }
  }

  void bank(Entity &e) {
    if (e.item == ORE) {
      e.action = "MOVE 0 " + to_string(e.y) + " cash money";
      e.flaire = false;
    }
  }

  void explore() {
    for (auto &e : moi) {
      if (e.action == "WAIT") {
        if (nturn % 2) {
          Point n = Point{e.x + 4, e.y};
          e.action = "MOVE " + to_string(n.x) + " " + to_string(n.y) + " rand move";
        } else {
          Point n = Point{e.x + 1, e.y};
          if (find(forbidden.begin(), forbidden.end(), n) == forbidden.end()) {
            e.action = "DIG " + to_string(n.x) + " " + to_string(n.y) + " rand dig";
          }
        }
        e.flaire = false;
      }
    }
  }

  void bestAction() {
    // sort(moi.begin(), moi.end(), [](Entity a, Entity b) { return a.item == RADAR; });
    takeRadar();
    for (auto &m : moi) {
      placeRadar(m);
      bank(m);
      mine(m);
      fetchRadar(m);
    }
    voleur();
    explore();
  }

  void playActions() {
    // sort(moi.begin(), moi.end(), [](Entity a, Entity b) { return a.item == RADAR; });
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
