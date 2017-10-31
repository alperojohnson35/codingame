#pragma GCC optimize("O3")

#include <algorithm>
#include <iostream>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

using namespace std;

struct Node {
  int i;
  int x;
  int y;
  int p;
  map<int, int> edges;

  Node(int i_, int x_, int y_, int p_) : i(i_), x(x_), y(y_), p(p_) {}
  inline bool operator==(const Node &a) const { return a.x == (*this).x && a.y == (*this).y; }
  friend inline ostream &operator<<(ostream &os, const Node &n);
  inline int nonLinked() {
    int ret(0);
    for (auto e : edges) {
      if (e.second == 0) ret++;
    }
    return ret;
  }
  inline bool linked(int e) { return edges[e] != 0; }
  inline int             nbLinks() const {
    int ret(0);
    for (auto e : edges) {
      ret += e.second;
    }
    return ret;
  }
  inline bool isFull() const { return p == nbLinks(); }
  inline int  rest() const { return p - nbLinks(); }
};

inline ostream &operator<<(ostream &os, const Node &n) {
  string o;
  for (auto e : n.edges) {
    o += to_string(e.first) + ",";
  }
  os << "Node " << n.i << " [" << (n).x << "," << (n).y << "] p=" << (n).p << ", r=" << (n).rest() << " edges"
     << n.edges.size() << " " << o << " links " << n.nbLinks();
  return os;
}

static vector<Node> theNodes;
static vector<Node> theCopy;

struct Out {
  int x1;
  int y1;
  int x2;
  int y2;
  int l;
  Out(int x, int y, int z, int t, int l) : x1(x), y1(y), x2(z), y2(t), l(l) {}

  inline void print() const {
    ostringstream os;
    os << x1 << " " << y1 << " " << x2 << " " << y2 << " " << l;
    string s = os.str();
    cout << s << endl;
    cerr << s << endl;
  }
};

static vector<Out> result;

struct Graph {
  set<pair<int, int> > walls;

  Graph() {}
  inline void insertWall(int x1, int y1, int x2, int y2) {
    if (x1 == x2) {
      int y(min(y1, y2)), Y(max(y1, y2));
      for (int i(y + 1); i < Y; ++i) walls.insert(make_pair(x1, i));
    } else {
      int x(min(x1, x2)), X(max(x1, x2));
      for (int i(x + 1); i < X; ++i) walls.insert(make_pair(i, y1));
    }
  }
};

static Graph g;

void upEdges(Node &n, int curr, int &prev) {
  if (prev != -1) {
    n.edges[prev]              = 0;
    theNodes[prev].edges[curr] = 0;
  }
  prev = curr;
}

int fullLinks(Node &n) {
  int r(0);
  for (auto e : n.edges) {
    if (e.second == 2 || theNodes[e.first].isFull()) r++;
  }
  return r;
}

void createLink(Node &v, int id, int nb) {
  if (nb > 0) {
    Out o(v.x, v.y, theNodes[id].x, theNodes[id].y, nb);
    // cerr << "  Assign " << nb << " links from " << v.i << " to " << id << endl;
    v.edges[id] += nb;
    theNodes[id].edges[v.i] += nb;
    g.insertWall(v.x, v.y, theNodes[id].x, theNodes[id].y);
    result.push_back(o);
  }
}

bool crossWall(Node &n1, Node &n2) {
  if (n1.edges[n2.i] == 0) {
    if (n1.x == n2.x) {
      int y(min(n1.y, n2.y)), Y(max(n1.y, n2.y));
      for (int i(y + 1); i < Y; ++i) {
        if (g.walls.count(make_pair(n1.x, i))) {
          return true;
        }
      }
    } else {
      int x(min(n1.x, n2.x)), X(max(n1.x, n2.x));
      for (int i(x + 1); i < X; ++i) {
        if (g.walls.count(make_pair(i, n1.y))) {
          return true;
        }
      }
    }
  }
  return false;
}

int validEdges(Node &n) {
  int v(0);
  for (auto e : n.edges) {
    if (!crossWall(n, theNodes[e.first])) {
      ++v;
    }
  }
  return v;
}

