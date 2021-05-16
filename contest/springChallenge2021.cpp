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


struct Cell {
  int index, richness;
  array<int, 6> neigh;
  inline Cell() = default;

  inline Cell &operator=(Cell const &) = default;
  inline Cell &operator=(Cell &&) = default;
  inline bool operator==(const Cell &c) const { return this->richness == c.richness and this->index == c.index; }
  inline bool operator!=(const Cell &c) const { return !((*this) == c); }
  virtual void serialize(ostream &os) const { os << "#" << index << ";" << richness; }

};
ostream &operator<<(ostream &os, const Cell &c) {
  c.serialize(os);
  return os;
}
struct Tree {
  int cellIndex, size, score;
  bool isMine, isDormant; // 1 if this tree is dormant

  inline Tree() = default;
  inline Tree &operator=(Tree const &) = default;
  // inline Tree &operator=(Tree &&) = default;
  inline bool operator==(const Tree &c) const { return this->cellIndex == c.cellIndex and this->size == c.size and this->isMine == c.isMine and this->isDormant == c.isDormant; }
  inline bool operator<(const Tree &c) const { return this->score < c.score; }
  inline bool operator>(const Tree &c) const { return this->score > c.score; }
  inline bool operator!=(const Tree &c) const { return !((*this) == c); }
  
  int growingCost(const array<int, 4> &nbTree) {
    array<int, 3> baseCost = {1, 3, 7};
    if (size > 2) return 0;
    return baseCost[size] + nbTree[size + 1];
  }

  virtual void serialize(ostream &os) const { os << "*" << cellIndex << ";" << size << ";" << isMine << ";" << isDormant; }
};
ostream &operator<<(ostream &os, const Tree &c) {
  c.serialize(os);
  return os;
}

namespace std {
template <>
struct hash<Cell> {
  inline size_t operator()(const Cell &p) const { return p.index * 1812433253 + p.richness; }
};
template <>
struct hash<Tree> {
  inline size_t operator()(const Tree &p) const { return p.cellIndex * 1812433253 + p.size; }
};
}  // namespace std
struct Action {
  string s;
  int id, idx;
  string m;
  inline Action() : s("WAIT"), id(), m("raf") {}
  inline Action(string s, string m) : s(s), m(m) {}
  inline Action(string s, int i, string m) : s(s), id(i), m(m) {}
  inline Action(Action const &) = default;
  inline Action(Action &&) = default;

  inline Action &operator=(Action const &) = default;
  inline Action &operator=(Action &&) = default;
  void play() {
    if (s == "WAIT") {
      cout << s << " " << m << endl;
    } else {
      cout << s << " " << id << " " << m << endl;
    }
  }
};
struct Score {
    int sun, score, sum;
    array<int, 4> nbTree;
  inline Score() = default;

  inline Score &operator=(Score const &) = default;
  inline Score &operator=(Score &&) = default;
  inline bool operator==(const Score &c) const { return this->sun == c.sun and this->score == c.score; }
  inline bool operator!=(const Score &c) const { return !((*this) == c); }

  bool canSeed() {
    return sun >= nbTree[0];
  }
  virtual void serialize(ostream &os) const { os << "!" << sun << ";" << score; }
};
ostream &operator<<(ostream &os, const Score &c) {
  c.serialize(os);
  return os;
}
struct Game {
  int nbCells, day, nutrients, nbTrees, nbActions;
  bool heIsWaiting;
  array<int, 3> richnessBonus = {0, 2, 4};
  array<Cell, 37> cells;
  array<Score, 2> scores;
  array<Tree, 37> trees;
  array<bool, 37> free;
  array<string, 100> actions;
 
  inline Game() : nbCells(0), day(0) {}

