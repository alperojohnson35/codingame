// 210

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
  inline int denorm() { return this->x + 12 * this->y; }
  int dist(const Point &p) { return (this->x - p.x) * (this->x - p.x) + (this->y - p.y) * (this->y - p.y); }
  int man(const Point &p) { return abs(this->x - p.x) + abs(this->y - p.y); }

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

typedef enum Type { NONE = -1, HQ = 0, MINE = 1, TOWER = 2, UNIT = 3, MOVE = 4 } Type_t;
struct Unit : public Point {
  bool actif;
  int owner, id, type, level;
  int nbMur, nbEmpty, nbMoi, nbLui;
  inline Unit()
      : Point(), actif(false), owner(-1), id(0), type(NONE), level(0), nbMur(0), nbEmpty(0), nbMoi(0), nbLui(0) {}
  inline Unit(int x, int y)
      : Point(x, y), actif(false), owner(-1), id(0), type(NONE), level(0), nbMur(0), nbEmpty(0), nbMoi(0), nbLui(0) {}
  inline int change() const {
    int r{0};
    switch (owner) {
      case 0:
        r = 2;
        break;
      case 1:
        r = 1;
        break;
    }
    return r;
  }
};

namespace std {
template <>
struct hash<Point> {
  inline size_t operator()(const Point &p) const { return p.x * 1812433253 + p.y; }
};
template <>
struct hash<Unit> {
  inline size_t operator()(const Unit &p) const { return p.x * 1812433253 + p.y; }
};
}  // namespace std

template <typename T, typename priority_t>
struct PriorityQueue {
  typedef std::pair<priority_t, T> PQElement;
  std::priority_queue<PQElement, std::vector<PQElement>, std::greater<PQElement>> elements;

  inline bool empty() const { return elements.empty(); }

  inline void put(T item, priority_t priority) { elements.emplace(priority, item); }

  T get() {
    T best_item = elements.top().second;
    elements.pop();
    return best_item;
  }
};

struct Action {
  string action;
  int id, building;
  Point pos;
  string message;
  const array<string, 2> bat = {"MINE", "TOWER"};

  inline Action() : action("WAIT"), id(-1), building(-1), pos(-1, -1), message("") {}
  // inline Action &operator=(Action const &) = default;
  // inline Action &operator=(Action &&) = default;

  inline string write() const {
    string out(action);
    if (id > 0) {
      out += " " + to_string(id);
    }
    if (building > 0) {
      out += " " + bat[building - 1];
    }
    if (pos.x >= 0) {
      out += " " + to_string(pos.x) + " " + to_string(pos.y);
    }
    out += ";" + message;
    return out;
  }
};

struct Board {
  int width, height, nbMineSpot, gold, income, opgold, opincome, nbBuilding, nbUnit, H;
  vector<Unit> mines, buildings, units, hisUnits, mesCases, sesCases;
  unordered_set<Unit> maZone, saZone, maInactive, saInactive, walls;
  Unit hq{-1, 0};
  Unit maison{-1, 0};
  Unit tow{10, 10};
  array<Unit, 144> carte;
  vector<Action> actions;
  const array<Point, 4> cross0{Point{1, 0}, Point{0, 1}, Point{-1, 0}, Point{0, -1}};
  const array<Point, 4> cross1{Point{-1, 0}, Point{0, -1}, Point{1, 0}, Point{0, 1}};
  array<Point, 4> cross{Point{1, 0}, Point{0, -1}, Point{-1, 0}, Point{0, 1}};

  Board() {
    width = 12;
    height = 12;
    cin >> nbMineSpot;
    cin.ignore();
    for (int i(0); i < nbMineSpot; ++i) {
      Unit q;
      cin >> q.x >> q.y;
      cin.ignore();
      mines.push_back(q);
      carte[q.denorm()].type = MINE;
    }
    for (int i = 0; i < 144; ++i) {
      carte[i].x = i % width;
      carte[i].y = i / width;
    }
  }

  Point norm(int id) { return Point(id % 12, id / 12); }

