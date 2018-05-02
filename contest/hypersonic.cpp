#include <algorithm>
#include <array>
#include <cmath>
#include <iostream>
#include <map>
#include <queue>
#include <set>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

using namespace std;

struct Loc {
  int x, y;
  bool isValid;

 public:
  Loc() : x(-1), y(-1), isValid(false) {}
  Loc(int x_, int y_) : x(x_), y(y_), isValid(true) {}
  void set(int x_, int y_) {
    x = x_;
    y = y_;
  }
  void save(Loc &a) {
    x = a.x;
    y = a.y;
  }
  inline friend bool operator==(const Loc &a, const Loc &b) { return a.x == b.x and a.y == b.y; }
  inline friend bool operator!=(const Loc &a, const Loc &b) { return !(a == b); }
  friend bool operator<(const Loc &a, const Loc &b) { return tie(a.x, a.y) < tie(b.x, b.y); }
  virtual void serialize(ostream &os) const { os << " [" << x << "," << y << "] "; }
};
ostream &operator<<(ostream &os, const Loc &p) {
  p.serialize(os);
  return os;
}

struct Box : public Loc {
  int type;  // 0 empty, 1 range, 2 bomb
 public:
  Box() {}
  Box(int x_, int y_, int type_) : Loc(x_, y_), type(type_) {}
  bool hasItem() { return type > 0; }
  virtual void serialize(ostream &os) const {
    Loc::serialize(os);
    os << "type " << type;
  }
};

struct Bomb : public Loc {
  int nbRound, range;

 public:
  Bomb() : Loc(), nbRound(0), range(0) {}
  Bomb(int x_, int y_, int nbRound_, int range_) : Loc(x_, y_), nbRound(nbRound_), range(range_) {}
  void set(int x_, int y_, int nbRound_, int range_) {
    Loc::set(x_, y_);
    nbRound = nbRound_;
    range = range_;
  }
  int blow() const { return range - 1; }
  virtual void serialize(ostream &os) const {
    os << "Bomb";
    Loc::serialize(os);
    os << "range " << range << " nbRounds " << nbRound;
  }
};

struct Player : public Bomb {
  int nbBomb;

 public:
  Player() : Bomb(), nbBomb(0) {}
  Player(int x_, int y_, int nbBomb_, int range_) : Bomb(x_, y_, 0, range_), nbBomb(nbBomb_) {}
  void upgrade(int type) {
    if (type == 1) {
      ++range;
    } else {
      ++nbBomb;
    }
  }
  void set(int x_, int y_, int nbBomb_, int range_) {
    Bomb::set(x_, y_, 0, range_);
    nbBomb = nbBomb_;
  }
  void drop() {
    if (nbBomb > 0) --nbBomb;
  }
  virtual void serialize(ostream &os) const {
    os << "Player";
    Loc::serialize(os);
    os << "range " << range << " nbBomb " << nbBomb;
  }
};

namespace std {
template <>
struct hash<Loc> {
  inline size_t operator()(const Loc &p) const { return p.x * 1812433253 + p.y; }
};
template <>
struct hash<Box> {
  inline size_t operator()(const Box &b) const { return b.x * 1812433253 + b.y; }
};
template <>
struct hash<Bomb> {
  inline size_t operator()(const Bomb &b) const { return b.x * 1812433253 + b.y; }
};
}  // namespace std

struct Grid {
  static array<Loc, 4> BLOW;

  int width, height;
  unordered_set<Loc> walls;
  unordered_set<Loc> bombs;
  unordered_map<Loc, int> boxes;
  unordered_map<Loc, int> items;

  Grid() {}
  Grid(int width_, int height_) : width(width_), height(height_) {}
  inline void set(int w, int h) {
    width = w;
    height = h;
  }

  inline bool inGrid(Loc p) const { return 0 <= p.x && p.x < width && 0 <= p.y && p.y < height; }

  inline bool isBox(Loc id) const { return boxes.count(id); }