  void init() {
    cin >> nbCells; cin.ignore();
    for (int i = 0; i < nbCells; i++) {
        cin >> cells[i].index >> cells[i].richness >> cells[i].neigh[0] >> cells[i].neigh[1] >> cells[i].neigh[2] >> cells[i].neigh[3] >> cells[i].neigh[4] >> cells[i].neigh[5]; cin.ignore();
    }
    for (int i(0); i< 37; ++i) {
      trees[i] = Tree();
    }
  }
  void turnInit() {
        cin >> day; cin.ignore();
        cin >> nutrients; cin.ignore();
        cin >> scores[ME].sun >> scores[ME].score; cin.ignore();
        cerr << scores[ME] <<endl;
        cin >> scores[HIM].sun >> scores[HIM].score >> heIsWaiting; cin.ignore();
        cerr << scores[HIM] <<endl;
        fill(scores[ME].nbTree.begin(),scores[ME].nbTree.end(), 0);
        fill(scores[HIM].nbTree.begin(),scores[HIM].nbTree.end(), 0);
        scores[ME].sum = 0;
        scores[HIM].sum = 0;
        fill(free.begin(),free.end(), true);
        cin >> nbTrees; cin.ignore();
        for (int i = 0; i < nbTrees; i++) {
            cin >> trees[i].cellIndex >> trees[i].size >> trees[i].isMine >> trees[i].isDormant; cin.ignore();
            trees[i].score = nutrients + richnessBonus[max(0, cells[trees[i].cellIndex].richness - 1)];
            free[trees[i].cellIndex] = false;
            for (int treeSize(0); treeSize < 4; treeSize++)
              scores[!trees[i].isMine].nbTree[treeSize] += static_cast<int>(trees[i].size == treeSize);
            scores[!trees[i].isMine].sum += trees[i].size + 1;
        }
        cerr << "'" << scores[ME].sum << ";" << scores[HIM].sum << endl;
        cin >>  nbActions; cin.ignore();
        for (int i = 0; i <  nbActions; i++) {
            getline(cin, actions[i]); // try printing something from here to start with
        }
        for (auto n : scores[ME].nbTree) {
          cerr<< " |" << n << endl;
        }
  }
  set<int> neighbors(int start, int depth) {
  queue<int> frontier;
  frontier.push(start);

  set<int> path;
  map<int, int> distances;
  path.insert(start);
  distances[start] = 0;

  while (!frontier.empty()) {
    int current = frontier.front();
    frontier.pop();
    // if (distances[current] > depth) break;

    for (auto next : cells[current].neigh) {
      if (next > 0 and distances[current] < depth and !path.count(next)) {
        distances[next] = distances[current] + 1;
        frontier.push(next);
        path.insert(next);
      }
    }
  }
  return path;
}
bool canGreen() {
  for (int i(0); i< 7; ++i) {
    if (free[i] and cells[i].richness > 0) return true;
  }
  return false;
}
bool shouldSeed(const Tree &tree) {
  auto maxTree = max_element(scores[ME].nbTree.begin(), scores[ME].nbTree.end());
  return scores[ME].sum < scores[HIM].sum and (*maxTree > scores[ME].nbTree[0] or (tree.cellIndex < 19 and canGreen()));
}

  Action selectBestTree() {
    cerr << "select" << endl;
    sort(trees.begin(), trees.begin() + nbTrees, [](Tree a, Tree b) {if (a.score == b.score) {return a.size > b.size;} else {return a.score > b.score;}});
    for (int i(0); i < nbTrees; ++i) {
      cerr << trees[i] << " c: " << trees[i].growingCost(scores[ME].nbTree) << endl;
      if (!trees[i].isMine or trees[i].isDormant) continue;
      if (scores[ME].sun > 3 and trees[i].size == 3 and (cells[trees[i].cellIndex].richness < 3 or day > 18)) {
        cerr <<cells[trees[i].cellIndex].richness << "|" << day<<endl;
        free[trees[i].cellIndex] = true;
        return Action("COMPLETE", trees[i].cellIndex, "banque");
      }
      if (trees[i].size < 3 and trees[i].growingCost(scores[ME].nbTree) <= scores[ME].sun) {
        return Action("GROW", trees[i].cellIndex, "croissance");
      }
      auto n = neighbors(trees[i].cellIndex, trees[i].size);
      cerr << " v " << n.size() << " canSeed " << scores[ME].canSeed() <<" should "<<shouldSeed(trees[i]) << endl;
      if (trees[i].size > 0 and n.size() > 0 and scores[ME].canSeed() and shouldSeed(trees[i])) {
        for(auto p : n) {
          cerr << "  x " << p << " ? " << free[p] <<endl;
        }
        for(auto p : n) {
          cerr << "  c " << p << " ? " << free[p] <<endl;
          if (cells[p].richness > 0 and free[p]) {
            free[p] = false;

            return Action("SEED", trees[i].cellIndex, to_string(p) + " plante");
          }
        }
      }


    }
      return Action();
  }
};
int main() {
  Game theGame;
  theGame.init();
  // game loop
  while (1) {
    theGame.turnInit();
    Action a = theGame.selectBestTree();
    cerr << "my action " << a.s << " " <<a.id<< endl;
    a.play();
  }
}