  void init() {
    maZone.clear();
    mesCases.clear();
    sesCases.clear();
    saZone.clear();
    maInactive.clear();
    saInactive.clear();
    // mines.clear();
    buildings.clear();
    units.clear();
    hisUnits.clear();
    actions.clear();
    cin >> gold;
    cin.ignore();
    cin >> income;
    cin.ignore();
    cin >> opgold;
    cin.ignore();
    cin >> opincome;
    cin.ignore();
    for (int j(0); j < width; j++) {
      string row;
      getline(cin, row);
      for (int i(0); i < row.size(); ++i) {
        int id = i + 12 * j;
        carte[id].level = 0;
        switch (row[i]) {
          case '#':
            walls.insert(Unit(i, j));
            break;
          case '.':
            break;
          case 'O':
            maZone.insert(Unit(i, j));
            mesCases.push_back(Unit{i, j});
            carte[id].owner = 0;
            carte[id].actif = true;
            break;
          case 'o':
            maInactive.insert(Unit(i, j));
            carte[id].owner = 0;
            carte[id].actif = false;
            break;
          case 'X':
            saZone.insert(Unit(i, j));
            sesCases.push_back(Unit{i, j});
            carte[id].owner = 1;
            carte[id].actif = true;
            break;
          case 'x':
            saInactive.insert(Unit(i, j));
            carte[id].owner = 1;
            carte[id].actif = false;
            break;
          default:
            break;
        }
      }
    }
    if (hq.x == -1) {
      if (carte[0].owner == 0) {
        hq.x = 11;
        hq.y = 11;
        maison.x = 0;
        maison.y = 0;
        tow.x = 1;
        tow.y = 1;
        cross = cross0;
      } else {
        hq.x = 0;
        maison.x = 11;
        maison.y = 11;
        cross = cross1;
      }
    }
    cin >> nbBuilding;
    cin.ignore();
    for (int i(0); i < nbBuilding; ++i) {
      Unit b;
      cin >> b.owner >> b.type >> b.x >> b.y;
      cin.ignore();
      buildings.push_back(b);
      if (b.type == 2) {
        for (auto v : neighbors(b)) {
          carte[v.denorm()].type = TOWER;
        }
        carte[b.denorm()].type = TOWER;
      } else {
        carte[b.denorm()].type = b.type;
      }
    }
    cin >> nbUnit;
    cin.ignore();
    for (int i = 0; i < nbUnit; i++) {
      Unit u;
      cin >> u.owner >> u.id >> u.level >> u.x >> u.y;
      cin.ignore();
      auto v = neighbors(u);
      if (u.owner == 0) {
        units.push_back(u);
      } else {
        hisUnits.push_back(u);
      }
      carte[u.denorm()].level = u.level;
    }
    cerr << "gold " << gold << endl;
    sort(mesCases.begin(), mesCases.end(), [&hq = hq](Unit a, Unit b) { return a.dist(hq) < b.dist(hq); });
  }

  inline bool inMap(Unit p) const { return 0 <= p.x && p.x < width && 0 <= p.y && p.y < height; }
  inline bool isWall(Unit id) const { return walls.count(id); }
  inline bool isFree(Unit t, int level) const {
    return level >= 3 or (carte[t.denorm()].type == NONE and level > carte[t.denorm()].level);
  }
  vector<Unit> neighbors(Unit &p) const {
    vector<Unit> results;
    p.nbMoi = 0;
    p.nbLui = 0;
    p.nbMur = 0;
    p.nbEmpty = 0;
    for (auto d : cross) {
      Unit next(p.x + d.x, p.y + d.y);
      if (inMap(next) and !isWall(next)) {
        results.push_back(next);
        switch (carte[next.denorm()].owner) {
          case -1:
            p.nbEmpty++;
            break;
          case 0:
            p.nbMoi++;
            break;
          case 1:
            p.nbLui++;
            break;
        }
      } else {
        p.nbMur++;
      }
    }
    return results;
  }

  int cost(Unit &u) const {
    auto loc = carte[u.denorm()];
    if (loc.type == TOWER or loc.level > 1) {
      return 30;
    } else if (loc.level == 1) {
      return 20;
    } else {
      return 10;
    }
  }
  unordered_map<Unit, int> bfs(Unit &start) {
    queue<Unit> frontier;
    frontier.push(start);

    unordered_map<Unit, int> path;
    path[start] = 0;

    while (!frontier.empty()) {
      auto current = frontier.front();
      frontier.pop();

      auto n = neighbors(current);
      /*if (current.nbMur > 0 or current.nbMoi > 0 or current.nbEmpty > 0) {
        break;
      }*/

      for (auto next : n) {
        if (!path.count(next) and saZone.count(next)) {
          frontier.push(next);
          path[next] = 1 + path[current];
        }
      }
    }
    path.erase(start);
    return path;
  }