void removeWall(Node &n1, Node &n2) {
  if (n1.edges[n2.i] == 0) {
    if (n1.x == n2.x) {
      int y(min(n1.y, n2.y)), Y(max(n1.y, n2.y));
      for (int i(y + 1); i < Y; ++i) {
        g.walls.erase(make_pair(n1.x, i));
      }
    } else {
      int x(min(n1.x, n2.x)), X(max(n1.x, n2.x));
      for (int i(x + 1); i < X; ++i) {
        g.walls.erase(make_pair(i, n1.y));
      }
    }
  }
}

bool prune(void) {
  bool cut(false);
  for (auto &v : theNodes) {
    int rest(v.rest());
    int freeLinks(v.edges.size() - fullLinks(v));
    // cerr << "node " << " rest " << rest << " free " << freeLinks << " full " << v.isFull() << endl;
    if (rest > 0) {
      if (rest == 2 * freeLinks) {
        // cerr << "   pruning " << v << endl;
        for (auto e : v.edges) {
          if (!v.linked(e.first) && !theNodes[e.first].isFull()) {
            cut = true;
            // cerr << "     links " << theNodes[e.first] << endl;
            createLink(v, e.first, 2 - e.second);
          }
        }
      } else if (v.p == 2 * v.edges.size()) {
        for (auto e : v.edges) {
          createLink(v, e.first, 2 - e.second);
        }
      } else if (rest == 1 && (freeLinks == 1 || validEdges(v) == 1)) {
        for (auto e : v.edges) {
          if (!theNodes[e.first].isFull() && !crossWall(v, theNodes[e.first])) {
            createLink(v, e.first, 1);
            break;
          }
        }
      } else if ((v.edges.size() == 2 && v.p == 3) || (v.edges.size() == 3 && v.p == 5) ||
                 (v.edges.size() == 4 && v.p == 7)) {
        // linking
        // cerr << "   linking " << v << endl;
        for (auto e : v.edges) {
          if (e.second == 0) {
            // cerr << " links " << e.first << " has " << e.second << endl;
            cut = true;
            createLink(v, e.first, 1);
          }
        }
      }
    }
  }
  return cut;
}

