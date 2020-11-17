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

typedef enum player { ME = 0, HIM } player_t;

struct Compo {
  int x, y, z, t;
  int i;

  inline Compo() : x(0), y(0), z(0), t(0), i(0) {}
  inline Compo(int x) : x(x), y(0), z(0), t(0), i(0) {}
  inline Compo(int x, int y, int z, int t) : x(x), y(y), z(z), t(t), i(0) {}
  inline Compo(int x, int y, int z, int t, int i) : x(x), y(y), z(z), t(t), i(i) {}
  inline Compo(Compo const &) = default;
  inline Compo(Compo &&) = default;

  inline Compo &operator=(Compo const &) = default;
  inline Compo &operator=(Compo &&) = default;
  inline bool operator==(const Compo &c) const { return x == c.x and y == c.y and z == c.z and t == c.t and i == c.i; }
  inline int raw_cost() const { return abs(x) + 10 * abs(y) + 100 * abs(z) + 1000 * abs(t); }
  inline bool operator!=(const Compo &c) const { return !((*this) == c); }
  inline bool operator<(const Compo &c) const { return raw_cost() < c.raw_cost(); }
  inline Compo operator+(const Compo &c) const { return Compo(x + c.x, y + c.y, z + c.z, t + c.t); }
  inline Compo operator-(const Compo &c) const { return Compo(x - c.x, y - c.y, z - c.z, t - c.t); }
  inline Compo distance(const Compo &c) const {
    return Compo(max(0, -1 * c.x - x), max(0, -1 * c.y - y), max(0, -1 * c.z - z), max(0, -1 * c.t - t));
  }
  inline Compo residual(const Compo &c) const {
    return Compo(max(0, c.x + x), max(0, c.y + y), max(0, c.z + z), max(0, c.t + t));
  }
  inline int sum() const { return x + y + z + t; }
  inline bool canCook(const Compo &c) const { return x + c.x >= 0 and y + c.y >= 0 and z + c.z >= 0 and t + c.t >= 0; }
  inline Compo requirement() const { return Compo(min(0, x), min(0, y), min(0, z), min(0, t)); }
  inline vector<int> list_elements() const {
    vector<int> elements{x, y, z, t};
    int value(0);
    vector<int> v;
    for (auto e : elements) {
      for (int j(0); j < abs(e); ++j) {
        v.push_back(value);
      }
      value++;
    }
    return v;
  }
  inline bool onlyProduce() const { return x >= 0 and y >= 0 and z >= 0 and t >= 0; }
  virtual void serialize(ostream &os) const { os << "C#" << i << "[" << x << "," << y << "," << z << "," << t << "]"; }
};
ostream &operator<<(ostream &os, const Compo &c) {
  c.serialize(os);
  return os;
}

struct Order {
  int id;
  Compo compo;
  string type;
  int cost;
  bool castable, repeat;
  int index, tax;
  inline Order() : id(0), compo(), type("BREW"), cost(0), castable(false), repeat(false), index(-1), tax(0) {}
  inline Order(Order const &) = default;
  inline Order(Order &&) = default;
  inline Order &operator=(Order const &) = default;
  inline Order &operator=(Order &&) = default;
  inline bool canProduce(int element) {
    return (element == 0 and compo.x > 0) or (element == 1 and compo.y > 0) or (element == 2 and compo.z > 0) or
           (element == 3 and compo.t > 0);
  }
  int spellCost(int element, int *cost0, int *cost1, int *cost2, int *cost3) {
    int turn(0);
    if (element == 0 or element == 1) {
      turn = 1;
    } else if (element == 2) {
      turn = 2;
    } else {
      turn = 3;
    }
    *cost0 = 1;
    return turn + 1;
  }
  int distance(const Compo &inv) {
    auto craft = inv.distance(compo);
    auto reste = inv.residual(compo);
    auto liste = craft.list_elements();
    int nbTurns(0);
    while (liste.size() > 0) {
      auto needed = liste.back();
      liste.pop_back();
      int cost0(0), cost1(0), cost2(0), cost3(0);
      nbTurns += spellCost(needed, &cost0, &cost1, &cost2, &cost3);
      if (reste.x > cost0) {
        reste.x -= cost0;
      } else {
        reste.x += 2;  // local hardcode
        nbTurns++;
      }
    }
    return nbTurns;
  }
};

