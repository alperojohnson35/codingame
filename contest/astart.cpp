#pragma GCC target("avx")                   // small optimization
#pragma GCC optimize("O3")                  // +44k killer optimization with "inline __attribute__ (( always_inline,
                                            // visibility("protected") ))" and "inline"
#pragma GCC optimize("omit-frame-pointer")  // good optimization
#pragma GCC optimize("unsafe-math-optimizations")  // not really useful it seems
#pragma GCC optimize("unroll-all-loops")           // good optimization
#pragma GCC optimize("inline")                     // killer optimization with O3

#include <stdlib.h>
#include <string.h>
#include <algorithm>
#include <array>
#include <bitset>
#include <cassert>
#include <chrono>
#include <climits>
#include <cmath>
#include <deque>
#include <fstream>
#include <iostream>
#include <limits>
#include <list>
#include <map>
#include <queue>
#include <random>
#include <set>
#include <sstream>
#include <string>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <vector>

using namespace std;

string dir("URDL");
map<char, int> mdir = {{'U', 0}, {'R', 1}, {'D', 2}, {'L', 3}};
/*
0
U, R, D, L
UR, RD, DL, LU, UD, RL
URD, RDL, DLU, LUR
URDL
*/
#define UP (1 << 0)
#define RIGHT (1 << 1)
#define DOWN (1 << 2)
#define LEFT (1 << 3)

array<string, 16> possible = {
    " ", "U ", "R ", "UR ", "D ", "UD ", "RD ", "URD ", "L ", "LU ", "RL ", "LUR ", "DL ", "DLU ", "RDL ", "URLD "};

#define HALF_TIME (200)
#define TIME_MAX (980)

struct Timer {
  chrono::time_point<chrono::system_clock> start;
  chrono::time_point<chrono::system_clock> end;
  chrono::time_point<chrono::system_clock> half;
  signed char not_always = 0;

  inline Timer(Timer const &) = default;
  inline Timer(Timer &&) = default;
  inline Timer &operator=(Timer const &) = default;
  inline Timer &operator=(Timer &&) = default;

  inline Timer() {
    this->start = chrono::system_clock::now();
    this->end = chrono::system_clock::now() + chrono::milliseconds(TIME_MAX);
    this->half = chrono::system_clock::now() + chrono::milliseconds(HALF_TIME);
  }
  inline long long click() const {
    return chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now() - this->start).count();
  }
  inline bool timesUp() { return std::chrono::system_clock::now() > this->end; }
  inline bool halfTime() { return std::chrono::system_clock::now() > this->half; }
};

struct Pos {
  int i, x, y;
  inline Pos() = default;
  inline Pos(int x, int y) : i(0), x(x), y(y) {}
  inline Pos(int i, int x, int y) : i(i), x(x), y(y) {}
  inline Pos(Pos const &) = default;
  inline Pos(Pos &&) = default;
  inline Pos &operator=(Pos const &) = default;
  inline Pos &operator=(Pos &&) = default;

  int dist(const Pos &p) { return abs(this->x - p.x) + abs(this->y - p.y); }

  int dist1(const Pos &p) { return (this->x - p.x) * (this->x - p.x) + (this->y - p.y) * (this->y - p.y); }

  double dist2(const Pos &p) { return sqrt(dist1(p)); }

  void norm() {
    x = x / sqrt(x * x + y * y);
    y = y / sqrt(x * x + y * y);
  }

  int scalar(const Pos &p) { return this->x * p.x + this->y * p.y; }

  Pos move(const Pos &p) { return Pos(x - (x - p.x) * 60.0 / this->dist2(p), y - (y - p.y) * 60.0 / this->dist2(p)); }

  bool isInRange(Pos p, double range) { return p != *this && dist2(p) <= range; }

