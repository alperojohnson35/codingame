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

typedef enum chifu { ROCK = 0, PAPER, SCISSORS, DEAD } chifu_t;
unordered_map<chifu_t, string> chifumi = {{ROCK, "ROCK"}, {PAPER, "PAPER"}, {SCISSORS, "SCISSORS"}, {DEAD, "DEAD"}};
unordered_map<string, chifu_t> shifumi = {{"ROCK", ROCK}, {"PAPER", PAPER}, {"SCISSORS", SCISSORS}, {"DEAD", DEAD}};
struct Cell : Point {
  int value, cooldown;
  bool visited, occupied;
  vector<Point> edges;
  inline Cell() : value(0), cooldown(0), visited(false), occupied(false), Point{} {}
  inline Cell(int x, int y) : value(0), cooldown(0), visited(false), occupied(false), Point{x, y} {}
  virtual void serialize(ostream &os) const {
    Point::serialize(os);
    os << ", va:" << value << ", v:" << visited << " ,o:" << occupied;
  }
};
ostream &operator<<(ostream &os, const Cell &c) {
  c.serialize(os);
  return os;
}

typedef enum choice { RIGHT, LEFT, NEAR, FFX, FFY, CORNER } choice_t;
typedef enum card { NORTH = 0, WEST, SOUTH, EAST } card_t;
typedef enum action { NONE, SPEED, MOVE, SWITCH } action_t;
struct Pac : Point {
  int s, ability, eaten;
  chifu_t type, new_type;
  Point target, from;
  action_t action;

  inline Pac()
      : Point{},
        s(0),
        ability(0),
        eaten(0),
        type(ROCK),
        new_type(PAPER),
        action(NONE),
        target(Point{-1, 0}),
        from(Point{0, 0}) {}
  inline Pac(int x, int y)
      : Point{x, y},
        s(0),
        ability(0),
        eaten(0),
        type(ROCK),
        new_type(PAPER),
        action(NONE),
        target(Point{-1, 0}),
        from(Point(0, 0)) {}
  inline Pac(Pac const &) = default;
  inline Pac(Pac &&) = default;
  inline Pac &operator=(Pac const &) = default;
  inline Pac &operator=(Pac &&) = default;
  inline bool operator==(const Pac &c) const { return this->x == c.x and this->y == c.y; }
  inline bool operator!=(const Pac &c) const { return !((*this) == c); }
  inline bool operator<(const Pac &c) const { return this->x + 8 * this->y < c.x + 8 * c.y; }
  inline Pac operator+(const Pac &c) const { return Pac(this->x + c.x, this->y + c.y); }
  inline Pac operator-(const Pac &c) const { return Pac(this->x - c.x, this->y - c.y); }

  string play() const {
    switch (action) {
      case MOVE:
        return "MOVE " + to_string(i) + " " + to_string(target.x) + " " + to_string(target.y) + "|";
        break;
      case SPEED:
        return "SPEED " + to_string(i) + "|";
        break;
      case SWITCH:
        return "SWITCH " + to_string(i) + " " + chifumi[new_type] + "|";
        break;
      default:
        return "WAIT";
        break;
    }
  }
  virtual void serialize(ostream &os) const {
    Point::serialize(os);
    os << ", a:" << action << " t:" << target << " e:" << eaten;
  }
};
ostream &operator<<(ostream &os, const Pac &c) {
  c.serialize(os);
  return os;
}

namespace std {
template <>
struct hash<Point> {
  inline size_t operator()(const Point &p) const { return p.x * 1812433253 + p.y; }
};
template <>
struct hash<Pac> {
  inline size_t operator()(const Pac &p) const { return p.x * 1812433253 + p.y; }
};
}  // namespace std
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
template <typename T>
void printList(const T &t) {
  for (auto e : t) {
    cerr << "##" << e << endl;
  }
}