namespace std {
template <>
struct hash<Order> {
  inline size_t operator()(const Order &p) const { return p.compo.x * 1812433253 + p.compo.y; }
};
template <>
struct hash<Compo> {
  inline size_t operator()(const Compo &p) const { return p.x * 1812433253 + p.y; }
};
}  // namespace std
struct Action {
  string s;
  int id;
  string m;
  inline Action() : s("WAIT"), id(), m("raf") {}
  inline Action(string s, string m) : s(s), m(m) {}
  inline Action(string s, int i, string m) : s(s), id(i), m(m) {}
  inline Action(Action const &) = default;
  inline Action(Action &&) = default;
  inline Action &operator=(Action const &) = default;
  inline Action &operator=(Action &&) = default;
  void play() {
    // in the first league: BREW <id> | WAIT; later: BREW <id> | CAST <id> [<times>] | LEARN <id> | REST | WAIT
    if (s == "REST") {
      cout << s << " " << m << endl;
    } else {
      cout << s << " " << id << " " << m << endl;
    }
  }
};
struct Game {
  array<Order, 1000> orders;
  vector<Order> currentOrders, book;
  unordered_set<Compo> compos;
  unordered_map<Compo, int> dict;
  array<Compo, 2> inventories;
  array<int, 2> scores, brew;
  array<vector<Order>, 2> spells;
  int bestOrder, canCast;
  array<vector<int>, 1000> combo_list;
  array<int, 1000> combo_cost;
  array<Compo, 1000> combo_compo;
  map<float, int> wishlist;
  bool orderIsSelected;

  inline Game() : orderIsSelected(false) {}

  void init() {
    cerr << "#init" << endl;
    currentOrders.clear();
    spells[0].clear();
    spells[1].clear();
    book.clear();
    canCast = 0;
    int nbActions;
    cin >> nbActions;
    cin.ignore();
    int comboId(0);
    for (int i = 0; i < nbActions; i++) {
      int id(0);
      cin >> id;
      cin.ignore();
      orders[id].id = id;
      cin >> orders[id].type >> orders[id].compo.x >> orders[id].compo.y >> orders[id].compo.z >> orders[id].compo.t >>
          orders[id].cost >> orders[id].index >> orders[id].tax >> orders[id].castable >> orders[id].repeat;
      if (orders[id].type == "BREW") {
        currentOrders.push_back(orders[id]);
      } else if (orders[id].type == "CAST") {
        spells[0].push_back(orders[id]);
        Compo c(orders[id].compo);
        // c.i = compos.size();
        dict[c] = compos.size();
        combo_list[compos.size()].push_back(compos.size());
        combo_cost[compos.size()] = 1;
        combo_compo[compos.size()] = c;
        compos.insert(c);
        canCast += static_cast<int>(orders[id].castable);
      } else if (orders[id].type == "OPPONENT_CAST") {
        spells[1].push_back(orders[id]);
      } else if (orders[id].type == "LEARN") {
        // cerr << " spell id " << orders[id].index << endl;
        book.push_back(orders[id]);
      }
    }
    for (int i = 0; i < 2; i++) {
      int score;
      cin >> inventories[i].x >> inventories[i].y >> inventories[i].z >> inventories[i].t >> score;
      cin.ignore();
      if (i == 1 and score > scores[i]) {
        orderIsSelected = false;  // re-assess selection in case of new orders
        brew[1]++;
      }
      scores[i] = score;
      // cerr << "inv " << i << " " << inventories[i] << endl;
    }
    cerr << " brew " << brew[0] << "-" << brew[1] << endl;
  }
  bool wasOrdered() {
    for (auto c : currentOrders) {
      if (c.id == bestOrder) return false;
    }
    return true;
  }

  int sum_turns(vector<int> v) {
    map<int, int> count;
    for (auto e : v) {
      if (count.count(e)) {
        count[e]++;
      } else {
        count[e] = 1;
      }
    }
    int m(0);
    for (auto c : count) {
      if (c.second > m) {
        m = c.second;
      }
    }
    return v.size() + m - 1;
  }

  void combineSpells() {
    vector<Compo> created(compos.begin(), compos.end());
    unordered_set<Compo> fresh;
    cerr << "created " << created.size() << " compos " << compos.size() << endl;
    int k(0), n(4);
    while (k < 10) {
      fresh.clear();
      for (auto c : created) {
        for (auto s : compos) {
          if (s.x * c.x < 0 or s.y * c.y < 0 or s.z * c.z < 0 or s.t * c.t < 0) {
            auto combo = s + c;
            int comboId = compos.size() + fresh.size();
            fresh.insert(combo);
            if (!compos.count(combo)) {
              dict[combo] = comboId;
            }
            combo_compo[comboId] = combo;
            combo_list[comboId] = combo_list[dict[c]];
            combo_list[comboId].insert(combo_list[comboId].end(), combo_list[s.i].begin(), combo_list[s.i].end());
            combo_cost[comboId] = sum_turns(combo_list[comboId]);
          }
        }
      }
      cerr << k << "# " << fresh.size() << " fresh" << endl;
      if (fresh.size() == 0) break;
      compos.insert(fresh.begin(), fresh.end());
      created.assign(fresh.begin(), fresh.end());
      k++;
    }
  }