vector<map<int, int> > getCombi(int e, int v) {
  vector<map<int, int> > V;
  map<int, int>          m;
  // for (int i(0); i<e; ++i) {
  //    for (int j(0); j<2
  //}
  if (e == 2 and v == 1) {
    m[0] = 1;
    m[1] = 0;
    V.push_back(m);
    m[0] = 0;
    m[1] = 1;
    V.push_back(m);
  } else if (e == 2 and v == 2) {
    m[0] = 1;
    m[1] = 1;
    V.push_back(m);
    m[0] = 2;
    m[1] = 0;
    V.push_back(m);
    m[0] = 0;
    m[1] = 2;
    V.push_back(m);
  } else if (e == 2 and v == 3) {
    m[0] = 2;
    m[1] = 1;
    V.push_back(m);
    m[0] = 1;
    m[1] = 2;
    V.push_back(m);
  } else if (e == 3 and v == 1) {
    m[0] = 1;
    m[1] = 0;
    m[2] = 0;
    V.push_back(m);
    m[0] = 0;
    m[1] = 1;
    m[2] = 0;
    V.push_back(m);
    m[0] = 0;
    m[1] = 0;
    m[2] = 1;
    V.push_back(m);
  } else if (e == 3 and v == 2) {
    m[0] = 1;
    m[1] = 1;
    m[2] = 0;
    V.push_back(m);
    m[0] = 0;
    m[1] = 1;
    m[2] = 1;
    V.push_back(m);
    m[0] = 1;
    m[1] = 0;
    m[2] = 1;
    V.push_back(m);
    m[0] = 2;
    m[1] = 0;
    m[2] = 0;
    V.push_back(m);
    m[0] = 0;
    m[1] = 2;
    m[2] = 0;
    V.push_back(m);
    m[0] = 0;
    m[1] = 0;
    m[2] = 2;
    V.push_back(m);
  } else if (e == 3 and v == 3) {
    m[0] = 1;
    m[1] = 1;
    m[2] = 1;
    V.push_back(m);
    m[0] = 2;
    m[1] = 1;
    m[2] = 0;
    V.push_back(m);
    m[0] = 1;
    m[1] = 2;
    m[2] = 0;
    V.push_back(m);
    m[0] = 1;
    m[1] = 0;
    m[2] = 2;
    V.push_back(m);
    m[0] = 2;
    m[1] = 0;
    m[2] = 1;
    V.push_back(m);
    m[0] = 0;
    m[1] = 1;
    m[2] = 2;
    V.push_back(m);
    m[0] = 0;
    m[1] = 2;
    m[2] = 1;
    V.push_back(m);
  } else if (e == 3 and v == 4) {
    m[0] = 2;
    m[1] = 2;
    m[2] = 0;
    V.push_back(m);
    m[0] = 0;
    m[1] = 2;
    m[2] = 2;
    V.push_back(m);
    m[0] = 2;
    m[1] = 0;
    m[2] = 2;
    V.push_back(m);
    m[0] = 2;
    m[1] = 1;
    m[2] = 1;
    V.push_back(m);
    m[0] = 1;
    m[1] = 2;
    m[2] = 1;
    V.push_back(m);
    m[0] = 1;
    m[1] = 1;
    m[2] = 2;
    V.push_back(m);
  } else if (e == 3 and v == 5) {
    m[0] = 2;
    m[1] = 2;
    m[2] = 1;
    V.push_back(m);
    m[0] = 2;
    m[1] = 1;
    m[2] = 2;
    V.push_back(m);
    m[0] = 1;
    m[1] = 2;
    m[2] = 2;
    V.push_back(m);
  } else if (e == 4 and v == 1) {
    m[0] = 1;
    m[1] = 0;
    m[2] = 0;
    m[3] = 0;
    V.push_back(m);
    m[0] = 0;
    m[1] = 1;
    m[2] = 0;
    m[3] = 0;
    V.push_back(m);
    m[0] = 0;
    m[1] = 0;
    m[2] = 1;
    m[3] = 0;
    V.push_back(m);
    m[0] = 0;
    m[1] = 0;
    m[2] = 0;
    m[3] = 1;
    V.push_back(m);
  } else if (e == 4 and v == 2) {
    m[0] = 1;
    m[1] = 1;
    m[2] = 0;
    m[3] = 0;
    V.push_back(m);
    m[0] = 0;
    m[1] = 1;
    m[2] = 1;
    m[3] = 0;
    V.push_back(m);
    m[0] = 0;
    m[1] = 0;
    m[2] = 1;
    m[3] = 1;
    V.push_back(m);
    m[0] = 0;
    m[1] = 1;
    m[2] = 0;
    m[3] = 1;
    V.push_back(m);
    m[0] = 1;
    m[1] = 0;
    m[2] = 1;
    m[3] = 0;
    V.push_back(m);
    m[0] = 1;
    m[1] = 0;
    m[2] = 0;
    m[3] = 1;
    V.push_back(m);
    m[0] = 2;
    m[1] = 0;
    m[2] = 0;
    m[3] = 0;
    V.push_back(m);
    m[0] = 0;
    m[1] = 2;
    m[2] = 0;
    m[3] = 0;
    V.push_back(m);
    m[0] = 0;
    m[1] = 0;
    m[2] = 2;
    m[3] = 0;
    V.push_back(m);
    m[0] = 0;
    m[1] = 0;
    m[2] = 0;
    m[3] = 2;
    V.push_back(m);
  } else if (e == 4 and v == 3) {
    m[0] = 1;
    m[1] = 1;
    m[2] = 1;
    m[3] = 0;
    V.push_back(m);
    m[0] = 1;
    m[1] = 1;
    m[2] = 0;
    m[3] = 1;
    V.push_back(m);
    m[0] = 1;
    m[1] = 0;
    m[2] = 1;
    m[3] = 1;
    V.push_back(m);
    m[0] = 0;
    m[1] = 1;
    m[2] = 1;
    m[3] = 1;
    V.push_back(m);
    m[0] = 2;
    m[1] = 1;
    m[2] = 0;
    m[3] = 0;
    V.push_back(m);
    m[0] = 2;
    m[1] = 0;
    m[2] = 1;
    m[3] = 0;
    V.push_back(m);
    m[0] = 2;
    m[1] = 0;
    m[2] = 0;
    m[3] = 1;
    V.push_back(m);
    m[0] = 1;
    m[1] = 2;
    m[2] = 0;
    m[3] = 0;
    V.push_back(m);
    m[0] = 0;
    m[1] = 2;
    m[2] = 1;
    m[3] = 0;
    V.push_back(m);
    m[0] = 0;
    m[1] = 2;
    m[2] = 0;
    m[3] = 1;
    V.push_back(m);
    m[0] = 1;
    m[1] = 0;
    m[2] = 2;
    m[3] = 0;
    V.push_back(m);
    m[0] = 0;
    m[1] = 1;
    m[2] = 2;
    m[3] = 0;
    V.push_back(m);
    m[0] = 0;
    m[1] = 0;
    m[2] = 2;
    m[3] = 1;
    V.push_back(m);
    m[0] = 1;
    m[1] = 0;
    m[2] = 0;
    m[3] = 2;
    V.push_back(m);
    m[0] = 0;
    m[1] = 1;
    m[2] = 0;
    m[3] = 2;
    V.push_back(m);
    m[0] = 0;
    m[1] = 0;
    m[2] = 1;
    m[3] = 2;
    V.push_back(m);
  } else if (e == 4 and v == 4) {
    m[0] = 1;
    m[1] = 1;
    m[2] = 1;
    m[3] = 1;
    V.push_back(m);
    m[0] = 2;
    m[1] = 1;
    m[2] = 1;
    m[3] = 0;
    V.push_back(m);
    m[0] = 2;
    m[1] = 1;
    m[2] = 0;
    m[3] = 1;
    V.push_back(m);
    m[0] = 2;
    m[1] = 0;
    m[2] = 1;
    m[3] = 1;
    V.push_back(m);
    m[0] = 1;
    m[1] = 2;
    m[2] = 1;
    m[3] = 0;
    V.push_back(m);
    m[0] = 1;
    m[1] = 2;
    m[2] = 0;
    m[3] = 1;
    V.push_back(m);
    m[0] = 0;
    m[1] = 2;
    m[2] = 1;
    m[3] = 1;
    V.push_back(m);
    m[0] = 1;
    m[1] = 1;
    m[2] = 2;
    m[3] = 0;
    V.push_back(m);
    m[0] = 1;
    m[1] = 0;
    m[2] = 2;
    m[3] = 1;
    V.push_back(m);
    m[0] = 0;
    m[1] = 1;
    m[2] = 2;
    m[3] = 1;
    V.push_back(m);
    m[0] = 1;
    m[1] = 1;
    m[2] = 0;
    m[3] = 2;
    V.push_back(m);
    m[0] = 1;
    m[1] = 0;
    m[2] = 1;
    m[3] = 2;
    V.push_back(m);
    m[0] = 0;
    m[1] = 1;
    m[2] = 1;
    m[3] = 2;
    V.push_back(m);
    m[0] = 2;
    m[1] = 2;
    m[2] = 0;
    m[3] = 0;
    V.push_back(m);
    m[0] = 2;
    m[1] = 0;
    m[2] = 2;
    m[3] = 0;
    V.push_back(m);
    m[0] = 2;
    m[1] = 0;
    m[2] = 0;
    m[3] = 2;
    V.push_back(m);
    m[0] = 0;
    m[1] = 2;
    m[2] = 2;
    m[3] = 0;
    V.push_back(m);
    m[0] = 0;
    m[1] = 0;
    m[2] = 2;
    m[3] = 2;
    V.push_back(m);
    m[0] = 0;
    m[1] = 2;
    m[2] = 0;
    m[3] = 2;
    V.push_back(m);
  } else if (e == 4 and v == 5) {
    m[0] = 2;
    m[1] = 1;
    m[2] = 1;
    m[3] = 1;
    V.push_back(m);
    m[0] = 1;
    m[1] = 2;
    m[2] = 1;
    m[3] = 1;
    V.push_back(m);
    m[0] = 1;
    m[1] = 1;
    m[2] = 2;
    m[3] = 1;
    V.push_back(m);
    m[0] = 1;
    m[1] = 1;
    m[2] = 1;
    m[3] = 2;
    V.push_back(m);
    m[0] = 2;
    m[1] = 2;
    m[2] = 1;
    m[3] = 0;
    V.push_back(m);
    m[0] = 2;
    m[1] = 2;
    m[2] = 0;
    m[3] = 1;
    V.push_back(m);
    m[0] = 2;
    m[1] = 0;
    m[2] = 2;
    m[3] = 1;
    V.push_back(m);
    m[0] = 2;
    m[1] = 1;
    m[2] = 2;
    m[3] = 0;
    V.push_back(m);
    m[0] = 2;
    m[1] = 0;
    m[2] = 1;
    m[3] = 2;
    V.push_back(m);
    m[0] = 2;
    m[1] = 1;
    m[2] = 0;
    m[3] = 2;
    V.push_back(m);
    m[0] = 1;
    m[1] = 2;
    m[2] = 2;
    m[3] = 0;
    V.push_back(m);
    m[0] = 0;
    m[1] = 2;
    m[2] = 2;
    m[3] = 1;
    V.push_back(m);
    m[0] = 0;
    m[1] = 1;
    m[2] = 2;
    m[3] = 2;
    V.push_back(m);
    m[0] = 1;
    m[1] = 0;
    m[2] = 2;
    m[3] = 2;
    V.push_back(m);
    m[0] = 0;
    m[1] = 2;
    m[2] = 1;
    m[3] = 2;
    V.push_back(m);
    m[0] = 1;
    m[1] = 2;
    m[2] = 0;
    m[3] = 2;
    V.push_back(m);
  } else if (e == 4 and v == 6) {
    m[0] = 2;
    m[1] = 2;
    m[2] = 1;
    m[3] = 1;
    V.push_back(m);
    m[0] = 2;
    m[1] = 1;
    m[2] = 2;
    m[3] = 1;
    V.push_back(m);
    m[0] = 2;
    m[1] = 1;
    m[2] = 1;
    m[3] = 2;
    V.push_back(m);
    m[0] = 1;
    m[1] = 2;
    m[2] = 2;
    m[3] = 1;
    V.push_back(m);
    m[0] = 1;
    m[1] = 1;
    m[2] = 2;
    m[3] = 2;
    V.push_back(m);
    m[0] = 1;
    m[1] = 2;
    m[2] = 1;
    m[3] = 2;
    V.push_back(m);
    m[0] = 2;
    m[1] = 2;
    m[2] = 2;
    m[3] = 0;
    V.push_back(m);
    m[0] = 2;
    m[1] = 2;
    m[2] = 0;
    m[3] = 2;
    V.push_back(m);
    m[0] = 2;
    m[1] = 0;
    m[2] = 2;
    m[3] = 2;
    V.push_back(m);
    m[0] = 0;
    m[1] = 2;
    m[2] = 2;
    m[3] = 2;
    V.push_back(m);
  } else if (e == 4 and v == 7) {
    m[0] = 2;
    m[1] = 2;
    m[2] = 2;
    m[3] = 1;
    V.push_back(m);
    m[0] = 2;
    m[1] = 2;
    m[2] = 1;
    m[3] = 2;
    V.push_back(m);
    m[0] = 2;
    m[1] = 1;
    m[2] = 2;
    m[3] = 2;
    V.push_back(m);
    m[0] = 1;
    m[1] = 2;
    m[2] = 2;
    m[3] = 2;
    V.push_back(m);
  }
  return V;
}

