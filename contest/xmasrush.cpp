// 271

#pragma GCC target("avx")
#pragma GCC optimize("O3")
#pragma GCC optimize("omit-frame-pointer")
#pragma GCC optimize("unsafe-math-optimizations")
#pragma GCC optimize("unroll-all-loops")
#pragma GCC optimize("inline")

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

template <typename T, typename priority_t>
struct PriorityQueue {
  typedef pair<priority_t, T> PQElement;
  priority_queue<PQElement, vector<PQElement>, std::greater<PQElement> > elements;

  inline bool empty() const { return elements.empty(); }

  inline void put(T item, priority_t priority) { elements.emplace(priority, item); }

  inline T get() {
    T best_item = elements.top().second;
    elements.pop();
    return best_item;
  }
};

struct Pos {
  int x, y;
  inline Pos() = default;
  inline Pos(const Pos &) = default;
  inline Pos(int x, int y) : x(x), y(y) {}
  inline Pos &operator=(Pos const &) = default;
  inline Pos &operator=(Pos &&) = default;
  inline bool operator==(const Pos &c) const { return this->x == c.x and this->y == c.y; }
  inline bool operator!=(const Pos &c) const { return !((*this) == c); }
  inline bool operator<(const Pos &c) const { return this->x + 100 * this->y < c.x + 100 * c.y; }
  inline Pos operator+(const Pos &c) const { return Pos(this->x + c.x, this->y + c.y); }
  inline Pos operator-(const Pos &c) const { return Pos(this->x - c.x, this->y - c.y); }
  inline int dist(const Pos &c) { return abs(this->x - c.x) + abs(this->y + c.y); }
};

array<Pos, 4> dirs = {Pos{0, -1}, Pos{1, 0}, Pos{0, 1}, Pos{-1, 0}};

struct Tile : Pos {
  int player;
  string type;
  string item;
  inline Tile() : player(-1), type("1111"), item("") {}
  inline Tile(const Tile &) = default;
  inline Tile &operator=(Tile const &) = default;
  inline Tile &operator=(Tile &&) = default;
  inline bool operator==(const Tile &c) const { return this->type == c.type; }
  inline bool operator!=(const Tile &c) const { return !((*this) == c); }
  inline bool operator<(const Tile &c) const { return this->type < c.type; }

  virtual void serialize(ostream &os) const {
    os << "[" << x << "," << y << " #" << player << ", " << type << ", " << item;
  }
};
ostream &operator<<(ostream &os, const Tile &c) {
  c.serialize(os);
  return os;
}

struct Player : Pos {
  int nbCard;
  Tile t;
};

struct Item : Pos {
  int player;
  string item;
};

struct Board {
  int turnType, numItems, numQuests;
  array<Tile, 49> tiles;
  array<Player, 2> players;
  array<Item, 2> items;
  array<Item, 2> quests;
  map<Pos, string> adir = {{Pos{0, -1}, "UP"}, {Pos{0, 1}, "DOWN"}, {Pos{-1, 0}, "LEFT"}, {Pos{1, 0}, "RIGHT"}};

  Board() {
    for (int i(0); i < 49; ++i) {
      tiles[i].x = i % 7;
      tiles[i].y = i / 7;
    }
  }

  void init() {
    cin >> turnType;
    cin.ignore();
    for (int j = 0; j < 7; j++) {
      for (int i = 0; i < 7; i++) {
        string tile;
        cin >> tile;
        tiles[i + 7 * j].x = i;
        tiles[i + 7 * j].y = j;
        tiles[i + 7 * j].type = tile;
        cin.ignore();
      }
    }
    for (int i = 0; i < 2; i++) {
      string playerTile;
      cin >> players[i].nbCard >> players[i].x >> players[i].y >> playerTile;
      cin.ignore();
      players[i].t.type = playerTile;
      tiles[players[i].x + 7 * players[i].y].player = i;
    }
    // items en jeu
    cin >> numItems;
    cin.ignore();
    for (int i = 0; i < numItems; i++) {
      cin >> items[i].item >> items[i].x >> items[i].y >> items[i].player;
      cin.ignore();
      // cerr << items[i].item << " " << items[i].x << " " << items[i].y << " " << items[i].player << endl;
      if (items[i].x >= 0) {
        tiles[items[i].x + 7 * items[i].y].item = items[i].item;
        tiles[items[i].x + 7 * items[i].y].player = items[i].player;
      }
    }
    // items a chercher
    cin >> numQuests;
    cin.ignore();
    for (int i = 0; i < numQuests; i++) {
      cin >> quests[i].item >> quests[i].player;
      cin.ignore();
    }
  }

