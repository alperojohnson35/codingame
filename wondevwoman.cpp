#pragma GCC target("avx") // small optimization
#pragma GCC optimize("O3") // +44k killer optimization with "inline __attribute__ (( always_inline, visibility("protected") ))" and "inline"
#pragma GCC optimize("omit-frame-pointer") // good optimization
#pragma GCC optimize("unsafe-math-optimizations") // not really useful it seems
#pragma GCC optimize("unroll-all-loops") // good optimization
#pragma GCC optimize("inline") // killer optimization with O3

#include <cmath>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string.h>
#include <string>
#include <vector>
#include <list>
#include <map>
#include <algorithm>
#include <chrono>
#include <unistd.h>
#include <set>
#include <random>
#include <bitset>
#include <stdlib.h>
#include <climits>
#include <limits>
#include <queue>
#include <cassert>
#include <array>
#include <deque>
#include <tuple>
#include <unordered_set>


using namespace std;

const signed short GLOBAL_TURN_TIME_MAX = 46;
const short GLOBAL_TURN_TIME_MAX_FIRST_TURN = 900;
const short GLOBAL_MAX_TURN = 200;

struct Timer {
    chrono::time_point<chrono::system_clock> end;
    signed char not_always = 0;

    inline Timer() = default;
    inline Timer(Timer const&) = default;
    inline Timer(Timer&&) = default;
    inline Timer& operator=(Timer const&) = default;
    inline Timer& operator=(Timer&&) = default;

    inline Timer(bool first_turn=false){
        if (first_turn) {
            this->end=chrono::system_clock::now()+chrono::milliseconds(GLOBAL_TURN_TIME_MAX_FIRST_TURN);
        } else {
            this->end=chrono::system_clock::now()+chrono::milliseconds(GLOBAL_TURN_TIME_MAX);
        }
    }
    inline bool isTimesUp(){
        return std::chrono::system_clock::now() > this->end;
    }
};


int main()
{
    int size;
    cin >> size; cin.ignore();
    SquareGrid g(size, size);
    vector<string> actions;

    int unitsPerPlayer;
    theBoard.mainInit();
    cin >> unitsPerPlayer; cin.ignore();

    // game loop
    while (1) {
      theBoard.turnInit();
        for (int i = 0; i < size; i++) {
            string row;
            cin >> row; cin.ignore();
        }
        for (int i = 0; i < unitsPerPlayer; i++) {
            int unitX;
            int unitY;
            cin >> unitX >> unitY; cin.ignore();
        }
        for (int i = 0; i < unitsPerPlayer; i++) {
            int otherX;
            int otherY;
            cin >> otherX >> otherY; cin.ignore();
        }
        int legalActions;
        cin >> legalActions; cin.ignore();
        for (int i = 0; i < legalActions; i++) {
            string atype;
            int index;
            string dir1;
            string dir2;
            cin >> atype >> index >> dir1 >> dir2; cin.ignore();
            actions.push_back(atype + " " + to_string(index) + " " + dir1 + " " + dir2);
            cerr  << actions[i] << endl;
        }

        // Write an action using cout. DON'T FORGET THE "<< endl"
        // To debug: cerr << "Debug messages..." << endl;

        cout << actions[rand() % legalActions] << endl;
    }
}
