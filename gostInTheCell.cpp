#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <stdint.h>
#include <unordered_map>
#include <vector>
#include <map>

using namespace std;

/**
 * Auto-generated code below aims at helping you parse
 * the standard input according to the problem statement.
 **/

typedef enum {MOI, LUI, NEUTRE} type_t;

class Usine {
  int owner;
  vector<int> targets;
  bool canProduce;
public:
  int nbCyborg, troopToGo, production;
  map<int, int> distances;
  string action;
  double average;
  static int bombs;

  Usine() : owner(0), nbCyborg(0), troopToGo(0), production(0), average(0.0), canProduce(true) {}
  string affiche () {
    string dist;
    for (map<int,int>::iterator it = distances.begin(); it!=distances.end();++it) {
      if (it->second > 0) dist += "u" + to_string(it->second) + ":" + to_string(it->first);
    }
    dist = "[" + dist + "]";
    return "Usine, owner" + to_string(owner) + "";
  }
  void setDistance(int factory, int dist) {
    distances[factory] = dist;
  }
  bool isMine() {
    return owner == 1;
  }
  type_t getType() {
    if (owner == 1) return MOI;
    else if (owner == -1) return LUI;
    else return NEUTRE;
  }
  void setOwner(type_t type) {
    switch(type) {
      case LUI:
        owner = -1;
        break;
      case MOI:
        owner = 1;
        break;
    }
  }
  void setup(int o, int c, int p, int x, int y) {
    owner = o;
    nbCyborg = c;
    production = p;
    troopToGo = 0;
    action.clear();
    targets.clear();
    canProduce = (x == 0);
  }
  void computeAverage() {
    for (int i(0); i< distances.size(); ++i) {
      average += distances[i];
    }
    average /= distances.size()-1;
  }
  vector<int> getOtherFactories() {
    vector<int> out;
    for (map<int,int>::iterator it = distances.begin(); it!=distances.end();++it) {
      if (find(targets.begin(), targets.end(), it->first) == targets.end()
          && it->second > 0)
        out.push_back(it->first);
    }
    return out;
  }
  double getRatio (int f) {
    //cerr << "...In get ratio " << production << " " + to_string(average) << " "  << distances[f]<< " " << nbCyborg <<endl;
    return static_cast<double>(10 - average + 1.5*production)/(distances[f]);
  }
  bool canSend () {
    return troopToGo < nbCyborg/2;
  }
  void addAction (int a, int b, int c, bool d) {
    troopToGo += c;
    targets.push_back(b);
    if (c > 30 && bombs > 0 && d) {
      action += "BOMB "+ to_string(a) + " " + to_string(b)+";";
      bombs--;
    } else {
      action += "MOVE " + to_string(a) + " " + to_string(b) + " " + to_string(c) + ";";
    }
  }
  int getGenerated(int source, int dist) {
    if (getType() == NEUTRE) return nbCyborg+1;
    return nbCyborg + production * (1 + distances[source] - dist);
  }
  int getProduction() {
    if (getType() == NEUTRE) return 0;
    return production;
  }

};
int Usine::bombs = 2;
class Troop {
  int owner;
  int nbCyborg;
public:
  int origin, destination, timeToReach;
  bool isBomb;

  Troop(int a, int b, int c, int d, int e, int f) : owner(a), nbCyborg(d), origin(b), destination(c), timeToReach(e), isBomb(f) {}

  string affiche () {
    string own(" MOI");
    if (!isMine()) own=" LUI";
    return own +":Troupe: "+to_string(nbCyborg)+"mecs de "+to_string(origin)+" vers "+to_string(destination)
        +" en "+to_string(timeToReach)+" tours";
  }
  int getNbOut(type_t type) {
    int out;
    switch(type) {
      case MOI:
        out = owner * nbCyborg;
        break;
      case LUI:
        out = -1  * owner * nbCyborg;
        break;
      case NEUTRE:
        out = -1 * nbCyborg;
        break;
    }
    return out;
  }
  int getFighters() {return owner * nbCyborg;}
  bool isMine() {return owner == 1;}
};
struct closeTroop {
  inline bool operator() (const Troop& struct1, const Troop& struct2) {
    return (struct1.timeToReach < struct2.timeToReach);
  }
};


