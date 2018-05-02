#include <algorithm>
#include <array>
#include <cmath>
#include <iostream>
#include <map>
#include <string>
#include <vector>

using namespace std;

namespace {
double EPSILON = 0.00001;
}

struct Point {
  int x, y;
  inline Point() = default;
  inline Point(int x, int y) : x(x), y(y) {}
  inline Point(Point const &) = default;
  inline Point(Point &&) = default;

  int dist(const Point &p) { return (this->x - p.x) * (this->x - p.x) + (this->y - p.y) * (this->y - p.y); }

  double dist2(const Point &p) { return sqrt(dist(p)); }

  void norm() {
    x = x / sqrt(x * x + y * y);
    y = y / sqrt(x * x + y * y);
  }

  int scalar(const Point &p) { return this->x * p.x + this->y * p.y; }

  void move(int _x, int _y) {
    x = _x;
    y = _y;
  }
  void moveTo(Point p, double distance) {
    double d = dist(p);
    // division by 0
    if (d < EPSILON) {
      return;
    }

    double dx = p.x - x;
    double dy = p.y - y;
    double coef = distance / d;

    this->x += dx * coef;
    this->y += dy * coef;
  }

  bool isInRange(Point p, double range) { return p != *this && dist2(p) <= range; }

  inline Point &operator=(Point const &) = default;
  inline Point &operator=(Point &&) = default;

  inline bool operator==(const Point &c) const { return this->x == c.x && this->y == c.y; }
  inline bool operator!=(const Point &c) const { return !((*this) == c); }
  inline bool operator<(const Point &c) const { return this->x + 8 * this->y < c.x + 8 * c.y; }
  inline Point operator+(const Point &c) const { return Point(this->x + c.x, this->y + c.y); }
  inline Point operator-(const Point &c) const { return Point(this->x - c.x, this->y - c.y); }
  inline Point operator*(const int &c) const { return Point(this->x * c, this->y * c); }
  inline Point operator/(const int &c) const { return Point(this->x / c, this->y / c); }
  virtual void serialize(ostream &os) const { os << " [" << x << "," << y << "]"; }
};
ostream &operator<<(ostream &os, const Point &c) {
  c.serialize(os);
  return os;
}

struct Player {
  int score, rage;
  inline Player() = default;
  inline Player(int score, int rage) : score(score), rage(rage) {}
  inline Player(Player const &) = default;
  inline Player(Player &&) = default;
  inline Player &operator=(Player const &) = default;
  inline Player &operator=(Player &&) = default;
  void serialize(ostream &os) const { os << "Player " /* << id << */ " [" << score << "," << rage << "]"; }
};
ostream &operator<<(ostream &os, const Player &c) {
  c.serialize(os);
  return os;
}

namespace {
typedef enum { REAPER, DESTROYER, DOOF, TANKER, WRECK, TAR, OIL } UnitType_t;
array<string, 7> UnitName = {"Reaper", "Destroyer", "Doof", "Tanker", "Wreck", "Tar", "Oil"};
typedef enum { WAIT, THROTTLE } Action_t;
array<string, 5> ActionName = {"WAIT", "THROTTLE"};
Point center(0, 0);
int map_radius(3000);
int gUnitId(0);
}  // namespace

struct Unit : public Point {
  int id, t, p, r, vx, vy, w, c;
  double m;

  inline Unit() = default;
  inline Unit(int id, int t, int p, double m, int r, int x, int y, int vx, int vy, int w, int c)
      : Point(x, y), id(id), t(t), p(p), r(r), vx(vx), vy(vy), w(w), c(c), m(m) {}
  inline Unit(Unit const &) = default;
  inline Unit(Unit &&) = default;

  void move(double t) {
    x += vx * t;
    y += vy * t;
  }