  inline bool isWall(Loc id) const { return walls.count(id); }

  inline bool isBomb(Loc id) const { return bombs.count(id); }

  inline bool isItem(Loc id) const { return items.count(id); }

  unordered_set<Loc> getBlow(Loc &p, int dist, int &nbBoxBlowed) const {
    nbBoxBlowed = 0;
    unordered_set<Loc> blow;

    if (!isBox(p)) {
      blow.insert(p);
      for (auto d : BLOW) {
        for (int k(1); k <= dist; ++k) {
          Loc next(p.x + k * d.x, p.y + k * d.y);
          if (isWall(next) || isBox(next) || isItem(next) || !inGrid(next) || isBomb(next)) {
            // no need to continue look in this direction
            if (isBox(next) || isItem(next) || isBomb(next)) {
              if (isBox(next)) {
                ++nbBoxBlowed;
              }
              blow.insert(next);
            }
            break;
          } else {
            blow.insert(next);
          }
        }
      }
    }

    return blow;
  }

  vector<Loc> neighbors(Loc &p) const {
    vector<Loc> results;

    for (auto d : BLOW) {
      Loc next(p.x + d.x, p.y + d.y);
      if (inGrid(next) && !isBox(next) && !isWall(next) and !isBomb(next)) {
        results.push_back(next);
      }
    }

    return results;
  }

  void upItems(Loc &p, int blow) {
    for (auto d : BLOW) {
      for (int k(1); k <= blow; ++k) {
        Loc next(p.x + k * d.x, p.y + k * d.y);
        if (isWall(next) || isBox(next) || isItem(next) || !inGrid(next)) {
          // no need to continue look in this direction
          if (isBox(next)) boxes.erase(next);
          if (isItem(next)) items.erase(next);
          if (isBomb(next)) bombs.erase(next);
          break;
        }
      }
    }
    if (isWall(p)) walls.erase(p);
    bombs.erase(p);
  }

  Loc findSafe(Loc &p, int dir, int &dist) {
    dist = 0;
    Loc d(BLOW[dir].x, BLOW[dir].y);
    bool found = false;
    Loc f(p);
    while (!found) {
      dist++;
      Loc m(p.x + dist * d.x, p.y + dist * d.y);
      vector<Loc> next = neighbors(f);
      for (auto n : next) {
        if ((dir % 2 && n.x != p.x) || (!(dir % 2) && n.y != p.y)) {
          f = m;
          dist++;
          found = true;
          break;
        }
      }
    }
    return f;
  }
};

static Grid grid;
static Player player;
static vector<Bomb> bombs;
static vector<Loc> ennemies;
static map<int, Box> items;
static Loc target, theNextLoc;
static unordered_map<Loc, unordered_set<Loc>> bombRanges;

array<Loc, 4> Grid::BLOW{Loc{1, 0}, Loc{0, -1}, Loc{-1, 0}, Loc{0, 1}};

void addRow(string &row, int y) {
  for (int i(0); i < row.size(); ++i) {
    switch (row[i]) {
      case '.':  // empty
        break;
      case 'X':  // wall
        grid.walls.insert(Loc(i, y));
        break;
      case '0':  // box
      default:
        grid.boxes[Loc(i, y)] = row[i];
        break;
    }
  }
}

void removeBox(Loc &loc) {
  if (grid.boxes.count(loc)) {
    grid.boxes.erase(loc);
  }
}

int countBox(Loc &p, int blow) {
  int count = 0;
  for (auto d : grid.BLOW) {
    for (int i(1); i < blow + 1; ++i) {
      Loc n(p.x + i * d.x, p.y + i * d.y);
      if (grid.boxes.count(n)) {
        ++count;
      }
    }
  }
  return count;
}

bool placedBomb(Loc &p) {
  for (auto b : bombs) {
    if (b == p) {
      return true;
    }
  }
  return false;
}

