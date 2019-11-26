#include <algorithm>

class Point {
public:
  int elevation;
  float trickle;
  float absorbed;
  vector<Point *> lower_points;
  int trickle_neighbor_num;
  float rest;
  int x;
  int y;
  Point() : trickle(0), rest(0), absorbed(0), x(0), y(0) {}

  void set_lower_points(Point *up, Point *down, Point *left, Point *right) {
    // stop if it's the lowest point itself
    if (elevation <= up->elevation && elevation <= down->elevation &&
        elevation <= left->elevation && elevation <= right->elevation) {
      return;
    }

    vector<Point *> points;
    points.push_back(up);
    points.push_back(down);
    points.push_back(left);
    points.push_back(right);

    // sort neighbor points in ascending order
    sort(points.begin(), points.end(),
         [](const Point *a, const Point *b) -> bool {
           return a->elevation < b->elevation;
         });

    int num;
    // find all minimal elevation neighbors
    for (num = 1; num < 4; num++) {
      if (points[num]->elevation > points[num - 1]->elevation) {
        break;
      }
    }
    // record number of neighbors rain can trickle to
    trickle_neighbor_num = num;
    for (int i = 0; i < num; i++) {
      lower_points.push_back(points[i]);
    }
  }

  void new_drop() {
    rest += 1; // new rain drop
  }

  // update rest raindrop on the point
  void absorb(float rate) {
    float value = rest > rate ? rate : rest;
    absorbed += value;
    rest -= value;                 // absorb
    trickle = rest > 1 ? 1 : rest; // calculate rain drops to trickle
  }

  void do_trickle() {
    if (lower_points.size() != 0) {
      rest -= trickle;
      float each = trickle / trickle_neighbor_num;
      for (Point *point : lower_points) {
        point->rest += each;
      }
    }
  }

  bool is_dry() { return rest == 0; }
};