int nbTroopToSend(vector<Troop>& lesTroupes,
                  vector<int>& attackingList,
                  Usine usine, int source) {
  int troopId(0);
  int lastTroopTurn(0);
  if (lesTroupes.size() > 0 && attackingList.size() > 0) {
    lastTroopTurn = lesTroupes[attackingList[attackingList.size()-1]].timeToReach;
  }
  lastTroopTurn = min(lastTroopTurn, usine.distances[source]);
  Usine localUsine(usine);
  // pour chaque tour de jeu
  for (int i(1); i<=lastTroopTurn; ++i) {
    // incrémente la production de l'usine
    localUsine.nbCyborg += localUsine.getProduction();
    if (lesTroupes[attackingList[troopId]].timeToReach == i) {
      int fight = 0;
      cerr <<  to_string(attackingList[troopId])+"is comming"<< endl;
      while (troopId < attackingList.size() && lesTroupes[attackingList[troopId]].timeToReach == i) {
        //cerr << troopId <<"xx" + to_string(attackingList[troopId])<< endl;
        fight += lesTroupes[attackingList[troopId]].getFighters();
        troopId++;
      }
      switch (localUsine.getType()) {
        case MOI:
          localUsine.nbCyborg += fight;
          if (localUsine.nbCyborg < 0) localUsine.setOwner(LUI);
          break;
        case LUI:
          localUsine.nbCyborg -= fight;
          if (localUsine.nbCyborg < 0) localUsine.setOwner(MOI);
          break;
        case NEUTRE:
          localUsine.nbCyborg -= abs(fight);
          if (localUsine.nbCyborg < 0) localUsine.setOwner(fight < 0 ? LUI : MOI);
          break;
      }
      if (troopId > attackingList.size()) break;
    }
    localUsine.nbCyborg = abs(localUsine.nbCyborg);
  }
  cerr <<"   "+ to_string(localUsine.nbCyborg)<< ".."<<localUsine.isMine()<< endl;
  if (localUsine.isMine()) {
    return 0;
  } else {
    return localUsine.getGenerated(source, lastTroopTurn);
  }
}

int defendTroops(vector<Troop>& lesTroupes,
                 vector<int>& attackingList,
                 Usine usine) {
  int nbTroupes(0), troopId(0);
  int troopTurn(0);

  while (troopId < attackingList.size()) {
    if (lesTroupes[attackingList[troopId]].isMine()) continue;
    troopTurn = lesTroupes[attackingList[troopId]].timeToReach - troopTurn;
    nbTroupes += usine.production * troopTurn;
    cerr << "NB TROOP "+to_string(nbTroupes)+" TURN " + to_string(troopTurn)<< endl;
    if (nbTroupes > lesTroupes[attackingList[troopId]].getNbOut(LUI)) {
      nbTroupes -= lesTroupes[attackingList[troopId]].getNbOut(LUI);
    } else {
      nbTroupes = 0;
      break;
    }
    troopId++;
  }
  return nbTroupes;
}


