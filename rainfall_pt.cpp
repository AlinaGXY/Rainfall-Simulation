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
#include <pthread.h>
#include <time.h>

using namespace std;

bool check_dry(int start, int end);
double calc_time(struct timespec start, struct timespec end);
void simulation();
void initMutex();
void *worker(void *arg);

Landscape landscape;
int thread_num;
int rain_step;
float absorption_rate;
int dimention;
int step_count;
bool is_all_dry;

pthread_mutex_t **mutexes;
pthread_barrier_t barrier;

int main(int argc, char **argv) {
  if (argc != 6) {
    cerr << "Usage: ./rainfall <P> <M> <A> <N> <elevation_file>" << endl;
    return EXIT_FAILURE;
  }

  thread_num = atoi(argv[1]);
  rain_step = atoi(argv[2]);
  absorption_rate = atof(argv[3]);
  dimention = atoi(argv[4]);
  landscape.initialize(argv[5], dimention);

  if (dimention / thread_num < 1) {
    thread_num = dimention;
  }

  initMutex();
  pthread_barrier_init(&barrier, NULL, thread_num);

  struct timespec start_time, end_time;
  clock_gettime(CLOCK_MONOTONIC, &start_time);
  // simulation
  simulation();
  clock_gettime(CLOCK_MONOTONIC, &end_time);

  // output
  double elapsed_ns = calc_time(start_time, end_time);
  cout << "Rainfall simulation completed in " << to_string(step_count)
       << " time steps" << endl;
  cout << "Runtime = " << to_string(elapsed_ns * 1e-9) << " seconds" << endl;
  cout << endl;
  cout << "The following grid shows the number of raindrops absorbed at each "
          "point:"
       << endl;
  cout << landscape;

  return EXIT_SUCCESS;
}

void initMutex() {
  mutexes =
      (pthread_mutex_t **)malloc((dimention + 2) * sizeof(pthread_mutex_t *));
  for (int i = 0; i <= dimention + 1; i++) {
    mutexes[i] =
        (pthread_mutex_t *)malloc((dimention + 2) * sizeof(pthread_mutex_t));
  }
  for (int i = 0; i <= dimention + 1; i++) {
    for (int j = 0; j <= dimention + 1; j++) {
      pthread_mutex_init(&mutexes[i][j], NULL);
    }
  }
}

void simulation() {
  step_count = 0;
  is_all_dry = false;
  pthread_t *threads = (pthread_t *)malloc(thread_num * sizeof(pthread_t));
  for (int i = 0; i < thread_num; i++) {
    int *threadId = (int *)malloc(sizeof(int));
    *threadId = i;
    pthread_create(&threads[i], NULL, worker, (void *)threadId);
  }
  for (int i = 0; i < thread_num; i++) {
    pthread_join(threads[i], NULL);
  }
}

void *worker(void *arg) {
  int id = *(int *)arg;
  int start = id * (dimention / thread_num) + 1;
  int end = (id + 1) * (dimention / thread_num) + 1;

  while (!is_all_dry) {
    for (int i = start; i < end; i++) {
      for (int j = 1; j <= landscape.dimention; j++) {
        if (step_count < rain_step) {
          // rain
          landscape.grids[i][j].new_drop();
        }
        // absorb
        landscape.grids[i][j].absorb(absorption_rate);
      }
    }

    pthread_barrier_wait(&barrier);

    for (int i = start; i < end; i++) {
      for (int j = 1; j <= landscape.dimention; j++) {
        // do_trickle
        if (landscape.grids[i][j].trickle_neighbor_num != 0) {
          // update rest water amount of current grid
          landscape.grids[i][j].rest -= landscape.grids[i][j].trickle;

          // trickle to lower neighbor grids
          float each = landscape.grids[i][j].trickle /
                       landscape.grids[i][j].trickle_neighbor_num;
          for (Point *point : landscape.grids[i][j].lower_points) {
            pthread_mutex_lock(&mutexes[point->x][point->y]);
            point->rest += each;
            pthread_mutex_unlock(&mutexes[point->x][point->y]);
          }
        }
      }
    }

    if (id == 0) {
      step_count++;
      is_all_dry = true;
    }

    pthread_barrier_wait(&barrier);

    // check if the working section is all dry
    if (check_dry(start, end) == false) {
      is_all_dry = false;
    }

    pthread_barrier_wait(&barrier);
  }
}

bool check_dry(int start, int end) {
  for (int i = start; i < end; i++) {
    for (int j = 1; j <= landscape.dimention; j++) {
      if (!landscape.grids[i][j].is_dry()) {
        return false;
      }
    }
  }
  return true;
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
