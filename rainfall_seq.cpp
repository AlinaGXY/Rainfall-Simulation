/*
1. Traverse over all landscape points
◦ 1) Receive a new raindrop (if it is still raining) for each point.
◦ 2) If there are raindrops on a point, absorb water into the point
◦ 3a) Calculate the number of raindrops that will next trickle to the lowest
neighbor(s)
2. Make a second traversal over all landscape points
◦ 3b) For each point, use the calculated number of raindrops that will trickle
to the lowest neighbor(s) to update the number of raindrops at each lowest
neighbor, if applicable.
*/

#include "landscape.hpp"
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <time.h>

using namespace std;

double calc_time(struct timespec start, struct timespec end);
int simulation(Landscape &landscape, float absorption_rate, int rain_step);

int main(int argc, char **argv) {
  if (argc != 6) {
    cerr << "Usage: ./rainfall <P> <M> <A> <N> <elevation_file>" << endl;
    return EXIT_FAILURE;
  }

  int thread_num = atoi(argv[1]);
  int rain_step = atoi(argv[2]);
  float absorption_rate = atof(argv[3]);
  int dimention = atoi(argv[4]);
  Landscape landscape(dimention);
  landscape.initialize(argv[5], dimention);

  struct timespec start_time, end_time;
  clock_gettime(CLOCK_MONOTONIC, &start_time);
  // simulation
  int steps = simulation(landscape, absorption_rate, rain_step);
  clock_gettime(CLOCK_MONOTONIC, &end_time);

  // output
  double elapsed_ns = calc_time(start_time, end_time);
  cout << "Rainfall simulation completed in " << to_string(steps)
       << " time steps" << endl;
  cout << "Runtime = " << to_string(elapsed_ns * 1e-9) << " seconds" << endl;
  cout << endl;
  cout << "The following grid shows the number of raindrops absorbed at each "
          "point:"
       << endl;
  cout << landscape;

  return EXIT_SUCCESS;
}


int simulation(Landscape &landscape, float absorption_rate, int rain_step) {
  int step = 0;
  for (; step < rain_step; step++) {
    for (int i = 1; i <= landscape.dimention; i++) {
      for (int j = 1; j <= landscape.dimention; j++) {
        landscape.grids[i][j].new_drop();
        landscape.grids[i][j].absorb(absorption_rate);
      }
    }

    for (int i = 1; i <= landscape.dimention; i++) {
      for (int j = 1; j <= landscape.dimention; j++) {
        landscape.grids[i][j].do_trickle();
      }
    }
  }

  while (!landscape.is_all_dry()) {
    step++;

    for (int i = 1; i <= landscape.dimention; i++) {
      for (int j = 1; j <= landscape.dimention; j++) {
        landscape.grids[i][j].absorb(absorption_rate);
      }
    }

    for (int i = 1; i <= landscape.dimention; i++) {
      for (int j = 1; j <= landscape.dimention; j++) {
        landscape.grids[i][j].do_trickle();
      }
    }
  }

  return step;
}



double calc_time(struct timespec start, struct timespec end) {
  double start_sec =
      (double)start.tv_sec * 1000000000.0 + (double)start.tv_nsec;
  double end_sec = (double)end.tv_sec * 1000000000.0 + (double)end.tv_nsec;

  if (end_sec < start_sec) {
    return 0;
  } else {
    return end_sec - start_sec;
  }
}