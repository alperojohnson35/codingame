// 235

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
#include <utility>
#include <vector>

using namespace std;

#define HALF_TIME (200)
#define TIME_MAX (96)
#define W (30)
#define H (20)

#define NOW chrono::high_resolution_clock::now()
#define START __time__start = NOW
#define TIME chrono::duration_cast<chrono::duration<double>>(NOW - __time__start).count()
chrono::high_resolution_clock::time_point __time__start = NOW;

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
  int i, x, y, px, py, wall;
  inline Pos() = default;
  inline Pos(int x, int y) : i(-1), x(x), y(y), px(-1), py(-1), wall(-1) {}
  inline Pos(int x, int y, int px, int py) : i(-1), x(x), y(y), px(px), py(py), wall(-1) {}
  inline Pos(Pos const &) = default;
  inline Pos(Pos &&) = default;
  inline Pos &operator=(Pos const &) = default;
  inline Pos &operator=(Pos &&) = default;

  int dist(const Pos &p) { return abs(this->x - p.x) + abs(this->y - p.y); }
  int dist1(const Pos &p) { return (this->x - p.x) * (this->x - p.x) + (this->y - p.y) * (this->y - p.y); }
  double dist2(const Pos &p) { return sqrt(dist1(p)); }
  int idx() const { return this->x + W * this->y; }
  int pidx() const { return this->px + W * this->py; }

  void norm() {
    x = x / sqrt(x * x + y * y);
    y = y / sqrt(x * x + y * y);
  }

  int scalar(const Pos &p) { return this->x * p.x + this->y * p.y; }

  Pos move(const Pos &p) { return Pos(x - (x - p.x) * 60.0 / this->dist2(p), y - (y - p.y) * 60.0 / this->dist2(p)); }

  bool isInRange(Pos p, double range) { return p != *this && dist2(p) <= range; }

  inline bool operator==(const Pos &c) const { return this->x == c.x and this->y == c.y; }
  inline bool operator!=(const Pos &c) const { return !((*this) == c); }
  inline bool operator<(const Pos &c) const { return this->x + 100 * this->y < c.x + 100 * c.y; }
  inline Pos operator+(const Pos &c) const { return Pos(this->x + c.x, this->y + c.y); }
  inline Pos operator-(const Pos &c) const { return Pos(this->x - c.x, this->y - c.y); }
  inline Pos operator*(const int &c) const { return Pos(this->x * c, this->y * c); }
  inline Pos operator/(const int &c) const { return Pos(this->x / c, this->y / c); }
  virtual void serialize(ostream &os) const { os << " [" << x << "," << y << "] " << wall; }
};
ostream &operator<<(ostream &os, const Pos &c) {
  c.serialize(os);
  return os;
}

namespace std {
template <>
struct hash<Pos> {
  inline size_t operator()(const Pos &p) const { return p.x * 1812433253 + p.y; }
};
}  // namespace std

struct Board {
  int N, P, turn, maxDepth;
  set<int> alive;
  array<Pos, 4> players;
  array<int, H * W> grid;
  array<Pos, 4> dir = {Pos{0, -1}, Pos{0, 1}, Pos{-1, 0}, Pos{1, 0}};
  map<Pos, string> adir = {{Pos{0, -1}, "UP"}, {Pos{0, 1}, "DOWN"}, {Pos{-1, 0}, "LEFT"}, {Pos{1, 0}, "RIGHT"}};
  bool first = true;

  Board() {
    turn = 0;
    maxDepth = 1;
    memset(&grid, 0, sizeof(grid));
  }

  void display() const {
    for (int j(0); j < H; ++j) {
      string s("");
      for (int i(0); i < W; ++i) {
        s += to_string(grid[i + W * j]);
      }
      cerr << s << endl;
    }
  }
  void init() {
    turn++;
    cin >> N >> P;
    cin.ignore();
    START;
    for (int i = 0; i < N; ++i) {
      cin >> players[i].px >> players[i].py >> players[i].x >> players[i].y;
      cin.ignore();
      if (players[i].x == -1) {
        if (alive.count(i)) {
          for (auto g : grid) {
            if (g == i + 1) g = 0;
          }
          alive.erase(alive.find(i));
        }
      } else {
        grid[players[i].idx()] = i + 1;
      }
      if (first) {
        grid[players[i].pidx()] = i + 1;
        alive.insert(i);
      }
      //   cerr << "i:" << i << " x," << players[i].x << " y," << players[i].y << " px," << players[i].px << " py,"
      //   << players[i].py << endl;
    }
    first = false;
    // cerr << alive.size() << " alive" << endl;
  }

  void endTurn() {
    for (auto p : alive) {
      grid[players[p].idx()] = 0;
    }
  }

  bool inBoard(const Pos &p) const { return p.x >= 0 and p.x < W and p.y >= 0 and p.y < H; }

  vector<Pos> neighbors(const Pos &p) {
    vector<Pos> n;
    for (auto d : dir) {
      Pos c(p + d);
      //   cerr << c.x<<";"<<c.y<<"|"<<c.wall<< endl;
      if (inBoard(c) and grid[c.idx()] == 0) {
        n.push_back(c);
      }
    }
    return n;
  }