  inline bool operator==(const Pos &c) const { return this->x == c.x and this->y == c.y and this->i == c.i; }
  inline bool operator!=(const Pos &c) const { return !((*this) == c); }
  inline bool operator<(const Pos &c) const { return this->x + 100 * this->y < c.x + 100 * c.y; }
  inline Pos operator+(const Pos &c) const { return Pos(this->x + c.x, this->y + c.y); }
  inline Pos operator-(const Pos &c) const { return Pos(this->x - c.x, this->y - c.y); }
  inline Pos operator*(const int &c) const { return Pos(this->x * c, this->y * c); }
  inline Pos operator/(const int &c) const { return Pos(this->x / c, this->y / c); }
  virtual void serialize(ostream &os) const { os << i << " [" << x << "," << y << "] "; }
};
ostream &operator<<(ostream &os, const Pos &c) {
  c.serialize(os);
  return os;
}

struct Robo : public Pos {
  char d;
  inline Robo() = default;
  inline Robo(int x, int y) : Pos(x, y), d('U') {}
  inline Robo(int x, int y, char d) : Pos(x, y), d(d) {}
  inline Robo(int i, int x, int y, char d) : Pos(i, x, y), d(d) {}
  inline Robo(Robo const &) = default;
  inline Robo(Robo &&) = default;
  inline Robo &operator=(Robo const &) = default;
  inline Robo &operator=(Robo &&) = default;

  inline bool operator==(const Robo &c) const {
    return this->x == c.x and this->y == c.y and this->i == c.i and this->d == c.d;
  }
  inline bool operator!=(const Robo &c) const { return !((*this) == c); }
  inline bool operator<(const Robo &c) const { return this->x + 100 * this->y < c.x + 100 * c.y; }
  inline Robo operator+(const Robo &c) const { return Robo(this->x + c.x, this->y + c.y); }
  inline Robo operator-(const Robo &c) const { return Robo(this->x - c.x, this->y - c.y); }
  inline Robo operator*(const int &c) const { return Robo(this->x * c, this->y * c); }
  inline Robo operator/(const int &c) const { return Robo(this->x / c, this->y / c); }
};

namespace std {
template <>
struct hash<Pos> {
  inline size_t operator()(const Pos &p) const { return p.x * 1812433253 + p.y; }
};
template <>
struct hash<Robo> {
  inline size_t operator()(const Robo &p) const { return p.x * 1812433253 + p.y; }
};
}  // namespace std

struct Grid {
  static array<Robo, 4> axis;

  int width, height;
  unordered_set<Pos> walls;
  unordered_set<Pos> cells;
  unordered_map<Pos, char> items;

  Grid() {}
  Grid(int width_, int height_) : width(width_), height(height_) {}
  inline Grid(Grid const &) = default;
  inline Grid(Grid &&) = default;

  inline bool isWall(Pos id) const { return walls.count(id); }
  inline bool isCell(Pos id) const { return cells.count(id); }
  inline bool isItem(Pos id) const { return items.count(id); }

  inline int remap(int i, int t) const {
    if (i == t)
      return 0;
    else if (i == -1)
      return t - 1;
    else
      return i;
  }

  unsigned int cellType(const Pos &r) const {
    unsigned int cell(0);
    for (int i(0); i < 4; ++i) {
      Pos next(remap(r.x + axis[i].x, width), remap(r.y + axis[i].y, height));
      if (!isWall(next)) cell += 1 << i;
    }
    return cell;
  }
};
array<Robo, 4> Grid::axis{Robo{0, -1}, Robo{1, 0}, Robo{0, 1}, Robo{-1, 0}};

struct Board {
  Grid g = Grid(19, 10);
  int nbRobo, maxScore;
  array<Robo, 10> robos;
  vector<vector<Robo> > Actions;
  vector<Robo> best;

  void init() {
    for (int j = 0; j < 10; ++j) {
      string line;
      cin >> line;
      cin.ignore();
      for (int i(0); i < line.size(); ++i) {
        switch (line[i]) {
          case '#':
            g.walls.insert(Pos(i, j));
            break;
          case 'U':
          case 'D':
          case 'L':
          case 'R':
          case 'u':
          case 'd':
          case 'l':
          case 'r':
            g.items[Robo(i, j)] = toupper(line[i]);
            break;
          case '.':
            g.cells.insert(Pos(i, j));
        }
      }
    }
    cin >> nbRobo;
    cin.ignore();
    for (int i = 0; i < nbRobo; ++i) {
      cin >> robos[i].x >> robos[i].y >> robos[i].d;
      cin.ignore();
    }
  }