struct Board {
  int width, height, myScore, hisScore, gamePac, gameBullet, nbPac;
  int nzx, nzy, nturn, eaten;
  bool inSight;
  bool firstTurn = true;
  Cell grid[40][20];
  array<Pac, 5> moi, lui;
  vector<Point> dirs = {Point{0, 1}, Point{1, 0}, Point{0, -1}, Point{-1, 0}};
  vector<Point> dirn = {Point{0, -1}, Point{-1, 0}, Point{0, 1}, Point{1, 0}};
  vector<Point> dire = {Point{1, 0}, Point{0, 1}, Point{-1, 0}, Point{0, -1}};
  vector<Point> dirw = {Point{-1, 0}, Point{0, -1}, Point{1, 0}, Point{0, 1}};
  vector<Point> dirx = {Point{0, -1}, Point{-1, 0}, Point{0, 1}, Point{1, 0}};
  unordered_set<Point> walls, cross, chosen, valued, bigs;
  array<multimap<int, Point>, 5> bullets, crosses;
  array<multimap<int, Pac>, 5> ennemies;
  unordered_map<Point, card_t> cards = {
      {Point{-1, 0}, WEST}, {Point{1, 0}, EAST}, {Point{0, 1}, SOUTH}, {Point{0, -1}, NORTH}};

  void init() {
    cin >> width >> height;
    cin.ignore();
    for (int i = 0; i < height; i++) {
      string row;
      getline(cin, row);  // one line of the grid: space " " is floor, pound "#" is wall
      for (int j(0); j < row.size(); ++j) {
        if (row[j] == '#') {
          walls.insert(Point{j, i});
        }
        grid[j][i].x = j;
        grid[j][i].y = i;
      }
    }
    locateCross();
    nzx = width / 3;
    nzy = height / 3;
    if (width % 3) ++nzx;
    if (height % 3) ++nzy;
    cerr << "w:" << width << " h:" << height << endl;
    cerr << nzx << " x " << nzx << " zones and " << nzy << " y" << endl;
    nturn = 0;
  }
  int surface(const Point &p) { int s = nzx * (p.y / 3) + (p.x / 3); }
  Point surfaceCenter(int n) { return Point{3 * (n % nzx) + 1, 3 * (n / nzx) + 1}; }