int main()
{
  vector<Usine> lesUsines;
  vector<Troop> lesTroupes;
  map<int, vector<int> > attacking;

  int factoryCount, usine1(0);
  cin >> factoryCount; cin.ignore();
  // Allocation of usines to set up their links
  for (int i(0); i<factoryCount; ++i) {
    lesUsines.push_back(Usine());
  }
  int linkCount;
  cin >> linkCount; cin.ignore();
  // Link set-up
  for (int i = 0; i < linkCount; i++) {
    int factory1, factory2, distance;
    cin >> factory1 >> factory2 >> distance; cin.ignore();
    lesUsines[factory1].setDistance(factory2, distance);
    lesUsines[factory2].setDistance(factory1, distance);
    // cerr << factory1 << ";"<< factory2 << "-"<< distance << endl;
  }
  for (int i(0); i<factoryCount; ++i) {
    lesUsines[i].computeAverage();
  }

  // game loop
  while (1) {

    attacking.clear();
    lesTroupes.clear();
    int max = 0;
    // Round init
    int entityCount;
    cin >> entityCount; cin.ignore();
    for (int i = 0; i < entityCount; ++i) {
      int entityId;
      string entityType;
      int arg1, arg2, arg3, arg4, arg5;
      cin >> entityId >> entityType >> arg1 >> arg2 >> arg3 >> arg4 >> arg5; cin.ignore();
      // cerr << "Id "<<entityId<<" type " << entityType << ":"<< arg1 <<"-"<< arg2 <<"-"<< arg3<<"-" << arg4<<"-" << arg5<<endl;
      if (entityType == "FACTORY") {
        lesUsines[entityId].setup(arg1, arg2, arg3, arg4, arg5);
        if (arg2 > max && arg1 == 1) {
          max = arg2;
          usine1 = entityId;
        }
        // cerr << lesUsines[entityId].affiche() << endl;
      } else if (entityType == "TROOP") {
        lesTroupes.push_back(Troop(arg1, arg2, arg3, arg4, arg5, false));
        cerr << "Id" + to_string(entityId) + lesTroupes[lesTroupes.size() - 1].affiche() << endl;
      } else {
        if (arg3 == -1) arg3 = usine1;
        if (arg4 == -1) arg5 = lesUsines[usine1].distances[arg2];
        else arg5 = arg4;
        arg4 = min(10, lesUsines[arg3].nbCyborg/2);
        lesTroupes.push_back(Troop(arg1, arg2, arg3, arg4, arg5, true));
        cerr << "Bm" + to_string(entityId) + lesTroupes[lesTroupes.size() - 1].affiche() << endl;
      }
    }

    // Map of troops, sorted by timetoreach
    sort(lesTroupes.begin(), lesTroupes.end(), closeTroop());
    for (int i(0); i < lesTroupes.size(); ++i) {
      //cerr << "troupe " +to_string(i) +" goes to "+to_string(lesTroupes[i].destination)+" from " + to_string(lesTroupes[i].origin)<<endl;
      attacking[lesTroupes[i].destination].push_back(i);
    }

    // select most suitable usine for action
    for (int i(0); i<factoryCount; ++i) {
      if (lesUsines[i].isMine()) {
        cerr << "Mon Usine " + to_string(i) << endl;
        cerr << " poids:";
        map<double, int> poids;
        for (auto other : lesUsines[i].getOtherFactories()) {
          cerr << to_string(other)+";" + to_string(lesUsines[other].getRatio(i)) + "|";
          poids[lesUsines[other].getRatio(i)] = other;
        }
        cerr << endl;
//        cerr << " defense " + to_string(defendTroops(lesTroupes, attacking[i], lesUsines[i])) << endl;;
        for (map<double, int>::reverse_iterator it = poids.rbegin(); it != poids.rend(); ++it) {
          if (lesUsines[it->second].isMine()) continue;
//          if (lesUsines[it->second].isMine() || lesUsines[it->second].production == 0) continue;
          cerr << "  vers " + to_string(it->second) << endl;
          int toSend = nbTroopToSend(lesTroupes, attacking[it->second], lesUsines[it->second], i);
          if (toSend <= 0) continue;
          // can add wait or bomb here
          bool canBomb = true;
          for (auto troup : lesTroupes) {
            if (troup.isBomb && troup.destination == it->second) {
                canBomb = false;
                break;
            }
          }
          lesUsines[i].addAction(i, it->second, toSend, canBomb);
          cerr << "  envoie "  + to_string(toSend)<< endl;
          if (lesUsines[i].troopToGo > lesUsines[i].nbCyborg) break;
        }
        cerr << " Total troops to go " + to_string(lesUsines[i].troopToGo) <<endl;
      }
    }
    string final("");
    for (auto usine : lesUsines) {
      if (usine.isMine()) {
        final += usine.action;
      }
    }
    if (final[0] == '\0') final="WAIT;";
    final.pop_back();

    cout << final << endl;
  }
}