  void thrust(Point p, int power) {
    double distance = dist(p);

    // Avoid a division by zero
    if (abs(distance) <= EPSILON) {
      return;
    }

    double coef = (static_cast<double>(power) / m) / distance;
    vx += (p.x - this->x) * coef;
    vy += (p.y - this->y) * coef;
  }

  bool harvest(vector<Unit> &reapers, array<Player, 3> &players) {
    for (auto rep : reapers) {
      if (isInRange(rep, r) /*and !inOil(rep)*/) {
        players[rep.p].score += 1;
        w -= 1;
      }
    }
    return w > 0;
  }
  /*Unit die() {
    // Don't spawn a wreck if our center is outside of the map
    if (dist2(center) >= map_radius) {
        return center;
    }

    return Unit(gUnitId, WRECK, -1, -1, r, round(x), round(y), 0, 0, w, -1);
}*/

  inline Unit &operator=(Unit const &) = default;
  inline Unit &operator=(Unit &&) = default;

  virtual void serialize(ostream &os) const {
    Point::serialize(os);
    os << UnitName[t] << id << " p:" << p << ", m:" << m << ", r:" << r << ", vx:" << vx << ", vy:" << vy << ", w:" << w
       << ", c:" << c;
  }
};
ostream &operator<<(ostream &os, const Unit &c) {
  c.serialize(os);
  return os;
}

struct Board {
  int nbUnits;
  bool firstTurn = true;
  array<int, 3> ppos;
  array<Player, 3> players;
  array<Unit, 3> reapers;
  array<Unit, 3> destroyers;
  array<Unit, 3> doofs;
  vector<Unit> tankers;
  vector<Unit> wrecks;
  vector<Unit> skills;
  map<double, int> wdist;
  map<int, int> wdist1, wdist2, tdist, ddist, rdist, rtdist;

  int nbUnitAround(const Unit &wreck, int &with) {
    int nb(0);
    double r = 1.5;
    for (auto u : reapers) {
      if (u.isInRange(wreck, r * wreck.r) and u.id != ppos[0]) ++nb;
    }
    for (auto u : destroyers) {
      if (u.isInRange(wreck, r * wreck.r)) ++nb;
    }
    for (auto u : doofs) {
      if (u.isInRange(wreck, r * wreck.r)) ++nb;
    }
    for (auto u : tankers) {
      if (u.isInRange(wreck, r * wreck.r)) ++nb;
    }
    for (auto w : wrecks) {
      if (w.isInRange(wreck, 1500)) {
        ++with;
      }
    }
    return nb;
  }

  bool within(const Unit &u, const double &a, const double &b, const int &c) {
    return u.y < a * u.x + b + c and u.y > a * u.x + b - c;
  }

  int nbUnitWithin(const Unit &reaper, const Unit &wreck) {
    int nb(0);
    double a(static_cast<double>(reaper.y - wreck.y) / (reaper.x - wreck.x));
    double b = reaper.y - a * reaper.x;
    int c = 300;
    for (auto u : reapers) {
      if (within(u, a, b, c)) ++nb;
    }
    for (auto u : destroyers) {
      if (within(u, a, b, c)) ++nb;
    }
    for (auto u : doofs) {
      if (within(u, a, b, c)) ++nb;
    }
    for (auto u : tankers) {
      if (within(u, a, b, c)) ++nb;
    }
    return nb;
  }