bool combiValid(map<int, int> &c, Node &n) {
  int idx(0);
  for (auto e : n.edges) {
    // cerr << "  test " << c[idx] << " links from " << n.i << " to " << e.first << ", has " << e.second << " reste "
    //     << theNodes[e.first].rest() << " cross " << crossWall(n, theNodes[e.first]) << endl;
    if (c[idx] > 0 &&
        (theNodes[e.first].rest() < c[idx] - e.second || e.second > c[idx] || crossWall(n, theNodes[e.first]))) {
      return false;
    }
    idx++;
  }
  return true;
}
int assignCombi(map<int, int> &c, Node &n, vector<int> &delta) {
  int idx(0), nb(0);
  for (auto e : n.edges) {
    int l(c[idx] - e.second);
    if (l > 0) {
      createLink(n, e.first, l);
      delta.push_back(l);
      nb++;
    } else {
      delta.push_back(0);
    }
    idx++;
  }
  return nb;
}
void removeCombi(Node &n, vector<int> &delta, int nb) {
  int idx(0);
  for (auto &e : n.edges) {
    e.second -= delta[idx];
    theNodes[e.first].edges[n.i] -= delta[idx];
    removeWall(n, theNodes[e.first]);
    idx++;
  }
  for (int i = 0; i < nb; ++i) {
    result.pop_back();
  }
}

