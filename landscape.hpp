#include <climits>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

#include "point.hpp"

class Landscape {
public:
  int dimention;
  vector<vector<Point>> grids;

  Landscape() {}
  Landscape(int d) : dimention(d), grids(d + 2, vector<Point>(d + 2)) {}

  // set elevation of each point
  // decide the directions to trickle of each point
  void initialize(const char *path, int d) {
    this->dimention = d;
    this->grids.resize(d + 2, vector<Point>(d + 2));
    ifstream f;
    f.open(path);
    if (f.fail()) {
      cerr << "cannot open file" << endl;
      exit(EXIT_FAILURE);
    }
    for (int i = 0; i <= dimention + 1; i++) {
      for (int j = 0; j <= dimention + 1; j++) {
        grids[i][j].x = i;
        grids[i][j].y = j;
      }
    }

    for (int i = 0; i <= dimention + 1; i++) {
      grids[0][i].elevation = INT_MAX;
    }

    for (int i = 1; i <= dimention; i++) {
      string curr;
      getline(f, curr);
      stringstream ss(curr);
      grids[i][0].elevation = INT_MAX;
      for (int j = 1; j <= dimention; j++) {
        ss >> grids[i][j].elevation;
      }
      grids[i][dimention + 1].elevation = INT_MAX;
    }

    for (int i = 0; i <= dimention + 1; i++) {
      grids[dimention + 1][i].elevation = INT_MAX;
    }

    for (int i = 1; i <= dimention; i++) {
      for (int j = 1; j <= dimention; j++) {
        grids[i][j].set_lower_points(&grids[i - 1][j], &grids[i + 1][j],
                                     &grids[i][j - 1], &grids[i][j + 1]);
      }
    }
  }

  bool is_all_dry() {
    for (int i = 1; i <= dimention; i++) {
      for (int j = 1; j <= dimention; j++) {
        if (!grids[i][j].is_dry()) {
          return false;
        }
      }
    }

    return true;
  }

  friend ostream &operator<<(ostream &os, const Landscape &landscape) {
    for (int i = 1; i <= landscape.dimention; i++) {
      for (int j = 1; j <= landscape.dimention; j++) {
        os << right << setw(8) << landscape.grids[i][j].absorbed;
      }
      os << endl;
    }
  }
};