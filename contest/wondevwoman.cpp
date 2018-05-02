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

const signed short GLOBAL_TURN_TIME_MAX = 46;
const short GLOBAL_TURN_TIME_MAX_FIRST_TURN = 900;
const short GLOBAL_MAX_TURN = 200;

template <class T, size_t ROW, size_t COL>
using Matrix = std::array<std::array<T, COL>, ROW>;

struct Point {
  int x, y;
  inline Point() = default;
  inline Point(int x, int y) : x(x), y(y) {}
  inline Point(Point const &) = default;
  inline Point(Point &&) = default;
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
  inline size_t operator()(const Point &c) const { return c.x * 1812433253 + c.y; }
};
}  // namespace std
struct Cell {
  int h, u;
  inline Cell() = default;
  inline Cell(int h, int u) : h(h), u(u) {}
  inline Cell(Cell const &) = default;
  inline Cell(Cell &&) = default;
  inline Cell &operator=(Cell const &) = default;
  inline Cell &operator=(Cell &&) = default;

  inline bool operator==(const Cell &c) const { return this->h == c.h && this->u == c.u; }
  inline bool operator!=(const Cell &c) const { return !((*this) == c); }
  inline bool operator<(const Cell &c) const { return this->h + 8 * this->u < c.h + 8 * c.u; }
  inline Cell operator+(const Cell &c) const { return Cell(this->h + c.h, this->u + c.u); }
  inline Cell operator-(const Cell &c) const { return Cell(this->h - c.h, this->u - c.u); }
  virtual void serialize(ostream &os) const { os << " h:" << h << ", u:" << u; }
};
ostream &operator<<(ostream &os, const Cell &c) {
  c.serialize(os);
  return os;
}

typedef enum { ME, HIM } Player_t;
typedef enum { CASTOR, POLLUX, DINGO, MAX } Unit_t;

struct Voronoi {
  int dist(const Point &c1, const Point &c2) {
    return max(abs(c1.x - c2.x), abs(c1.y - c2.y));
    // return (c1.x - c2.x)*(c1.x - c2.x)+(c1.y - c2.y)*(c1.y - c2.y);
  }

  int voronoi(const Matrix<Cell, 7, 7> &cells, const vector<Point> &units, const int &size, const Player_t &player) {
    vector<int> nbCell(4, 0);
    for (int i(0); i < size; ++i) {
      for (int j(0); j < size; ++j) {
        if (cells[i][j].h >= 4) continue;
        int d(8);
        vector<int> unit(4, -1);
        int unitId(0);
        Point c(i, j);
        for (auto u : units) {
          int D = dist(c, u);
          if (D < d) {
            unit.clear();
            d = dist(c, u);
            unit.push_back(unitId);
          } else if (D == d) {
            unit.push_back(unitId);
          }
          unitId++;
        }
        for (auto u : unit) {
          if (u >= 0) {
            nbCell[u]++;
          }
        }
      }
    }
    return (1 - 2 * player) * (nbCell[CASTOR] + nbCell[POLLUX] - nbCell[DINGO] - nbCell[MAX]);
  }
};

typedef enum { MOVE, PUSH } Type_t;

struct Action {
  Point pos, build, move;
  Unit_t unit;
  Type_t type;
  array<string, 2> actionName{"MOVE&BUILD ", "PUSH&BUILD "};
  map<Point, string> adir = {{Point{1, 0}, "E"},
                             {Point{1, 1}, "SE"},
                             {Point{0, 1}, "S"},
                             {Point{-1, 1}, "SW"},
                             {Point{-1, 0}, "W"},
                             {Point{-1, -1}, "NW"},
                             {Point{0, -1}, "N"},
                             {Point{1, -1}, "NE"}};

  inline Action() = default;
  inline Action(Point p, Point b, Point m, Unit_t u, Type_t t) : pos(p), build(b), move(m), unit(u), type(t) {}
  inline Action(Action const &) = default;
  inline Action(Action &&) = default;
  inline Action &operator=(Action const &) = default;
  inline Action &operator=(Action &&) = default;

  inline void print() {
    string s;
    if (type == PUSH) {
      s = actionName[type] + to_string(unit) + " " + adir[build - pos] + " " + adir[move - build];
    } else {
      s = actionName[type] + to_string(unit) + " " + adir[move - pos] + " " + adir[build - move];
    }
    cout << s << endl;
  }

  void play(Matrix<Cell, 7, 7> &cells, vector<Point> &units) {
    ++cells[build.x][build.y].h;
    if (type == MOVE) {
      cells[move.x][move.y].u = unit;
      cells[pos.x][pos.y].u = -1;
      units[unit] = move;
    } else {
      cells[move.x][move.y].u = cells[build.x][build.y].u;
      cells[build.x][build.y].u = -1;
      units[cells[move.x][move.y].u] = move;
    }
  }