  void locateCross() {
    for (size_t x = 0; x < width; x++) {
      for (size_t y = 0; y < height; y++) {
        auto n = neighbors(Point(grid[x][y]));
        if (n.size() > 2) {
          cross.insert(Point{x, y});
          grid[x][y].edges = n;
        }
      }
    }
  }
  void turnInit() {
    ++nturn;
    int xx(5), yy(7);
    Point pp(xx, yy);
    cerr << nturn << "init " << grid[xx][yy] << " " << walls.count(pp) << " |" << passable(pp) << endl;
    for (auto &b : bullets) b.clear();
    for (auto &c : crosses) c.clear();
    for (auto &e : ennemies) e.clear();
    chosen.clear();
    valued.clear();
    // bigs.clear();
    inSight = false;
    cin >> myScore >> hisScore;
    cin.ignore();
    cin >> gamePac;
    cin.ignore();
    vector<int> seen, viewed;
    eaten = 0;
    for (int i = 0; i < gamePac; ++i) {
      int id, x, y, s, a;
      bool mine;  // true if this pac is yours
      string t;
      cin >> id >> mine >> x >> y >> t >> s >> a;
      cerr << mine << "/" << id << "/" << s << "/" << a << endl;
      cin.ignore();
      if (mine) {
        if (!firstTurn or Point(moi[id]) != Point(x, y)) {
          moi[id].from.x = moi[id].x - x;
          moi[id].from.y = moi[id].y - y;
        }
        grid[moi[id].x][moi[id].y].occupied = false;
        moi[id].x = x;
        moi[id].y = y;
        moi[id].i = id;
        moi[id].s = s;
        moi[id].ability = a;
        moi[id].type = shifumi[t];
        moi[id].action = NONE;
        moi[id].eaten = 0;
        seen.push_back(id);
        if (firstTurn) {
          ++nbPac;
          lui[id].x = width - 1 - x;
          lui[id].y = y;
          grid[lui[id].x][lui[id].y].visited = true;
          grid[lui[id].x][lui[id].y].occupied = true;
          lui[id].i = id;
          lui[id].s = s;
          lui[id].ability = a;
          lui[id].type = shifumi[t];
        }
      } else {
        grid[lui[id].x][lui[id].y].occupied = false;
        lui[id].x = x;
        lui[id].y = y;
        lui[id].i = id;
        lui[id].s = s;
        lui[id].ability = a;
        lui[id].type = shifumi[t];
        viewed.push_back(id);
        inSight = true;
      }
      grid[x][y].value = 0;
      grid[x][y].visited = true;
      grid[x][y].occupied = true;
      setCrosses(moi[id]);
    }
    killPac(seen);
    resetTargets(viewed);
    distancePacs();
    cin >> gameBullet;
    cin.ignore();
    for (int i = 0; i < gameBullet; ++i) {
      int x, y, value;
      cin >> x >> y >> value;
      cin.ignore();
      if (grid[x][y].cooldown > 0) grid[x][y].cooldown--;
      grid[x][y].value = value;
      valued.insert(Point{x, y});
      // assign best bullets now
      if (firstTurn and grid[x][y].value == 10) bigs.insert(Point(x, y));
      if (grid[x][y].value == 10 and grid[x][y].cooldown == 0) {
        assignBigs(x, y);
      }
    }
    updateValues();
  }
  void setCrosses(const Pac &p) {
    for (auto c : cross) {
      auto d = a_star(Point(p), c).size();
      if (d > 0) crosses[p.i].insert({d, c});
    }
  }
  void updateValues() {
    for (auto m : moi) {
      if (m.i == -1) continue;
      // cerr << ">>" << m.i << endl;
      for (size_t i = 0; i < 4; i++) {
        auto p = Point(m) + dirs[i];
        while (passable(p)) {
          if (!valued.count(p)) {
            // cerr << " flush " << p.x << ";" << p.y << endl;
            grid[p.x][p.y].value = 0;
          }
          p = p + dirs[i];
        }
      }
    }
    for (auto b : bigs) {
      if (!valued.count(b)) grid[b.x][b.y].value = 0;
    }
  }
  void killPac(const vector<int> &seen) {
    if (seen.size() < nbPac) {
      for (int i(0); i < nbPac; ++i) {
        if (find(seen.begin(), seen.end(), i) == seen.end()) {
          moi[i].i = -1;  // kill
        }
      }
    }
  }
  void resetTargets(const vector<int> &seen) {
    if (firstTurn) return;
    for (int i(0); i < nbPac; ++i) {
      if (find(seen.begin(), seen.end(), i) == seen.end()) {
        cerr << "clean e" << i << endl;
        grid[lui[i].x][lui[i].y].occupied = false;
        lui[i].x = 39;
        lui[i].y = 19;
      }
    }
  }
  void distancePacs() {
    for (auto m : moi) {
      for (auto e : lui) {
        ennemies[m.i].insert({m.dist(e), e});
      }
    }
  }
  inline int heuristic(Point a, Point b) { return abs(a.x - b.x) + abs(a.y - b.y); }