  template <typename Location>
  void dijkstra(Location start,
                Location goal,
                std::unordered_map<Location, Location> &came_from,
                std::unordered_map<Location, int> &cost_so_far) {
    PriorityQueue<Location, int> frontier;
    frontier.put(start, 0);

    came_from[start] = start;
    cost_so_far[start] = 0;

    while (!frontier.empty()) {
      Location current = frontier.get();

      if (current == goal) {
        break;
      }

      for (Location next : neighbors(current)) {
        int new_cost = cost_so_far[current] + cost(next);
        if (cost_so_far.find(next) == cost_so_far.end() || new_cost < cost_so_far[next]) {
          cost_so_far[next] = new_cost;
          came_from[next] = current;
          frontier.put(next, new_cost);
        }
      }
    }
  }
  template <typename Location>
  std::vector<Location> path(Location start, Location goal, std::unordered_map<Location, Location> came_from) {
    std::vector<Location> path;
    Location current = goal;
    while (current != start) {
      path.push_back(current);
      current = came_from[current];
    }
    path.push_back(start);  // optional
    std::reverse(path.begin(), path.end());
    return path;
  }

  inline bool inZone(Unit &u, int owner) const { return carte[u.denorm()].owner == owner; }
  vector<Unit> copains(Unit &p, int owner) const {
    vector<Unit> results;
    for (auto d : cross) {
      Unit next(p.x + d.x, p.y + d.y);
      if (inMap(next) and !isWall(next) and inZone(next, owner)) {
        results.push_back(next);
      }
    }
    return results;
  }
  unordered_map<Unit, int> hfs(const Unit &start, int owner) {
    queue<Unit> frontier;
    frontier.push(start);
    unordered_map<Unit, int> path;
    path[start] = 0;
    while (!frontier.empty()) {
      auto current = frontier.front();
      frontier.pop();
      for (auto next : copains(current, owner)) {
        if (!path.count(next)) {
          frontier.push(next);
          path[next] = 1 + path[current];
        }
      }
    }
    // path.erase(start);
    return path;
  }
  void build(Unit &c, int type) {
    Action action;
    action.pos = c;
    int bid = min(++carte[c.denorm()].level, 3);
    if (carte[c.denorm()].type == TOWER) {
      bid = 3;
    }
    int cout{0};
    switch (type) {
      case UNIT:
        action.id = bid;
        cout = 10 * action.id;
        action.action = "TRAIN";
        break;
      case TOWER:
        action.action = "BUILD";
        action.building = TOWER;
        cout = 15;
        break;
      case MINE:
        action.action = "BUILD";
        action.building = MINE;
        cout = 20;
        break;
      case MOVE:
        action.id = c.id;
        action.action = "MOVE";
        break;
    }
    if (cout <= gold) {
      gold -= cout;
      cerr << action.id << " do " << action.action << " in " << action.pos.x << " " << action.pos.y << endl;
      actions.push_back(action);
    }
  }
  void train(Unit &c) {
    if (gold < 10) return;
    auto n = neighbors(c);
    sort(n.begin(), n.end(), [&hq = hq](Unit a, Unit b) { return a.dist(hq) < b.dist(hq); });
    for (auto v : n) {
      cerr << " try unit in " << v.x << " " << v.y << " " << carte[v.denorm()].level << endl;
      if (carte[v.denorm()].level == 0) {
        build(v, UNIT);
        train(v);
        if (gold < 10) break;
      }
    }
  }

  inline bool frontiere() {
    for (auto c : mesCases) {
      auto v = neighbors(c);
      if (c.nbLui > 0) {
        cerr << "FRONTIERE" << endl;
        return true;
      }
    }
    return false;
  }