  Robo nextPos(const Robo &r) {
    auto i = g.items.find(r);
    if (i != g.items.end() and i->second != ' ') {
      // cerr << r.x << " " << r.y << " in items" << endl;
      Robo n = Robo(r + g.axis[mdir[i->second]]);
      n.x = g.remap(n.x, g.width);
      n.y = g.remap(n.y, g.height);
      n.d = i->second;
      return n;
    } else {
      Robo n = Robo(r + g.axis[mdir[r.d]]);
      n.d = r.d;
      n.x = g.remap(n.x, g.width);
      n.y = g.remap(n.y, g.height);
      // cerr << r.d << " " << mdir[r.d] << " " << g.axis[mdir[r.d]] << endl;
      return n;
    }
    return r;
  }

  int walk(const Robo &r) {
    int score(0);
    unordered_set<Robo> states;
    Robo next = Robo(r.x, r.y, r.d);
    // cerr << "init " << next.x << " " << next.y << " " << next.d << endl;
    while (!g.isWall(next)) {
      next = nextPos(next);
      if (states.count(next)) break;
      states.insert(next);
      ++score;
      // cerr << states.count(next) << " " << next.x << " " << next.y << endl;
    }
    // cerr << "final " << next.x << " " << next.y << " " << next.d << endl;
    return score;
  }

  vector<Pos> strategicCells() {
    vector<Pos> k;
    for (auto c : g.cells) {
      // cerr << c.x << " " << c.y << " " << g.cellType(c) << endl;
      if (g.cellType(c) != 5 and g.cellType(c) != 10 and g.cellType(c) != 0) k.push_back(c);
    }
    return k;
  }

  void allActions(const vector<Pos> &k, vector<Robo> &combi, int n, Timer theClock) {
    if (theClock.halfTime()) return;
    if (n == k.size() - 1) {
      for (auto c : possible[g.cellType(k[n])]) {
        // cerr << k[n].x << "-" << k[n].y << " " << c << " " << g.cellType(k[n]) << endl;
        combi.push_back(Robo(k[n].x, k[n].y, c));
        Actions.push_back(combi);
        combi.pop_back();
      }
    } else {
      for (auto c : possible[g.cellType(k[n])]) {
        // cerr << k[n].x << "+" << k[n].y << " " << c << " " << g.cellType(k[n]) << endl;
        combi.push_back(Robo(k[n].x, k[n].y, c));
        allActions(k, combi, n + 1, theClock);
        combi.pop_back();
      }
    }
  }

  string printCombi(vector<Robo> r) {
    string s("");
    for (auto c : r) s += to_string(c.x) + "," + to_string(c.y) + "|" + c.d + "-";
    return s;
  }

  int play(Timer theClock) {
    int played(0);
    cerr << Actions.size() << endl;
    for (auto combis : Actions) {
      if (theClock.timesUp()) return played;
      for (auto item : combis) g.items[item] = item.d;

      int s(0);
      for (int i(0); i < nbRobo; ++i) s += walk(robos[i]);
      // cerr << "# " << combis.size() << " pt " << s << " " << printCombi(combis) << endl;
      if (s >= maxScore) {
        maxScore = s;
        best = combis;
      }
      for (auto item : combis) g.items.erase(item);
      ++played;
    }
    return played;
  }

  void writeResult() const {
    string out("");
    // cerr << best.size() << " " << maxScore << endl;
    for (auto e : best) {
      if (e.d != ' ') out += to_string(e.x) + " " + to_string(e.y) + " " + e.d + " ";
    }
    cout << out << endl;
  }
};

int main() {
  Board b = Board();
  b.init();
  Timer theClock;
  vector<Pos> cells = b.strategicCells();
  vector<Robo> combi;
  b.allActions(cells, combi, 0, theClock);
  cerr << "actions " << theClock.click() << endl;
  int played = b.play(theClock);
  cerr << " play " << theClock.click() << " / " << played << endl;
  b.writeResult();
}