  template <typename Location>
  bool passable(const Location &p, bool occup = false) {
    return p.x >= 0 and p.x < width and p.y >= 0 and p.y < height and !walls.count(Point(p)) and
           (occup or !grid[p.x][p.y].occupied);
  }
  card_t cardTrans(const card_t &c) {
    card_t v = NORTH;
    switch (c) {
      case NORTH:
        v = EAST;
        break;
      case EAST:
        v = NORTH;
        break;
      case WEST:
        v = SOUTH;
        break;
      case SOUTH:
        v = WEST;
        break;
    }
    return v;
  }
  template <typename Location>
  vector<Location> neighbors(const Location &p, card_t card = NORTH, bool occup = false) {
    vector<Location> res;
    auto vec = dirn;  // isDirn ? dirn : dire;
    auto rot = card;  // isDirn ? card : cardTrans(card);
    rotate(vec.begin(), vec.begin() + card, vec.end());
    for (auto d : vec) {
      Location n(p.x + d.x, p.y + d.y);
      // if (p.x == 17 and p.y == 9) cerr << "       ]" << n << endl;
      if (n.x == -1) n.x = width - 1;
      if (n.x == width) n.x = 0;
      if (passable(n, occup)) res.push_back(n);
    }
    return res;
  }

  template <typename Location>
  Location bfs(Location start, bool bullet = true) {
    std::queue<Location> frontier;
    frontier.push(start);

    std::unordered_map<Location, Location> came_from;
    came_from[start] = start;

    while (!frontier.empty()) {
      Location current = frontier.front();
      frontier.pop();

      if ((bullet and grid[current.x][current.y].value > 0) or (!bullet and !grid[current.x][current.y].visited))
        return current;

      for (Location next : neighbors(current)) {
        if (came_from.find(next) == came_from.end()) {
          frontier.push(next);
          came_from[next] = current;
        }
      }
    }
    return start;
  }

