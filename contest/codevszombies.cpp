#pragma GCC target("avx") // small optimization
#pragma GCC optimize("O3") // +44k killer optimization with "inline __attribute__ (( always_inline, visibility("protected") ))" and "inline"
#pragma GCC optimize("omit-frame-pointer") // good optimization
#pragma GCC optimize("unsafe-math-optimizations") // not really useful it seems
#pragma GCC optimize("unroll-all-loops") // good optimization
#pragma GCC optimize("inline") // killer optimization with O3


#include <algorithm>
#include <array>
#include <cmath>
#include <iostream>
#include <map>
#include <set>
#include <cmath>
#include <string>
#include <vector>
#include <chrono>

using namespace std;

#define EPSILON 0.00001
#define XMAX (16000)
#define YMAX (9000)
const signed short TIME_MAX = 96;

namespace {
const double PI = 3.141592653589793238463;
}

struct Timer {
    chrono::time_point<chrono::system_clock> end;
    signed char not_always = 0;

    inline Timer(Timer const&) = default;
    inline Timer(Timer&&) = default;
    inline Timer& operator=(Timer const&) = default;
    inline Timer& operator=(Timer&&) = default;

    inline Timer(){
      this->end=chrono::system_clock::now()+chrono::milliseconds(TIME_MAX);
    }
    inline bool timesUp(){
        return std::chrono::system_clock::now() > this->end;
    }
};

struct Point {
  int x, y, range, speed;
  inline Point() = default;
  inline Point(int x, int y) : x(x), y(y), range(0), speed(0) {}
  inline Point(Point const &) = default;
  inline Point(Point &&)      = default;

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
  Point move(const Point &p) {
    if (p == *this) {
        return *this;
    } else {
    return Point(x-(x - p.x)*static_cast<double>(speed)/this->dist2(p), y-(y - p.y)*static_cast<double>(speed)/this->dist2(p));
    }
  }
  void moveTo(Point p, double distance) {
    double d = dist(p);
        // division by 0
    if (d < EPSILON) {
      return;
    }

    double dx   = p.x - x;
    double dy   = p.y - y;
    double coef = distance / d;

    this->x += dx * coef;
    this->y += dy * coef;
  }

  bool isInRange(Point p, double range) { return p != *this && dist2(p) <= range; }
  void copy(const Point& p) {
      x = p.x;
      y = p.y;
  }

  inline Point &operator=(Point const &) = default;
  inline Point &operator=(Point &&) = default;

  inline bool operator==(const Point &c) const { return this->x == c.x and this->y == c.y; }
  inline bool operator!=(const Point &c) const { return !((*this) == c); }
  inline bool operator<(const Point &c) const { return this->x + 8 * this->y < c.x + 8 * c.y; }
  inline Point operator+(const Point &c) const { return Point(this->x + c.x, this->y + c.y); }
  inline Point operator-(const Point &c) const { return Point(this->x - c.x, this->y - c.y); }
  inline Point operator*(const int &c) const { return Point(this->x * c, this->y * c); }
  inline Point operator/(const int &c) const { return Point(this->x / c, this->y / c); }
  virtual void serialize(ostream &os) const { os << " [" << x << "," << y << "] "; }
};
ostream &operator<<(ostream &os, const Point &c) {
  c.serialize(os);
  return os;
}

struct Unit : public Point {
  int id, nx, ny, s;

  inline Unit() = default;
  inline Unit(int s) : Point(0, 0), id(0), s(s) {}
  inline Unit(Unit const &) = default;
  inline Unit(Unit &&)      = default;

  void move() {
    int mx = nx, my = ny;
    nx  = 2 * nx - x;
    ny  = 2 * ny - y;
    x = mx;
    y = my;
  }

  void next(Point t, double r) {
    double d = dist2(t);
    // Avoid a division by zero
    if (abs(d) <= EPSILON) {
      return;
    }
    double coef = (d > s) ? static_cast<double>(s) / d : d / s;
    nx = (t.x - x) * coef * r + x;
    ny = (t.y - y) * coef * r + y;
  }

  bool inBounds(Point p) {
    return p.x > 0 and p.x < XMAX and p.y > 0 and p.y < YMAX;
  }
  vector<Point> neighbours(int n) {
    vector<Point> out;
    for (int i = 0; i < n; ++i) {
      Point p(x + s * cos((2 * i * PI)/n), y + s * sin((2 * i * PI)/n));
      if (inBounds(p)) {out.push_back(p);}
    }
    return out;
  }