  void rewind(Matrix<Cell, 7, 7> &cells, vector<Point> &units) {
    --cells[build.x][build.y].h;
    if (type == MOVE) {
      cells[pos.x][pos.y].u = unit;
      cells[move.x][move.y].u = -1;
      units[unit] = pos;
    } else {
      cells[build.x][build.y].u = cells[move.x][move.y].u;
      cells[move.x][move.y].u = -1;
      units[cells[build.x][build.y].u] = build;
    }
  }
  inline Player_t player() {
    switch (unit) {
      case CASTOR:
      case POLLUX:
        return ME;
      case DINGO:
      case MAX:
        return HIM;
    }
  }

  virtual void serialize(ostream &os) const {
    os << unit << actionName[type] << " Pos " << pos << " build " << build << " move " << move;
  }
};
ostream &operator<<(ostream &os, const Action &a) {
  a.serialize(os);
  return os;
}

struct Board {
  int size, nbUnits;
  Matrix<Cell, 7, 7> cells;
  vector<Point> units;
  vector<int> scores;
  vector<string> actions;

  vector<Point> dirs = {
      Point{1, 0}, Point{1, 1}, Point{0, 1}, Point{-1, 1}, Point{-1, 0}, Point{-1, -1}, Point{0, -1}, Point{1, -1}};
  map<Point, string> adir = {{Point{1, 0}, "E"},
                             {Point{1, 1}, "SE"},
                             {Point{0, 1}, "S"},
                             {Point{-1, 1}, "SW"},
                             {Point{-1, 0}, "W"},
                             {Point{-1, -1}, "NW"},
                             {Point{0, -1}, "N"},
                             {Point{1, -1}, "NE"}};

  inline Board() = default;
  inline Board(int size) : size(size), nbUnits(0) {}
  inline Board(Board const &) = default;
  inline Board(Board &&) = default;
  inline Board &operator=(Board const &) = default;
  inline Board &operator=(Board &&) = default;

  void mainInit() {
    cin >> this->size;
    cin.ignore();
    cin >> this->nbUnits;
    cin.ignore();
  }

  void turnInit() {
    actions.clear();
    units.clear();
    for (int i = 0; i < size; ++i) {
      string row;
      cin >> row;
      cin.ignore();
      for (int j(0); j < row.size(); ++j) {
        if (row[j] == '.') {
          cells[j][i].h = 4;
        } else {
          cells[j][i].h = row[j] - '0';
        }
        cells[j][i].u = -1;
      }
    }
    for (int unitId(0); unitId < 2 * nbUnits; ++unitId) {
      Point p;
      cin >> p.x >> p.y;
      cin.ignore();
      cells[p.x][p.y].u = unitId;
      units.push_back(p);
    }
    int nbActions;
    cin >> nbActions;
    cin.ignore();
    cerr << nbActions << " legal actions " << endl;
    for (auto u : units) {
      cerr << "units " << u << endl;
    }
    for (int i = 0; i < nbActions; ++i) {
      string atype, dir1, dir2;
      int index;
      cin >> atype >> index >> dir1 >> dir2;
      cin.ignore();
      actions.push_back(atype + " " + to_string(index) + " " + dir1 + " " + dir2);
      // cerr  << actions[i] << endl;
    }
  }
  inline bool inBounds(const Point &p) { return p.x >= 0 && p.x < size && p.y >= 0 && p.y < size; }
  inline bool isWall(const Point &p) { return !inBounds(p) or cells[p.x][p.y].h >= 4; }
  inline bool canClimb(const Point &p, const Point &t) { return cells[t.x][t.y].h <= cells[p.x][p.y].h + 1; }
  inline int delta(const Point &p, const Point &t) { return cells[t.x][t.y].h - cells[p.x][p.y].h; }
  inline bool isEmpty(const Point &p) { return cells[p.x][p.y].u == -1; }

  vector<Point> neighbors(const Point &c) {
    vector<Point> v;
    for (auto d : dirs) {
      Point p(c + d);
      if (!isWall(p) and canClimb(c, p)) {
        v.push_back(p);
      }
    }
    return v;
  }

  inline bool otherUnit(const Point &p, const int &id) { return cells[p.x][p.y].u >= 0 and cells[p.x][p.y].u != id; }

  vector<Point> build(const Point &c, const int &id) {
    vector<Point> v;
    for (auto d : dirs) {
      Point p(c + d);
      if (!isWall(p) and !otherUnit(p, id)) {
        v.push_back(p);
      }
    }
    return v;
  }

  inline bool unitHere(const Point &c) {
    for (auto p : units) {
      if (p == c) return true;
    }
    return false;
  }