  void towerDefense() {
    if (frontiere()) {
      for (auto c : mesCases) {
        auto car = carte[c.denorm()];
        // if (carte[tow.denorm()].type == NONE and carte[tow.denorm()].owner == 0) {
        // build(tow, TOWER);
        //}
        if (car.type == NONE and car.level == 0) {
          build(c, TOWER);
          break;
        }
      }
    }
  }
  void rushProtect() {
    if (hisUnits.size()) {
      sort(hisUnits.begin(), hisUnits.end(), [&maison = maison](Unit a, Unit b) {
        return a.dist(maison) < b.dist(maison);
      });
      if (static_cast<int>((opgold + opincome) / 10) > hisUnits[0].man(maison)) {
        unordered_map<Unit, Unit> from;
        unordered_map<Unit, int> cout;
        dijkstra(maison, hisUnits[0], from, cout);
        auto chemin = path(maison, hisUnits[0], from);
        for (auto c : chemin) {
          if (carte[c.denorm()].owner == 0 and carte[c.denorm()].type == NONE) {
            cerr << "tower power in " << c.x << " " << c.y << endl;
            break;
          }
        }
      }
    }
  }
  int prix(Unit u) {
    if (u.level >= 2 or u.type == TOWER) {
      return 30;
    } else if (u.level == 1) {
      return 20;
    }
    return 10;
  }
  bool tentativeRush(Unit &u, vector<Unit> &v, int &g) {
    bool found{false};
    auto h = hfs(sesCases[0], 1);
    if (h.size() < H) {
      return true;
    }
    for (auto n : neighbors(u)) {
      if (carte[n.denorm()].owner == 1 and g >= prix(carte[n.denorm()])) {
        sesCases.erase(find(sesCases.begin(), sesCases.end(), n));
        g -= prix(carte[n.denorm()]);
        found = tentativeRush(n, v, g);
        if (found) {
          v.push_back(n);
        } else {
          sesCases.push_back(n);
          g -= prix(carte[n.denorm()]);
        }
      }
    }
    return found;
  }
  void moteur() {
    // petit test hfs
    auto h = hfs((*maZone.begin()), 0);
    auto mt{h.size()};
    h = hfs((*saZone.begin()), 1);
    auto st{h.size()};
    H = st;
    cerr << "moi: " << mt << " lui: " << st << endl;

    towerDefense();
    // units
    sort(units.begin(), units.end(), [&hq = hq](Unit a, Unit b) {
      if (a.nbEmpty == b.nbEmpty) {
        return a.dist(hq) < b.dist(hq);
      } else {
        return a.nbEmpty < b.nbEmpty;
      }
    });
    for (auto u : units) {
      // cerr << "unit " << u.id << " nbs" << u.nbEmpty << " " << u.nbMoi << " " << u.nbLui << " " << u.nbMur << endl;
      auto v = neighbors(u);
      sort(v.begin(), v.end(), [&hq = hq](Unit a, Unit b) { return a.dist(hq) < b.dist(hq); });
      vector<Unit> possibleMove;
      for (auto n : v) {
        auto car = carte[n.denorm()];
        if (car.owner == -1 or (car.owner == 1 and car.level == 0 and car.type == NONE) or
            (car.owner == 0 and u.nbLui == 0 and car.level == 0)) {
          n.id = u.id;
          n.owner = car.owner;
          possibleMove.push_back(n);
          cerr << "add " << n.x << " " << n.y << " for " << u.id << " owned" << car.owner << endl;
        }
      }
      for (auto p : possibleMove) {
        int g{gold};
        vector<Unit> v;
        auto rush = tentativeRush(p, v, g);
        if (rush) {
          cerr << "RUUUUSH" << endl;
          build(p, MOVE);
          for (auto it = v.rbegin(); it < v.rend(); ++it) {
            build(*it, UNIT);
          }
          return;
        }
      }
      if (possibleMove.size()) {
        sort(possibleMove.begin(), possibleMove.end(), [&, &m = hq](Unit a, Unit b) {
          auto n1 = this->neighbors(a);
          auto n2 = this->neighbors(b);
          if (a.man(m) == b.man(m)) {
            if (a.nbEmpty == b.nbEmpty) {
              return a.change() < b.change();
            } else {
              return a.nbEmpty > b.nbEmpty;
            }
          } else {
            return a.dist(m) < b.dist(m);
          }
        });
        build(possibleMove[0], MOVE);
        mesCases.push_back(possibleMove[0]);
      }
    }

    sort(mesCases.begin(), mesCases.end(), [&hq = hq](Unit a, Unit b) { return a.dist(hq) < b.dist(hq); });
    for (auto c : mesCases) {
      if (gold < 10) break;
      auto n = neighbors(c);
      if (c.nbEmpty > 0) {
        cerr << "train " << c.x << " " << c.y << endl;
        train(c);
      }
    }
  }

  void jeu() {}

  inline void play() {
    string s;
    if (!actions.size()) s = "WAIT";
    for (auto a : actions) {
      s += a.write();
    }
    cout << s << endl;
  }
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