  Action findBestOrder() {
    if (compos.size() == spells[0].size()) {
      combineSpells();
      cerr << compos.size() << endl;
    }
    int m(1000), cid(0);
    for (auto c : compos) {
      cerr << c << " cost " << combo_cost[dict[c]] << endl;
      if (c.t > 0 and combo_cost[dict[c]] < m and combo_cost[dict[c]] > 1) {
        cid = dict[c];
        m = combo_cost[dict[c]];
      }
    }
    cerr << "cheapest path to triangle: " << m << "," << combo_compo[cid] << endl;
    for (auto o : currentOrders) {
      for (auto c : compos) {
        auto n = inventories[0] + c;
        if (n.canCook(o.compo)) {
          cerr << " I can cook " << o.id << " costing" << o.cost << " with compo " << n << endl;
          float f = o.cost / static_cast<float>(combo_cost[dict[c]]);
          wishlist[f] = o.id;
        }
      }
    }
    auto w = wishlist.begin();
    cerr << "wished is " << w->second << endl;

    sort(currentOrders.begin(), currentOrders.end(), [](const Order &a, const Order &b) { return a.cost > b.cost; });
    auto f = currentOrders.begin();
    return Action("BREW", f->id, "expensive");
  }

  void selectOrder() {
    cerr << "#select" << endl;
    float m(0);
    for (auto o : currentOrders) {
      double d(0.0);
      if (brew[0] == 5) {
        bool canBeat = scores[0] + o.cost >= scores[1];
        d = static_cast<int>(canBeat) * (100 - o.distance(inventories[0]));
      } else {
        // d = o.cost * static_cast<float>(pow(0.95, o.distance(inventories[0])));
        d = o.cost / static_cast<double>(pow(o.distance(inventories[0]), 1));
      }
      cerr << " order " << o.id << " value " << d << endl;
      if (d > m) {
        bestOrder = o.id;
        m = d;
      }
    }
    orderIsSelected = true;
    cerr << "selected " << bestOrder << " reward " << orders[bestOrder].cost << " value " << m << endl;
  }

  Order primarySpell(const Compo &required, const Compo &residu) {
    auto current = required;
    Order spell;
    int n(0);
    while (!residu.canCook(current) and n < canCast) {
      cerr << n << " cant " << current << " with " << residu << endl;
      for (auto e : current.list_elements()) {
        for (auto s : spells[0]) {
          if (s.canProduce(e)) {
            cerr << " " << s.id << " do " << e << endl;
            current = s.compo.requirement();
            spell = s;
            break;
          }
        }
      }
      ++n;
    }
    if (n == canCast) {
      spell.castable = false;
    }
    return spell;
  }
  Order goodSpell() {
    int n(0);
    for (auto b : book) {
      Compo tax(-n);
      if (b.compo.onlyProduce() and inventories[0].canCook(tax)) {
        return b;
      }
      n++;
    }
    return *(book.begin());
  }

  Action nextAction() {
    cerr << "#next" << endl;
    auto s = goodSpell();
    if (spells[0].size() < 6 or s.compo.onlyProduce() or s.tax >= 2) {
      return Action("LEARN", s.id, "fast learning");
    }
    auto d = inventories[0].distance(orders[bestOrder].compo);
    cerr << " need " << d << endl;
    if (d.sum() == 0) {
      orderIsSelected = false;
      brew[0]++;
      return Action("BREW", bestOrder, "brew");
    }
    for (auto l : d.list_elements()) {
      cerr << "need element " << l << endl;
      for (auto s : spells[0]) {
        if (s.castable) {
          if (s.canProduce(l) and inventories[0].sum() + s.compo.sum() < 11) {
            if (inventories[0].canCook(s.compo)) {
              return Action("CAST", s.id, "crafting");
            } else {
              cerr << " get " << l << " miss " << s.compo << endl;
              auto t = primarySpell(s.compo.requirement(), inventories[0].residual(orders[bestOrder].compo));
              if (t.castable and inventories[0].sum() + t.compo.sum() < 11) {
                return Action("CAST", t.id, "primary");
              }
            }
          }
        }
      }
    }
    if (s.tax >= 2) {
      return Action("LEARN", s.id, "tax");
    }
    return Action("REST", "restore burma");
  }
};

int main() {
  Game theGame;
  // game loop
  while (1) {
    theGame.init();
    if (!theGame.orderIsSelected or theGame.wasOrdered()) {
      theGame.selectOrder();
    }
    Action a = theGame.nextAction();
    a.play();
  }
}