  bool validPush(const Point &origin, const Point &pushed, const Point &dest) {
    if (isWall(dest)) return false;
    Point dir(pushed - origin);
    Point c(dest - pushed);
    if (c == dir) return true;
    auto pos = find(dirs.begin(), dirs.end(), dir) - dirs.begin();
    if (pos == 0) {
      if (c == dirs[7] or c == dirs[1]) return true;
    } else if (pos == 7) {
      if (c == dirs[6] or c == dirs[0]) return true;
    } else {
      if (c == dirs[pos - 1] or c == dirs[pos + 1]) return true;
    }
    return false;
  }

  inline bool isEnnemy(const int &e, const int &id) {
    if ((id <= 1 and e >= 2) or (id >= 2 and e <= 1)) return true;
    return false;
  }

  double evalAction(Action &a) {
    Voronoi V;
    int v1 = V.voronoi(cells, units, size, a.player());

    // action
    a.play(cells, units);

    // nb point gagne
    int v2 = V.voronoi(cells, units, size, a.player());
    double score(0);

    if (a.unit <= 1 and cells[a.move.x][a.move.y].h == 3) {
      score -= 100;
    }
    if (a.type == PUSH) {
      score += 20 * delta(a.pos, a.move);
      if (delta(a.pos, a.move) == 0 and cells[a.move.x][a.move.y].h == 3) {
        score += 10;
      }
      if (neighbors(a.move).size() == 0) {
        score -= 50;
      }
    } else {
      score += -20 * delta(a.pos, a.move);
    }
    score -= v2 - v1;

    // rewind
    a.rewind(cells, units);
    return score;
  }

  vector<Action> computeActions(const int &user) {
    vector<Action> actions;
    int nb(0);
    for (Unit_t id = (2 * user); id < 2 * user + 2; ++id) {
      Point c(units[id]);
      vector<Point> voisins = neighbors(c);
      for (auto v : voisins) {
        vector<Point> vois = neighbors(v);
        if (vois.size() == 0) {
          cerr << "pas de voisins en " << v << endl;
          continue;
        }

        if (!isEmpty(v)) {
          if (isEnnemy(cells[v.x][v.y].u, id)) {
            for (auto w : vois) {
              if (w == c or !validPush(c, v, w) or !isEmpty(w)) continue;
              // cerr << "add action " << s << " "<<val<< endl;
              actions.push_back(Action(c, v, w, id, PUSH));
              ++nb;
            }
          }
        } else {
          vector<Point> builds = build(v, id);
          for (auto b : builds) {
            // cerr <<"add action " << s << " "<<val<< endl;
            actions.push_back(Action(c, b, v, id, MOVE));
            ++nb;
          }
        }
      }
      cerr << nb << " actions computed for id " << id << endl;
    }
    return actions;
  }

  void printBestAction(Player_t user) {
    vector<Action> actions = computeActions(user);

    Action bestAction;
    double bestScore = 1000;
    for (auto a : actions) {
      double currentScore = evalAction(a);
      cerr << a << " -score " << currentScore << endl;
      if (currentScore < bestScore) {
        bestAction = a;
        bestScore = currentScore;
      }
    }
    bestAction.print();
  }
};

struct Timer {
  chrono::time_point<chrono::system_clock> end;
  signed char not_always = 0;

  inline Timer() = default;
  inline Timer(Timer const &) = default;
  inline Timer(Timer &&) = default;
  inline Timer &operator=(Timer const &) = default;
  inline Timer &operator=(Timer &&) = default;

  inline Timer(bool first_turn = false) {
    if (first_turn) {
      this->end = chrono::system_clock::now() + chrono::milliseconds(GLOBAL_TURN_TIME_MAX_FIRST_TURN);
    } else {
      this->end = chrono::system_clock::now() + chrono::milliseconds(GLOBAL_TURN_TIME_MAX);
    }
  }
  inline bool isTimesUp() { return std::chrono::system_clock::now() > this->end; }
};

/*
struct MinMax {
  double minmax(Board theBoard, int depth, Action a, Player_t player) {
    if (depth == 0) {
      return evalAction(a);
    }
    if (ME) {
      double bestValue = 1000;
      for (auto a : theBoard.computeActions()) {
        a.play();
        double v = minmax(theBoard, depth - 1, a, HIM);
        if (v < bestValue) {
          bestValue = v;
          bestAction = a;
        }
        a.rewind();
      }
      return bestValue;
    } else {
      int bestValue = 1000;
      for (auto a : theBoard.computeActions()) {
        int v = minmax(theBoard, depth - 1, true);
        bestValue = min(bestValue, v);
        return bestValue;
      }
    }
  }
};*/

int main() {
  Board theBoard;
  theBoard.mainInit();

  // game loop
  while (1) {
    theBoard.turnInit();

    // Write an action using cout. DON'T FORGET THE "<< endl"
    // To debug: cerr << "Debug messages..." << endl;
    theBoard.printBestAction(ME);
  }
}