  vector<int> neighbors(int t) {
    vector<int> voisins;
    for (int i(0); i < 4; ++i) {
      Pos n(tiles[t] + dirs[i]);
      if (n.x >= 0 and n.x < 7 and n.y >= 0 and n.y < 7) {
        int k(0);
        switch (i) {
          case 0:  // UP
            k = -7;
            break;
          case 1:  // RIGHT
            k = 1;
            break;
          case 2:  // DOWN
            k = 7;
            break;
          case 3:  // LEFT
            k = -1;
            break;
        }
        // cerr << n.x << "," << n.y << ";" << t + k << ";" << tiles[t + k] << " " << i + 2 % 4 << " " << endl;
        if (tiles[t].type[i] - '0' == 1 and tiles[t + k].type[(i + 2) % 4] - '0' == 1) {
          voisins.push_back(t + k);
        }
      }
    }
    return voisins;
  }

  unordered_map<int, int> bfs(int &start, int item) {
    queue<int> frontier;
    frontier.push(start);

    unordered_map<int, int> path;
    path[start] = 0;

    while (!frontier.empty()) {
      auto current = frontier.front();
      frontier.pop();
      if (current == item) break;

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

  inline double heuristic(Pos a, Pos b) { return abs(a.x - b.x) + abs(a.y - b.y); }

  /*void a_star(Pos &start, Pos &goal, unordered_map<Pos, Pos> &came_from, unordered_map<Pos, double> &cost) {
    PriorityQueue<Pos, double> frontier;
    frontier.put(start, 0);

    came_from[start] = start;
    cost[start] = 0;

    while (!frontier.empty()) {
      auto current = frontier.get();

      if (current == goal) {
        break;
      }

      for (auto next : tiles[current.x + 7 * current.y].around()) {
        double new_cost = cost[current] + 1;
        if (!cost.count(next) || new_cost < cost[next]) {
          cost[next] = new_cost;
          double priority = new_cost + heuristic(next, goal);
          frontier.put(next, priority);
          came_from[next] = current;
        }
      }
    }
  }*/

  void display() {
    // for (auto t : tiles) {}
    Tile t = tiles[40];
    cerr << t << endl;
    for (auto v : neighbors(40)) {
      cerr << v << endl;
    }
  }

  void play() {
    int maxMove(0);
    // rows
    for (int j(0); j < 7; ++j) {
      // move right
      Tile temp = tiles[7 * j + 6];
      for (int i(5); i >= 0; --i) {
        tiles[7 * j + i + 1] = tiles[7 * j + i];
      }
      if (tiles[7 * j + 6].player >= 0) {
        tiles[7 * j] = temp;
      } else {
        tiles[7 * j] = players[0].t;
      }
      cerr << "move " << j << endl;
      auto path = bfs(players[0].x + 7 * players[0].y, items[0].x + 7 * items[0].y);
      if (path.count(items[0].x + 7 * items[0].y)) display();
      // move back
      for (int i(1); i < 7; ++i) {
        tiles[7 * j + i - 1] = tiles[7 * j + i];
      }
      tiles[7 * j + 6] = temp;
    }
    if (turnType) {
      cout << "MOVE DOWN" << endl;  // PUSH <id> <direction> | MOVE <direction> | PASS
    } else {
      cout << "PUSH 3 RIGHT" << endl;  // PUSH <id> <direction> | MOVE <direction> | PASS
    }
  }
};
/**
 * Help the Christmas elves fetch presents in a magical labyrinth!
 **/
int main() {
  Board b;
  // game loop
  while (1) {
    b.init();
    b.play();
    b.display();
  }
}