bool isValid(int id) {
  if (id >= theNodes.size()) {
    cerr << id << ": end of loop" << endl;
    return true;
  }
  int n(theCopy[id].i);
  if (theNodes[n].rest() <= 0) {
    // cerr << "node " << n << theNodes[n]<< " is complete" << endl;
    return isValid(++id);
  }
  // cerr << "Traite " << theNodes[n] << endl;

  int j(0);
  for (auto c : getCombi(theNodes[n].edges.size(), theNodes[n].p)) {
    // cerr << "combi " << j << " node " << n << endl;
    if (combiValid(c, theNodes[n])) {
      vector<int> delta;
      int         nb = assignCombi(c, theNodes[n], delta);
      if (isValid(id + 1)) {
        return true;
      } else {
        // cerr << "remove combi " << j << " for node " << n << endl;
        removeCombi(theNodes[n], delta, nb);
      }
    }
    j++;
  }
  cerr << "no valid combi for " << n << endl;
  return false;
}

void traceNodes(const vector<Node> &V, string w) {
  int c(0);
  for (auto v : V) {
    cerr << w + " [" << c << "] " << v << endl;
    c++;
  }
}
void traceCombi(const vector<map<int, int> > &V) {
  int c(0);
  for (auto v : V) {
    cerr << "map " << c << endl;
    for (auto m : v) {
      cerr << "assign " << m.second << " link to node " << m.first << endl;
    }
    c++;
  }
}
/**
 * The machines are gaining ground. Time to show them what we're really made of...
 **/

