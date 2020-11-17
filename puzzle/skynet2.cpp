#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <set>
#include <map>
#include <utility>
#include <unordered_map>
#include <unordered_set>
#include <queue>

using namespace std;

struct Graph {
  unordered_map<int, set<int>> edges;
  unordered_map<int, int> gates;


  set<int> neighbors(int id) {
    return edges[id];
  }
  int cost(int id) {
    if (gates.count(id)) {
      return 1 - gates[id];
    }
    return 0;
  }
  int cost1(int i) const {
    return 1;
  }
};

template<typename T, typename priority_t>
struct PriorityQueue {
  typedef std::pair<priority_t, T> PQElement;
  std::priority_queue<PQElement, std::vector<PQElement>,
    std::greater<PQElement>> elements;

  inline bool empty() const {
    return elements.empty();
  }

  inline void put(T item, priority_t priority) {
    elements.emplace(priority, item);
  }

  T get() {
    T best_item = elements.top().second;
    elements.pop();
    return best_item;
  }
};


template<typename Location, typename Graph>
  bool dijkstra
(Graph graph,
 Location start,
 Location goal,
 std::unordered_map<Location, Location>& came_from,
 std::unordered_map<Location, double>& cost_so_far)
{
  PriorityQueue<Location, double> frontier;
  frontier.put(start, 0);

  came_from[start] = start;
  cost_so_far[start] = 0;

  while (!frontier.empty()) {
    Location current = frontier.get();

    if (current == goal) {
      return true;
    }

    for (Location next : graph.neighbors(current)) {
      double new_cost = cost_so_far[current] + graph.cost1(next);
      if (cost_so_far.find(next) == cost_so_far.end()
          || new_cost < cost_so_far[next]) {
        cost_so_far[next] = new_cost;
        came_from[next] = current;
        frontier.put(next, new_cost);
      }
    }
  }
  return false;
}

map<int, int> bfs(Graph graph, int start) {
  std::vector<int> frontier;
  frontier.push_back(start);
  map<int, int> links;
  links[start] = 0;
  int distance(10000);
  int target(0);
  std::unordered_set<int> visited;
  visited.insert(start);

  while (!frontier.empty()) {
    int current = frontier.back();
    frontier.pop_back();
    //if (graph.gates[current]) break;
    for (int next : graph.neighbors(current)) {
      if (visited.find(next) == visited.end()) {
        frontier.push_back(next);
        visited.insert(next);
        links[next] = links[current] + 1;
        if (graph.gates[next] and links[next] < distance) {
          distance = links[next];
          target = next;
        }
      }
    }
  }
  cerr << "found " << target << " to be the nearest with gates " << distance << endl;
  return links;
}



template<typename Location>
std::vector<Location> path(
                           Location start, Location goal,
                           std::unordered_map<Location, Location> came_from
                          ) {
  std::vector<Location> path;
  Location current = goal;
  while (current != start) {
    path.push_back(current);
    current = came_from[current];
  }
  path.push_back(start); // optional
  std::reverse(path.begin(), path.end());
  return path;
}

/**
 * Auto-generated code below aims at helping you parse
 * the standard input according to the problem statement.
 **/
int main()
{
  int N; // the total number of nodes in the level, including the gateways
  int L; // the number of links
  int E; // the number of exit gateways
  cin >> N >> L >> E; cin.ignore();
  Graph graph;
  for (int i = 0; i < L; i++) {
    int N1; // N1 and N2 defines a link between these nodes
    int N2;
    cin >> N1 >> N2; cin.ignore();
    graph.edges[N1].insert(N2);
    graph.edges[N2].insert(N1);
    graph.gates[N1] = 0;
    graph.gates[N2] = 0;
  }
  /* cerr << "edges" << endl;
     for (auto e: graph.edges) {
     cerr << " "<<e.first<<endl;
     for (auto l : e.second) {
     cerr << "  "<<l<<endl;
     }
     }*/
  vector<int> out;
  for (int i = 0; i < E; i++) {
    int EI; // the index of a gateway node
    cin >> EI; cin.ignore();
    out.push_back(EI);
    graph.gates[EI] = graph.edges[EI].size();
  }
  for (auto e : graph.edges) {
    for (auto o : out) {
      if (e.second.count(o)) {
        if (graph.gates.count(e.first))
          ++graph.gates[e.first];
        else
          graph.gates[e.first] = 1;
      }
    }
  }
  int top(0);
  for (auto g : graph.gates) {
    // cerr << "node " << g.first << " gates:" << g.second<<endl;
    top = max(top, g.second);
  }

  // game loop
  while (1) {
    int SI; // The index of the node on which the Skynet agent is positioned this turn
    cin >> SI; cin.ignore();
    int court(0);
    vector<int> res;
    bool init(true);
    auto toto = bfs(graph, SI);
    /*for (auto t : toto) {
      cerr << "Node " << t.first << " dist " << t.second<<endl;
      }*/
    for (auto EI : out) {
      cerr <<SI<< " toward "<<EI<<endl;
      unordered_map<int, int> came_from;
      unordered_map<int, double> cost_so_far;
      auto found = dijkstra(graph, SI, EI, came_from, cost_so_far);
      if (!found) continue;
      // cerr << came_from.size() << endl;
      // cerr << cost_so_far.size() << endl;
      /*for (auto e : came_from) {
        cerr << e.first << " " << e.second << endl;
        }
        for (auto e : cost_so_far) {
        cerr << e.first << " " << e.second << endl;
        }*/
      vector<int> p = path(SI, EI, came_from);
      int score(0);
      if (init) {
        res = p; init = false;}
      string ps(" path: ");
      for (auto i : p ) {
        score += graph.gates[i];
        ps += to_string(i) + ":";
      }
      cerr <<  ps <<endl;
      int total = static_cast<int>(score - p.size());
      // cerr << "OUP " << graph.gates[p[p.size() - 2]] << " reste " << p.size()-2<<" score"<<score<<endl;
      cerr << "  size " << p.size() << " score " <<score << " " <<total<<endl;
      // if (p.size() < court  or (p.size() == court and graph.gates[p[p.size() - 2]] >= top)) {
      // if (p.size() == 2 or graph.gates[p[p.size() - 2]] > p.size() - 2) {
      if (p.size() == 2 or total > court) {
        res = p;
        court = total;
        if (p.size() == 2) break;
      }
    }
    // Write an action using cout. DON'T FORGET THE "<< endl"
    // To debug: cerr << "Debug messages..." << endl;
    int j = res.size();
    string s(to_string(res[j-2]) + " " + to_string(res[j-1]));

    // Example: 0 1 are the indices of the nodes you wish to sever the link between
    cout << s << endl;
    graph.edges[res[j-2]].erase(res[j-1]);
    graph.edges[res[j-1]].erase(res[j-2]);
    graph.gates[res[j-2]]--;
    top = 0;
    for (auto g : graph.gates) {
      cerr << "node " << g.first << " gates:" << g.second<<endl;
      top = max(top, g.second);
    }

    }
    }