  template <typename Location>
  Location bfsCross(Location start) {
    std::queue<Location> frontier;
    frontier.push(start);

    std::unordered_map<Location, Location> came_from;
    came_from[start] = start;

    while (!frontier.empty()) {
      Location current = frontier.front();
      frontier.pop();

      if (cross.count(current) and grid[current.x][current.y].edges.size() > 0) return current;

      for (Location next : neighbors(current)) {
        if (came_from.find(next) == came_from.end()) {
          frontier.push(next);
          came_from[next] = current;
        }
      }
    }
    return start;
  }
  template <typename Location>
  Location dfs(Location start, card_t card = NORTH, int depth = 3) {
    std::stack<Location> frontier;
    int d(-1);
    frontier.push(start);
    int value(0);
    Location last;
    std::unordered_set<Location> visited;

    while (!frontier.empty()) {
      Location current = frontier.top();
      cerr << "      |" << current << ";" << grid[current.x][current.y].value << endl;
      frontier.pop();
      last = current;
      if (visited.find(current) != visited.end()) continue;
      // card_t lcard = d == -1 ? static_cast<card_t>(card + 1) : card;
      ++d;
      value += grid[current.x][current.y].value;
      if (value > 1 or d == depth) return current;
      visited.insert(current);
      auto n = neighbors(current);
      if (n.size() < 1 and grid[current.x][current.y].value > 0) break;
      // if (d > 0 and n.size() > 2) n = neighbors(current, static_cast<card_t>(card + 1));
      for (auto i = n.rbegin(); i < n.rend(); ++i) {
        Location next = *i;
        if (visited.find(next) == visited.end()) {
          frontier.push(next);
        }
      }
    }
    return last;
  }
  template <typename Location>
  vector<Location> a_star(const Location &start, const Location &goal) {
    PriorityQueue<Location, double> frontier;
    frontier.put(start, 0);
    unordered_map<Location, Location> from;
    unordered_map<Location, int> cost;
    from[start] = start;
    cost[start] = 0;
    while (!frontier.empty()) {
      auto current = frontier.get();
      if (current == goal) {
        break;
      }
      for (auto next : neighbors(current)) {
        int new_cost = cost[current] + 1;
        if (!cost.count(next) || new_cost < cost[next]) {
          cost[next] = new_cost;
          int priority = new_cost + heuristic(next, goal);
          frontier.put(next, priority);
          from[next] = current;
        }
      }
    }
    vector<Location> path;
    if (from.count(goal)) {
      Location current = goal;
      while (current != start) {
        path.push_back(current);
        current = from[current];
      }
      // path.push_back(start);
      reverse(path.begin(), path.end());
    }
    return path;
  }
  void assignBigs(int x, int y) {
    cerr << "bullet " << x << "," << y << grid[x][y] << endl;
    int max(width), idx(-1);
    nearest(moi, Point(x, y), &max, &idx);
    cerr << " found " << idx << " nearest " << endl;
    nearest(lui, Point(x, y), &max, &idx, false);
    if (idx < 42) {
      cerr << "  assign" << endl;
      moi[idx].target = Point{x, y};
      moi[idx].action = MOVE;
      moi[idx].eaten = 1;
      auto path = a_star(Point(moi[idx]), moi[idx].target);
      if (moi[idx].s > 0 and path.size() == 1) {
        int mx(moi[idx].x), my(moi[idx].y);
        moi[idx].x = x;
        moi[idx].y = y;
        moi[idx].target = nextCell(moi[idx], Point(mx, my));
        moi[idx].x = mx;
        moi[idx].y = my;
      }
    } else {
      cerr << "  dismiss " << x << " " << y << " " << 42 - idx << endl;
      grid[x][y].cooldown = a_star(Point(lui[42 - idx]), Point(x, y)).size();
      lui[42 - idx].action = MOVE;
    }
  }
  void nearest(const array<Pac, 5> &p, const Point &t, int *max, int *idx, bool me = true) {
    for (auto m : p) {
      if (m.i == -1 or (m.action != NONE)) continue;
      auto d = a_star(Point(m), t).size();
      // cerr << m << " dist " << *max << " d " << d << endl;
      if (d > 0 and d < *max) {
        *max = d;
        if (me) {
          *idx = m.i;
        } else {
          *idx = 42 + m.i;
        }
      }
    }
  }
  void findCross() {
    for (auto &m : moi) {
      if (m.i == -1 or m.action != NONE) continue;
      m.target = bfsCross(Point(m));
      for (auto c : crosses[m.i]) {
        if (grid[c.second.x][c.second.y].edges.size() > 0) {
          m.target = grid[c.second.x][c.second.y].edges.back();
          m.action = MOVE;
          grid[c.second.x][c.second.y].edges.pop_back();
        }
      }
      if (m.action == NONE or a_star(Point(m), m.target).size() == 1) continue;
    }
  }
  void findNearest() {
    cerr << "toFind " << endl;
    vector<choice_t> c = {RIGHT, LEFT, RIGHT, LEFT, RIGHT};
    for (auto &m : moi) {
      if (m.i == -1 or m.action != NONE) continue;
      auto previous = Point(m) + m.from;
      auto aa = a_star(Point(m), previous);
      cerr << aa.size() << "initial p " << previous << endl;
      if (aa.size() > 0) previous = aa[0];
      Point pill = nextCell(m, previous);
      if (grid[pill.x][pill.y].value != 0) ++m.eaten;
      if (m.s > 0) {
        int x(m.x), y(m.y);
        m.x = pill.x;
        m.y = pill.y;
        pill = nextCell(m, Point(x, y), false);
        if (grid[pill.x][pill.y].value != 0) ++m.eaten;
        m.x = x;
        m.y = y;
      }
      // m.target = dfs(Point(m), cards[pill]);
      m.target = pill;
      m.action = MOVE;
    }
  }
  Point nextCell(Pac &m, const Point &previous, bool first = true) {
    Point moi(m);
    auto from = previous - moi;
    // int lcard = (m.i % 2) ? cards[from] + 1 : cardTrans(cards[from]) + 1;
    int lcard = cards[from] + 1;
    lcard = lcard % 4;
    bool canMove = neighbors(m).size() > 1;
    cerr << "can move " << canMove << endl;
    if (canMove) walls.insert(previous);
    auto pills = neighbors(moi, static_cast<card_t>(lcard));
    cerr << m.i << "> " << first << ".from " << previous << "/" << from << lcard << " " << pills.size() << endl;
    if (pills.size() == 0) {
      if (canMove) walls.erase(previous);
      if (first) {
        auto v = neighbors(moi, static_cast<card_t>(lcard), true);
        cerr << " bloque " << v.size() << " " << v[0] << endl;
        if (v.size() > 0) return v[0];
      } else {
        return moi;
      }
    }
    if (pills.size() == 1) {
      if (canMove) walls.erase(previous);
      return pills[0];
    }
    Point pill{-2, 0};
    int max = 0;
    for (auto p : pills) {
      int score = 0;
      auto n = p;
      auto d = p - moi;
      while (passable(n) and !grid[n.x][n.y].occupied) {
        int value = grid[n.x][n.y].value == -1 ? 10 : grid[n.x][n.y].value;
        score += value;
        n = n + d;
      }
      if (score > max) {
        cerr << "  eat " << score << " in " << p << endl;
        pill = p;
        max = score;
        m.eaten++;
      }
    }
    if (pill.x == -2) {
      for (auto p : pills) {
        if (!grid[p.x][p.y].visited) {
          cerr << "  unk " << p.x << " " << p.y << ":" << grid[p.x][p.y].visited << endl;
          pill = p;
          break;
        }
      }
    }
    if (pill.x == -2) pill = pills[0];
    cerr << " next on " << pill << endl;
    if (canMove) walls.erase(previous);
    return pill;
  }

