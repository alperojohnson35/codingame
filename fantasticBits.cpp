#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <string>

using namespace std;

#define WIDTH (16001)
#define HEIGTH (7501)
#define POLE_RADIUS (300)
#define POLE_DIST (4000)


class Location
{
  int _x,_y;
public:
  Location() : _x(0), _y(0) {}
  Location(int x, int y) : _x(x), _y(y) {}
  string print() const {
    return "[" + to_string(_x) + "," + to_string(_y) + "]";
  }

  void update(int x, int y) {
   _x = x; _y = y;
  }

  Location operator+(const Location& l) {
    return Location(l._x + this->_x, l._y + this->_y);
  }
  Location operator-(const Location& l) {
    return Location(l._x - this->_x, l._y - this->_y);
  }
  Location operator*(const Location& l) {
    return Location(l._x * this->_x, l._y * this->_y);
  }
  int dist(const Location& l) const {
    return pow((this->_x - l._x), 2) + pow((this->_y - l._y), 2);
  }
  double norm() const {
    return sqrt(pow((this->_x),2) + pow((this->_y),2));
  }
};
ostream& operator<<(ostream& out, const Location& l){
  return out << l.print();
}


class Snaffle : public Location
{
    int _r;
    int _vx, _vy;
public:
    Snaffle() : Location() ,_vx(0), _vy(0), _r(150) {}
    Snaffle(int x, int y, int vx, int vy, int r = 150) : Location(x, y) ,_vx(vx), _vy(vy), _r(r) {}
    string print() const {
      return Location::print() + "-(" + to_string(_vx) + "," + to_string(_vy) + ")";
    }

    void update(int x, int y, int vx, int vy) {
      Location::update(x, y);
      _vx = vx; _vy = vy;
    }
};
ostream& operator<<(ostream& out, const Snaffle& s){
  return out << s.print();
}

class Wizard : public Snaffle
{
  bool _snaffle;
public:
    Wizard() : Snaffle(0, 0, 0, 0, 400), _snaffle(0) {}
    Wizard(int x, int y, int vx, int vy, int s) : Snaffle(x, y, vx, vy, 400), _snaffle(s) {}
    string print() const {
      return Snaffle::print() + " " + to_string(_snaffle);
    }
    void update(int x, int y, int vx, int vy, int s) {
      Snaffle::update(x, y, vx, vy);
      _snaffle = s;
    }
};
ostream& operator<<(ostream& out, const Wizard& w){
  return out << w.print();
}

/**
 * Grab Snaffles and try to throw them through the opponent's goal!
 * Move towards a Snaffle and use your team id to determine where you need to throw it.
 **/
int main()
{
    int myTeamId; // if 0 you need to score on the right of the map, if 1 you need to score on the left
    cin >> myTeamId; cin.ignore();
    vector<Location> Goals;
    if (myTeamId == 0) {
      Goals.push_back(Location(0, 1750));
      Goals.push_back(Location(0, 5750));
    } else {
      Goals.push_back(Location(16000, 1750));
      Goals.push_back(Location(16000, 5750));
    }

    vector<Wizard> Me = {Wizard(), Wizard()};
    vector<Wizard> Him = {Wizard(), Wizard()};
    vector<Snaffle> Balls = {Wizard(), Wizard()};


    // game loop
    while (1) {
        int entities; // number of entities still in game
        cin >> entities; cin.ignore();


        for (int i = 0; i < entities; ++i) {
            int entityId; // entity identifier
            string entityType; // "WIZARD", "OPPONENT_WIZARD" or "SNAFFLE" (or "BLUDGER" after first league)
            int x, y, vx, vy, state; // position
            cin >> entityId >> entityType >> x >> y >> vx >> vy >> state; cin.ignore();
            if (entityType == "WIZARD") {
                Wizard w(x, y, vx, vy, state);
                Me.push_back(w);
            } else if (entityType == "OPPONENT_WIZARD") {
                Wizard w(x, y, vx, vy, state);
            } else if (entityType == "SNAFFLE") {
                if (i < 0) {
                  Snaffle w(x, y, vx, vy);
                } else {

                }
            } else {
               // nothing now
            }
        }
        for (int i(0); i < 2; ++i) {

            // Write an action using cout. DON'T FORGET THE "<< endl"
            // To debug: cerr << "Debug messages..." << endl;

            cerr << Me[i] << endl;
            // Edit this line to indicate the action for each wizard (0 ≤ thrust ≤ 150, 0 ≤ power ≤ 500)
            // i.e.: "MOVE x y thrust" or "THROW x y power"
            cout << "MOVE 8000 3750 100" << endl;
        }
    }
}