  void mapsUpdate() {
    int wid(0);
    for (auto w : wrecks) {
      int with(0);
      int nbUnit = nbUnitAround(w, with);
      wdist1[reapers[ppos[1]].dist(w)] = wid;
      wdist2[reapers[ppos[2]].dist(w)] = wid;
      int sk(1);
      for (auto s : skills) {
        if (s.t == OIL and s.dist2(w) <= w.r) {
          sk = 100;
          break;
        }
      }
      int eval = sk * ((reapers[ppos[0]].dist2(w)) / (w.w + (with))) * (nbUnit + 1);
      cerr << nbUnit << " unit " << with << " wrecks around wreck " << w.id << " " << sk << " " << eval << endl;
      wdist[eval] = wid++;
    }
    int tid(0);
    for (auto t : tankers) {
      tdist[destroyers[ppos[0]].dist(t) / (1)] = tid++;
    }
    int did(0);
    for (auto d : destroyers) {
      ddist[reapers[ppos[0]].dist(d)] = did++;
    }
    int rid(0);
    for (auto r : reapers) {
      if (r.p == ppos[0]) {
        ++rid;
        continue;
      }
      rdist[doofs[ppos[0]].dist(r)] = rid++;
    }
    rid = 0;
    for (auto r : reapers) {
      if (r.p == ppos[0]) {
        ++rid;
        continue;
      }
      rtdist[destroyers[ppos[0]].dist(r)] = rid++;
    }
  }

  void turnInit() {
    tankers.clear();
    wrecks.clear();
    skills.clear();
    wdist.clear();
    wdist1.clear();
    wdist2.clear();
    tdist.clear();
    ddist.clear();
    rdist.clear();
    rtdist.clear();
    for (int i(0); i < 3; ++i) {
      cin >> players[i].score;
      cin.ignore();
    }
    for (int i(0); i < 3; ++i) {
      cin >> players[i].rage;
      cin.ignore();
    }
    cin >> nbUnits;
    cin.ignore();
    int nbr(0), nbd(0), nbf(0);
    for (int i(0); i < nbUnits; ++i) {
      double mass;
      int unitId, unitType, player, radius, x, y, vx, vy, w, c;
      cin >> unitId >> unitType >> player >> mass >> radius >> x >> y >> vx >> vy >> w >> c;
      cin.ignore();
      switch (unitType) {
        case REAPER:
          reapers[nbr++] = (Unit(unitId, unitType, player, mass, radius, x, y, vx, vy, w, c));
          break;
        case DESTROYER:
          destroyers[nbd++] = (Unit(unitId, unitType, player, mass, radius, x, y, vx, vy, w, c));
          break;
        case DOOF:
          doofs[nbf++] = (Unit(unitId, unitType, player, mass, radius, x, y, vx, vy, w, c));
          break;
        case TANKER:
          tankers.push_back(Unit(unitId, unitType, player, mass, radius, x, y, vx, vy, w, c));
          break;
        case WRECK:
          wrecks.push_back(Unit(unitId, unitType, player, mass, radius, x, y, vx, vy, w, c));
          break;
        default:
          skills.push_back(Unit(unitId, unitType, player, mass, radius, x, y, vx, vy, w, c));
      }
      ++gUnitId;
    }
    if (firstTurn) {
      int id(0);
      for (auto r : reapers) {
        ppos[r.p] = id++;
      }
      firstTurn = false;
    }
    mapsUpdate();
  }

  void printAll() {
    for (auto p : players) {
      cerr << p << endl;
    }
    for (auto r : reapers) {
      cerr << r << endl;
    }
    for (auto r : destroyers) {
      cerr << r << endl;
    }
    for (auto r : doofs) {
      cerr << r << endl;
    }
    for (auto w : wrecks) {
      cerr << w << endl;
    }
    for (auto w : skills) {
      cerr << w << endl;
    }
  }

  void lineAction() {
    // vector<vector<int> > lines;
    for (auto w1 : wrecks) {
      for (auto w2 : wrecks) {
        if (w1 != w2) {
          Point w((w1 + w2) / 2);
          Point v1(w1 - reapers[ppos[0]]);
          Point v2(w2 - reapers[ppos[0]]);
          Point v(w - reapers[ppos[0]]);
          // v1.norm(); v2.norm(); v.norm();
          cerr << "scalar with w" << w1.id << " " << v.scalar(v1) << " and w" << w2.id << " " << v.scalar(v2) << endl;
        }
      }
    }
  }