  void noPill() {
    cerr << "no pill " << endl;
    for (auto &m : moi) {
      if (m.i == -1 or m.action != MOVE or m.eaten != 0) continue;
      auto t = bfs(Point(m));
      if (t == Point(m)) {
        cerr << "recall" << endl;
        t = bfs(Point(m), false);
      }
      cerr << " <<" << m.i << " can see bullet " << t << endl;
      if (t.x != m.x or t.y != m.y) m.target = t;
    }
  }
  void toSwitch() {
    cerr << "toSwitch " << endl;
    for (auto &m : moi) {
      if (m.i == -1 or m.ability != 0) continue;
      auto e = ennemies[m.i].begin()->second;
      cerr << " mien #" << m.i << " vise #" << e << endl;
      if (canSee(m, e)) {
        if (m.type != beatChi(e)) {
          cerr << " je me transforme" << endl;
          m.new_type = beatChi(e);
          m.action = SWITCH;
          m.ability = 10;
        } else {
          m.target = ennemies[m.i].begin()->second;
          m.action = MOVE;
          cerr << " piste " << m.target << endl;
        }
      } else if (canCatch(e, m)) {
        cerr << m.i << " can be caught " << e.i << endl;
        m.new_type = beatChi(e);
        m.action = SWITCH;
        m.ability = 10;
      }
    }
  }
  bool canSee(const Pac &p1, const Pac &p2) const {
    if (p1.x == p2.x) {
      if (p1.y > p2.y) {
        return !areWall(p2.x, p2.y, p1.y - p2.y, false);
      } else {
        return !areWall(p1.x, p1.y, p2.y - p1.y, false);
      }
    }
    if (p1.y == p2.y) {
      if (p1.x > p2.x) {
        return !areWall(p2.x, p2.y, p1.x - p2.x);
      } else {
        return !areWall(p1.x, p1.y, p2.x - p1.x);
      }
    }
    return false;
  }
  bool areWall(int i, int j, int count, bool abs = true) const {
    for (int k = 1; k < count; k++) {
      int x, y;
      if (abs) {
        x = i + k;
        y = j;
      } else {
        x = i;
        y = j + k;
      }
      if (walls.count(Point{x, y})) {
        return true;
      }
    }
    return false;
  }
  void escape() {
    cerr << "escape" << endl;
    if (inSight) {
      for (auto &m : moi) {
        if (m.i == -1) continue;
        for (auto e : lui) {
          if (e.i == -1 or e.type == DEAD) continue;
          if (canCatch(e, m)) {
            cerr << " e#" << e.i << " can see " << m.i << endl;
            if (m.ability == 0) {
              m.new_type = beatChi(e);
              m.action = SWITCH;
              // m.ability = 10;
            } else {
              int d(0);
              Point t(m);
              auto ne = neighbors(m);
              for (auto n : ne) {
                if (n.dist(e) > 0) {
                  t = n;
                  d = n.dist(e);
                }
              }
              cerr << m.i << " flee from " << e.i << "/" << t << endl;
              m.target = t;
              m.action = MOVE;
            }
          }
        }
      }
    }
  }