  set<Pos> Bfs(const Pos &start) {
    queue<Pos> frontier;
    frontier.push(start);

    set<Pos> path;
    path.insert(start);

    while (!frontier.empty()) {
      auto current = frontier.front();
      frontier.pop();

      for (auto next : neighbors(current)) {
        if (!path.count(next)) {
          frontier.push(next);
          path.insert(next);
        }
      }
    }
    path.erase(start);
    return path;
  }

  int bfs(const Pos &start) {
    queue<Pos> frontier;
    frontier.push(start);

    set<Pos> path;
    path.insert(start);
    int res(0);

    while (!frontier.empty()) {
      auto current = frontier.front();
      frontier.pop();

      for (auto next : neighbors(current)) {
        if (!path.count(next)) {
          frontier.push(next);
          path.insert(next);
          res++;
        }
      }
    }
    return res;
  }

  int voronoi(const array<Pos, 4> &node) {
    array<int, 4> scores = {0};
    int id(0);
    for (auto g : grid) {
      if (g) continue;
      int player = 0, dist = INT_MAX;
      for (int i = 0; i < N; ++i) {
        Pos local{id % W, id / W};
        if ((alive.count(i)) and (dist == INT_MAX or local.dist(node[i]) < dist)) {
          dist = local.dist(node[i]);
          player = i;
        }
      }
      scores[player] += 1;
      ++id;
    }
    // cerr << P << ";" << scores[P] << ";" << scores[1 - P] << endl;
    return scores[P];
  }

  int space(const array<Pos, 4> &node) {
    // array<int, 4> scores = {0};
    // for (auto i : alive) {
    auto path = bfs(node[P]);
    // scores[i] = path.size();
    // }
    return path;  //.size()
  }

  int heuristic(const array<Pos, 4> &node, bool trace = false) {
    int v = voronoi(node);
    int s = space(node);
    if (trace) cerr << "v:" << v << " s:" << s << endl;
    return v + s;
  }

  int minmax(array<Pos, 4> &node, int depth, int player, Pos &action, int &alpha, int &beta) {
    auto mine = neighbors(node[P]);
    // cerr << "p" << player << " " << node[P] << endl;
    if (depth == 0 or mine.size() == 0 or TIME > 0.085) {
      int res = heuristic(node, true);
      if (mine.size() == 0) res -= 1000;
      cerr << "  f " << node[P] << " " << node[1 - P] << " " << res << endl;
      // display();
      return res;
    }
    auto current = node[player];
    if (player == P) {
      int value = -10000000;
      for (auto &n : neighbors(node[player])) {
        node[player] = n;
        grid[n.idx()] = player + 1;
        int m = minmax(node, depth - 1, (player + 1) % (alive.size()), action, alpha, beta);
        if (m >= value) {
          value = m;
          if (depth == maxDepth) action = n;
        }
        // cerr << "n0 " << n << " " << value<<" " <<alpha<<" "<<beta << endl;
        node[player] = current;
        grid[n.idx()] = 0;
        if (value > beta) {
          return value;
        }
        alpha = max(alpha, value);
      }
      return value;
    } else {
      int value = 10000000;
      for (auto &n : neighbors(node[player])) {
        node[player] = n;
        grid[n.idx()] = player + 1;
        value = min(value, minmax(node, depth - 1, (player + 1) % (alive.size()), action, alpha, beta));
        // cerr << " n1 " << n <<" " <<alpha<<" "<<beta << endl;
        node[player] = current;
        grid[n.idx()] = 0;
        if (value < alpha) {
          return value;
        }
        beta = min(beta, value);
      }
      return value;
    }
  }

  void dump() {
    cerr << "dump" << endl;
    heuristic(players, true);
  }

  void bestPos() {
    Pos action;
    Pos saved(players[P]);
    int alpha(-10000000), beta(10000000);
    int res = minmax(players, maxDepth, P, action, alpha, beta);
    // cerr << "res " << res << " " << action << endl;
    // saved.x = players[P].x; saved.y = players[P].y;
    /*Pos jeu;
    int score(-600), possible(0);
    for (auto n : neighbors(saved)) {
      players[P] = n;
      auto path = bfs(n);
      int rt(0);
      int v = voronoi(players);
      int local = v + 2 * path.size();
      // cerr << "n " << n.x << ";" << n.y << " s " << v << " p " << path.size() << " " << 600 - 2 * turn << endl;
      if (local >= score) {
        jeu = n;
        score = local;
      }
    }*/
    // cerr << "jeu " << jeu.x << ";" << jeu.y << " " << jeu.x + 30 * jeu.y << endl;
    // cerr << "saved " << saved.x << ";" << saved.y << " " << action.x << ";" << action.y << endl;
    cout << adir[action - saved] << endl;
  }
};

int main() {
  Board theBoard;
  while (1) {
    theBoard.init();
    Timer theClock;
    // theBoard.display();
    theBoard.bestPos();
    theBoard.dump();
  }
}