  bool wreckAround(Unit &r, Unit &wr) {
    for (auto w : wrecks) {
      if (r.isInRange(w, 1.5 * w.r) and w.w > 1) {
        wr = w;
        return true;
      }
    }
    return false;
  }

  void playActions() {
    // lineAction();

    int tid = rtdist.begin()->second;
    tid = ppos[tid];

    // REAPER
    if (wdist.size() > 0) {
      int rx(0), ry(0);
      int wid = wdist.begin()->second;
      double d = reapers[ppos[0]].dist2(wrecks[wid]);
      /*if (d >= 300) {
        int ra = rand() % 2;
        if (ra == 0) {rx = 50;}
        else {ry = 50;}
      }*/
      int xt = wrecks[wid].x;
      int yt = wrecks[wid].y;
      double coef = d / 600;
      int deltax = static_cast<int>(coef * (xt - reapers[ppos[0]].x - reapers[ppos[0]].vx));
      int deltay = static_cast<int>(coef * (yt - reapers[ppos[0]].y - reapers[ppos[0]].vy));
      cout << reapers[ppos[0]].x + deltax + rx << " " << reapers[ppos[0]].y + deltay + ry << " " << 300
           << " eau " + to_string(wrecks[wid].id) << endl;

      int n(nbUnitWithin(wrecks[wid], reapers[ppos[0]]));
      cerr << n << "units within" << endl;
    } else {
      double d = reapers[ppos[0]].dist2(center);
      int xt = center.x;
      int yt = center.y;
      double coef = d / 600;
      int deltax = static_cast<int>(coef * (xt - reapers[ppos[0]].x - reapers[ppos[0]].vx));
      int deltay = static_cast<int>(coef * (yt - reapers[ppos[0]].y - reapers[ppos[0]].vy));
      // cout << reapers[ppos[0]].x + deltax << " " << reapers[ppos[0]].y + deltay << " " << 300 << " replace " << endl;

      if (tdist.size() > 0) {
        int tid = tdist.begin()->second;
        cout << tankers[tid].x - reapers[ppos[0]].vx << " " << tankers[tid].y - reapers[ppos[0]].vy << " " << 300
             << " piste " + to_string(tid) << endl;
      } else {
        cout << "WAIT"
             << " wait" << endl;
      }
    }
    // DESTROYER
    cerr << destroyers[ppos[0]].dist2(reapers[tid]) << " dist from " << tid << endl;
    if (players[0].rage >= 60 and destroyers[ppos[0]].isInRange(reapers[tid], 2000)) {
      cout << "SKILL " << static_cast<int>(reapers[tid].x + reapers[tid].vx / .8) << " "
           << static_cast<int>(reapers[tid].y + reapers[tid].vy / .8) << " SHOOT " + to_string(tid) << endl;
    } else {
      if (tdist.size() > 0) {
        int tid = tdist.begin()->second;
        cout << tankers[tid].x << " " << tankers[tid].y << " " << 300 << " casse " + to_string(tid) << endl;
      } else {
        cout << "WAIT"
             << " wait" << endl;
      }
    }
    // DOOF
    int rid = players[ppos[1]].score > players[ppos[2]].score ? ppos[1] : ppos[2];
    rid = ppos[1];
    Unit wr;
    if (players[0].rage >= 30 and reapers[rid].isInRange(doofs[ppos[0]], 2000) and wreckAround(reapers[rid], wr)) {
      cout << "SKILL " << wr.x << "  " << wr.y << " oil this" << endl;
    } else {
      cout << static_cast<int>(reapers[rid].x + reapers[rid].vx / .8) << " "
           << static_cast<int>(reapers[rid].y + reapers[rid].vy / .8) << " " << 300 << " traque " + to_string(rid)
           << endl;
    }
  }
  void playTurn() {}
};

int main() {
  Board theBoard;
  while (1) {
    theBoard.turnInit();
    theBoard.printAll();
    theBoard.playActions();
  }
}