  chifu_t beatChi(const Pac &p) const { return static_cast<chifu_t>(static_cast<int>(p.type + PAPER) % 3); }
  bool canCatch(Pac &m, const Pac &e) const {
    // cerr << "  #" << m.i << m.type << " " << e.type + PAPER << " s:" << m.s << " " << e.s << " " << m.dist(e) <<
    // endl;
    cerr << " <" << e.i << "<" << m.i << "   x " << m.ability << ":" << m.type << ":" << beatChi(e) << ":"
         << canSee(m, e) << endl;
    return (m.ability == 0 or m.type == beatChi(e)) and
           ((canSee(m, e) or firstTurn) and (m.dist(e) <= m.s - e.s or m.dist(e) == 1));
  }
  void toSpeed() {
    cerr << "toSpeed " << endl;
    for (auto &m : moi) {
      if (m.i == -1) continue;
      if (m.action == NONE or (m.ability == 0 and !inSight) or nturn == 1) {
        cerr << " speed " << m.i << endl;
        m.action = SPEED;
      }
    }
  }
  void canKill() {
    for (auto &m : moi) {
      if (m.i == -1) continue;
      for (auto e : lui) {
        if (e.i == -1) continue;
        if (m.dist(e) == 1 and e.ability > 0 and m.type == beatChi(e)) {
          cerr << "xkill " << m.i << ">" << e.i << endl;
          m.action = MOVE;
          m.target = Point(e);
        }
      }
    }
  }

  void bestAction() {
    // findBigPills();
    findNearest();
    // toSwitch();
    toSpeed();
    noPill();
    canKill();
    escape();
    preventCollisions();
  }
  void preventCollisions() {
    unordered_set<Point> ex;
    for (auto &m : moi) {
      if (m.i == -1 or m.action != MOVE) continue;
      auto p = a_star(Point(m), m.target);
      if (p.size() > 2) {
        p = vector<Point>(p.begin(), p.begin() + 2);
      }
      Point t(m);
      for (auto c : p) {
        if (ex.count(c)) {
          cerr << c << " this cell is already aimed " << m.i << " goes " << t << endl;
          m.target = Point(t);
          break;
        } else {
          ex.insert(c);
        }
        t = c;
      }
    }
  }

  void eatPills() {
    for (auto m : moi) {
      if (m.i == -1 or m.action != MOVE) continue;
      auto p = a_star(Point(m), m.target);
      int nb = m.s > 0 ? 2 : 1;
      for (int i = 0; i < nb and i < p.size(); ++i) {
        auto f = p[i];
        cerr << "   #" << m.i << "eat " << f << endl;
        grid[f.x][f.y].value = 0;
        grid[f.x][f.y].visited = true;
      }
    }
  }

  void playActions() {
    string a("");
    for (auto m : moi) {
      if (m.i == -1) continue;
      a += m.play();
    }
    cout << a << endl;
    if (firstTurn) firstTurn = false;
  }
};

int main() {
  Board theBoard;
  theBoard.init();
  while (1) {
    theBoard.turnInit();
    theBoard.bestAction();
    theBoard.eatPills();
    theBoard.playActions();
  }
}