void play(string &action, string &msg, Loc &p) {
  string message = " " + action + to_string(p.x) + string(" ") + to_string(p.y) + msg;
  cout << action << " " << p.x << " " << p.y << message << endl;
}

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

unordered_map<Loc, int> bfs(Loc &start) {
  queue<Loc> frontier;
  frontier.push(start);

  unordered_map<Loc, int> path;
  path[start] = 0;

  while (!frontier.empty()) {
    auto current = frontier.front();
    frontier.pop();

    for (auto next : grid.neighbors(current)) {
      if (!path.count(next)) {
        frontier.push(next);
        path[next] = 1 + path[current];
      }
    }
  }
  path.erase(start);
  return path;
}

inline double heuristic(Loc a, Loc b) { return abs(a.x - b.x) + abs(a.y - b.y); }

void a_star(Loc &start, Loc &goal, unordered_map<Loc, Loc> &came_from, unordered_map<Loc, double> &cost) {
  PriorityQueue<Loc, double> frontier;
  frontier.put(start, 0);

  came_from[start] = start;
  cost[start] = 0;

  while (!frontier.empty()) {
    auto current = frontier.get();

    if (current == goal) {
      break;
    }

    for (auto next : grid.neighbors(current)) {
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

double norm(const Loc &p1, const Loc &p2) { return sqrt(pow(abs(p1.x - p2.x), 2) + pow(abs(p1.y - p2.y), 2)); }

void computeBestMoves(map<double, Loc> &bestLoc) {
  // Get all possible moves
  cerr << "get all moves" << endl;
  unordered_map<Loc, int> playerPath = bfs(player);

  // Analyze all possible moves
  for (auto move : playerPath) {
    // if(++cnt > 60) break;
    Loc bombLoc(move.first);
    int nbBoxBlowed;
    // Get range of bomb explosion at this location and nbBoxBlowed
    unordered_set<Loc> blow = grid.getBlow(bombLoc, player.blow(), nbBoxBlowed);
    if (nbBoxBlowed) {  // valid candidate
      // Check no current bomb will explode when I get there
      // cerr<<" potential "<<bombLoc<<endl;
      bool safeBomb = true;
      for (auto bomb : bombs) {
        if (bomb.nbRound == move.second) {
          unordered_set<Loc> bblow = bombRanges[bomb];
          if (bblow.count(bombLoc)) safeBomb = false;
        }
      }
      if (safeBomb) {
        // cerr <<" safe for bombs"<<endl;
        // Get possible moves from this location
        unordered_map<Loc, int> potPath = bfs(bombLoc);
        bool safePos = false;
        // Check I can escape the potential explosion
        for (auto potMove : potPath) {
          if (!blow.count(potMove.first)) {
            safePos = true;
            break;
          }
        }
        if (safePos) {
          // cerr <<" safe for me"<<endl;
          // Save this bomb in bestlocations
          double dist = move.second;
          double load = nbBoxBlowed - dist / 6 + grid.items.count(move.first);
          bestLoc[load] = bombLoc;
          // cerr << "  save " << printLoc(bombLoc) << " load " << load << ": " << nbBoxBlowed<< " " << dist <<endl;
        }
      }
    }
  }
}

void computeBombRanges() {
  for (auto bomb : bombs) {
    // when not already analysed
    // if (!bombRanges.count(loc)) {
    // Insert itself
    int n;
    unordered_set<Loc> blow = grid.getBlow(bomb, bomb.blow(), n);
    bombRanges[bomb] = blow;
    for (auto otherBomb : bombs) {
      if (otherBomb != bomb) {
        unordered_set<Loc> blew = grid.getBlow(otherBomb, otherBomb.blow(), n);
        if (blew.count(bomb) || blow.count(otherBomb)) {
          // Add the 2 ranges
          bombRanges[bomb].insert(blew.begin(), blew.end());
          bombRanges[otherBomb] = bombRanges[bomb];
        }
      }
    }
    //}
  }
}

void itemResolutionAndMapUpdate(int height, int myId) {
  // bomb resolution
  for (auto bomb : bombs) {
    if (bomb.nbRound == 1) {
      grid.upItems(bomb, bomb.blow());
    }
  }
  // item resolution
  for (auto item : items) {
    Box b = item.second;
    if (b == player) {
      player.upgrade(b.type);
    }
  }
  vector<Loc> toRemove;
  for (auto mapItem : grid.items) {
    bool stillHere = false;
    for (auto item : items) {
      if (item.second.x == mapItem.first.x and item.second.y == mapItem.first.y) {
        stillHere = true;
        break;
      }
      if (!stillHere) {
        toRemove.push_back(mapItem.first);
      }
    }
  }
  for (auto loc : toRemove) {
    grid.items.erase(loc);
  }

  // Assess validity of the target
  if (grid.boxes.size() > 0) {
    if (countBox(target, player.blow()) == 0 || placedBomb(target)) {
      target.isValid = false;
    }
  } else {
    // no ennemy facing the bomb, invalidate it
  }

  // map update
  for (int i(0); i < height; ++i) {
    string row;
    getline(cin, row);
    addRow(row, i);
  }
  bombs.clear();
  items.clear();
  ennemies.clear();
  int entities;
  cin >> entities;
  cin.ignore();
  // entities acquistion
  for (int i(0); i < entities; ++i) {
    int type, owner, x, y, param1, param2;
    cin >> type >> owner >> x >> y >> param1 >> param2;
    cin.ignore();
    switch (type) {
      case 0:  // players
        if (owner == myId) {
          player.set(x, y, param1, param2);
        } else {
          Loc loc(x, y);
          ennemies.push_back(loc);
        }
        break;
      case 1:  // bombs
      {
        bombs.push_back(Bomb(x, y, param1, param2));
        Loc loc(x, y);
        if (loc != player || owner != myId) {
          grid.bombs.insert(loc);
        }
      } break;
      case 2:  // items
        items[i] = Box(x, y, param1);
        Loc it(x, y);
        removeBox(it);
        grid.items[it] = param1;
        break;
    }
  }
}

bool emptyPath(Loc &t, Bomb &b, Loc &p) {
  auto empty(true);
  grid.bombs.insert(t);
  unordered_map<Loc, int> path = bfs(p);
  cerr << "bfs for next loc " << path.size() << endl;
  for (auto m : path) {
    int n;
    if (!grid.getBlow(b, b.blow(), n).count(m.first)) {
      // at least one possible issue
      empty = false;
      break;
    }
  }
  grid.bombs.erase(t);
  return empty;
}

bool isNextSecure(Loc p) {
  auto secure(true);
  for (auto bomb : bombs) {
    cerr << bomb << endl;
    vector<Loc> next = grid.neighbors(p);
    next.push_back(p);
    auto nextHasEscape(false);
    for (auto n : next) {
      if (!bombRanges[bomb].count(n)) {
        nextHasEscape = true;
        break;
      }
    }
    if ((bomb.nbRound == 2 && bombRanges[bomb].count(p)) or (bomb.nbRound == 3 and !nextHasEscape)) {
      cerr << "    will blow here -" << p << endl;
      secure = false;
      break;
    }
    grid.bombs.insert(bomb);
    unordered_map<Loc, int> potPath = bfs(p);
    if (potPath.size() == 0) {
      secure = false;
      grid.bombs.erase(bomb);
      break;
    }
    grid.bombs.erase(bomb);
  }
  return secure;
}

Loc findSecureNextLoc(string &msg) {
  // Check path to target is safe
  cerr << " check next loc is safe" << endl;
  unordered_map<Loc, Loc> potPath;
  unordered_map<Loc, double> cost;
  a_star(target, player, potPath, cost);

  Loc nextLoc(player.x, player.y);

  bool isSafe(true);
  Loc loc;
  for (auto bomb : bombs) {
    auto empty = emptyPath(player, bomb, nextLoc);
    if (!bombRanges[bomb].count(potPath[player]) && bomb != player && !empty) {
      continue;
    }
    cerr << bomb << endl;
    int count = bomb.nbRound;
    loc.x = -1;
    loc.y = -1;
    nextLoc.x = player.x;
    nextLoc.y = player.y;
    while (count > 1) {
      loc = nextLoc;
      nextLoc = potPath[loc];
      count--;
    }
    cerr << "    loc when bombing " << nextLoc << " empty " << empty << endl;
    if (bombRanges[bomb].count(nextLoc) or empty) {
      cerr << "      i will blow up" << endl;
      isSafe = false;
    }
  }

  vector<Loc> next = grid.neighbors(player);
  if (!isSafe) {
    // remove this position from possible nexts and add current position
    next.erase(find(next.begin(), next.end(), nextLoc));
    next.push_back(player);
    // stay tight
    for (auto l : next) {
      cerr << " potential loc " << l << endl;
      if (isNextSecure(l)) {
        msg = " escape blow";
        nextLoc = l;
        break;
      }
    }
    target.isValid = false;
  } else {
    msg = " to target";
    if (potPath[player].x != -1) {
      nextLoc = potPath[player];
    } else {
      // cannot move, reset the targets
      target.isValid = false;
    }
  }
  return nextLoc;
}

/*
 * MAIN CODE
 */

int main() {
  int width;
  int height;
  int myId;
  cin >> width >> height >> myId;
  cin.ignore();

  grid.set(width, height);

  while (1) {
    itemResolutionAndMapUpdate(height, myId);

    cerr << player << endl;
    cerr << " target " << target << "valid " << target.isValid << endl;

    // Action resolution
    string action("MOVE");
    string msg("");
    // We have a valid target
    if (target.isValid && target == player && player.nbBomb > 0) {
      // drop the bomb
      msg = " drop";
      theNextLoc.save(target);
      action = "BOMB";
      player.drop();
      bombs.push_back(Bomb(player.x, player.y, 8, player.range));
      target.isValid = false;
      // grid.walls.insert(target);
    }

    // Compute bomb ranges
    bombRanges.clear();
    computeBombRanges();

    // Target deternimation
    if (!target.isValid) {
      if (grid.boxes.size() > 0) {
        map<double, Loc> bestLoc;
        computeBestMoves(bestLoc);

        for (auto m : bestLoc) {
          cerr << m.second << ": " << m.first << endl;
        }
        // Valid target found
        if (bestLoc.size()) {
          cerr << "potential targets" << endl;
          for (map<double, Loc>::reverse_iterator it = bestLoc.rbegin(); it != bestLoc.rend(); ++it) {
            target = it->second;
            cerr << "> " << target << it->first << endl;
            bool secure = true;
            for (auto bomb : bombs) {
              cerr << bomb << endl;
              if (bombRanges[bomb].count(target) and ((target.x == bomb.x) || (target.y == bomb.y)) &&
                  (norm(target, player) + 2 >= bomb.blow())) {
                cerr << "is too close, not secure " << norm(target, player) << " " << bomb.blow() << endl;
                secure = false;
                break;
              }
            }
            if (secure) {
              break;
            }
          }
        }
      } else if (grid.boxes.size() == 0) {
        cerr << "no more box, hunt" << endl;
        int close(50);
        Loc en(0, 0);
        for (auto e : ennemies) {
          if (norm(player, e) < close) {
            close = static_cast<int>(norm(player, e));
            en = e;
          }
        }
        for (auto e : grid.neighbors(en)) {
          bool isBomb(false);
          for (auto b : bombs) {
            if (b == e) {
              isBomb = true;
              break;
            }
          }
          if (!isBomb) {
            target = e;
            break;
          }
        }
      }
    }

    // action quand plus de box
    // check current is safe
    // pas se faire bloquer (nb sortie)
    theNextLoc = findSecureNextLoc(msg);

    play(action, msg, theNextLoc);
  }
}