int main() {
  int width;  // the number of cells on the X axis
  cin >> width;
  cin.ignore();
  vector<int> colHist(width, -1);
  int         height;  // the number of cells on the Y axis
  cin >> height;
  cin.ignore();
  vector<int> rowHist(height, -1);
  for (int i = 0; i < height; i++) {
    string line;  // width characters, each either a number or a '.'
    getline(cin, line);
    // cerr << line << endl;
    for (int j(0); j < width; ++j) {
      if (line[j] != '.') {
        // Get the node value
        int value = line[j] - '0';
        // Save the node in vector
        int  curr = theNodes.size();
        Node n(curr, j, i, value);
        theNodes.push_back(n);
        // cerr << "created node " << curr << " with ancestors c:" << colHist[j] << " r:" << rowHist[i] <<endl;
        // Create edges based on previous nodes from col and row
        upEdges(theNodes[curr], curr, colHist[j]);
        upEdges(theNodes[curr], curr, rowHist[i]);
      }
    }
  }

  // traceCombi(getCombi(4,4));
  // traceNodes(theNodes, " cc");
  bool canPrune(false);
  do {
    canPrune = prune();
  } while (canPrune);
  theCopy = theNodes;
  sort(theCopy.begin(), theCopy.end(), [](Node a, Node b) {
    if (a.rest() == b.rest()) {
      if (a.p == b.p) {
        return a.edges.size() < b.edges.size();
      } else {
        return a.p > b.p;
      }
    } else {
      return a.rest() < b.rest();
    }
  });
  traceNodes(theCopy, " uu");
  // isValid(0);

  for (auto o : result) {
    o.print();
  }
}