  inline Unit &operator=(Unit const &) = default;
  inline Unit &operator=(Unit &&) = default;

  virtual void serialize(ostream &os) const {
    Point::serialize(os);
    os << id <<  ", nx:" << nx << ", ny:" << ny << ", s:" << s;
  }
};
ostream &operator<<(ostream &os, const Unit &c) {
  c.serialize(os);
  return os;
}

struct Square{
  int xm, xM, ym, yM;
  bool isSet;

  inline Square() : isSet(false) {}
  inline Square(Square const &) = default;
  inline Square(Square &&)      = default;

  set<Point> sample(int d) {
    set<Point> out;
    int x = (xM - xm) / d;
    int y = (yM - ym) / d;
    for(int i(0); i < d; ++i) {
      for(int j(0); j < d; ++j) {
        out.insert(Point(xm + i * x, ym + j * y));
      }
    }
    return out;
  }
  bool within(Point p) {
    return p.x > xm and p.x < xM and p.y > ym and p.y < yM;
  }
  Point random(Point p) {
    double r = static_cast<double>(rand() % 10) / 10;
    return p;
  }

  void print() {
    cerr << "x: " << xm << "," << xM << " y: " << ym << "," << yM << endl;
  }
};

struct Board {
  Unit Ash;
  vector<Unit> humans;
  vector<Unit> H;
  map<int, Unit> zombies;
  map<int, Unit> Z;
  Square square;
  Unit bestPos;
  int ns;

  Board() : ns(0) {}

  void setSquare() {
    if (!square.isSet) {
      cerr << "setting square" << endl;
      square.xm = XMAX;
      square.xM = 0;
      square.ym = YMAX;
      square.yM = 0;
      humans.push_back(Ash);
      for (auto h : humans) {
        if (h.x < square.xm) {square.xm = h.x;}
        if (h.x > square.xM) {square.xM = h.x;}
        if (h.y < square.ym) {square.ym = h.y;}
        if (h.y > square.yM) {square.yM = h.y;}
      }
      humans.pop_back();
      square.isSet = true;
    }
  }

  void turnInit() {
    humans.clear();
    zombies.clear();
    cin >> Ash.x >> Ash.y; cin.ignore();
    int nbHuman(0);
    cin >> nbHuman; cin.ignore();
    for (int i = 0; i < nbHuman; i++) {
      Unit human(0);
      cin >> human.id >> human.x >> human.y; cin.ignore();
      humans.push_back(human);
      H.push_back(human);
    }
    int nbZombies;
    cin >> nbZombies; cin.ignore();
    for (int i = 0; i < nbZombies; i++) {
      Unit z(400);
      cin >> z.id >> z.x >> z.y >> z.nx >> z.ny; cin.ignore();
      zombies[Ash.dist2(z)] = z;
      Z[Ash.dist2(z)] = z;
    }
    if (ns == 0) {
      auto zt = zombies.begin();
      ns = Ash.dist2(zt->second) / Ash.s;
      setSquare();
      if (ns == 0) ++ns;
    }
  }

void moveZombies() {
    for (auto& z : zombies) {
        z.second.move();
    }
}

void copyZombies(bool copy) {
    if (copy) {
        zCopy.clear();
        zCopy = zombies;
    } else {
        zombies.clear();
        zombies = zCopy;
    }
}

void path() {

  int n(0);
  copyZombies(true);
  while (n < ns) {
    auto p(Ash);
    moveZombies();

    auto nx = square.random(p);
    p.next(nx, 1);
    //score += computeScore(p);
  }
  copyZombies(false);
}

  void simu(){
    vector<Point> neighbour = Ash.neighbours(4);
    double r = static_cast<double>(rand() % 10) / 10;
    for(auto n : neighbour) {
      if (square.within(n)) {
        Ash.next(n, 1);
      }
    }
    // no simu
    auto zt = zombies.begin();
    Ash.next(zt->second, 1.0);
    bestPos = zt->second;
  }

  void playAction(){
    auto zt = zombies.begin();
    cerr << "ns" <<ns << endl;
    cerr << "best" <<bestPos << endl;
    cout << bestPos.x << " " << bestPos.y << endl;
  }
};

int main() {
  Board theBoard;
  theBoard.Ash.s = 1000;
  while (1) {
    Timer theClock;
    theBoard.turnInit();
    int n(0);
    while (!theClock.timesUp()) {
      theBoard.simu();
      ++n;
    }
    cerr << n << " simus" <<endl;
    theBoard.playAction();
  }
}
