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

/**
 * Auto-generated code below aims at helping you parse
 * the standard input according to the problem statement.
 **/

int main() {
  string L;
  cin >> L;
  cin.ignore();
  int N;
  cin >> N;
  cin.ignore();
  unordered_map<char, unordered_set<string>> dict;
  for (int i = 0; i < N; i++) {
    string W;
    cin >> W;
    char first(W[0]);
    if (dict.count(first)) {
      dict[first].insert(W);
    } else {
      dict[first] = unordered_set({W});
    }
    cin.ignore();
  }

  // for (auto d : dict) cerr << d.first << " vs " << *(d.second.begin()) << endl;

  unordered_map<string, char> letters;
  letters["."] = 'E';
  letters["-"] = 'T';
  letters[".-"] = 'A';
  letters[".."] = 'I';
  letters["--"] = 'M';
  letters["-."] = 'N';
  letters["-.."] = 'D';
  letters["--."] = 'G';
  letters["-.-"] = 'K';
  letters["---"] = 'O';
  letters[".-."] = 'R';
  letters["..."] = 'S';
  letters["..-"] = 'U';
  letters[".--"] = 'W';
  letters["-..."] = 'B';
  letters["-.-."] = 'C';
  letters["..-."] = 'F';
  letters["...."] = 'H';
  letters[".---"] = 'J';
  letters[".-.."] = 'L';
  letters[".--."] = 'P';
  letters["--.-"] = 'Q';
  letters["...-"] = 'V';
  letters["-..-"] = 'X';
  letters["-.--"] = 'Y';
  letters["--.."] = 'Z';

  // Write an action using cout. DON'T FORGET THE "<< endl"
  // To debug: cerr << "Debug messages..." << endl;

  cout << "answer" << endl;
}