## Report for Assignment 5: Parallel Programming with Pthreads ##

### Sequential algorithm and data structures ###

#### Data Structure ####
To implement this simulation program, we created two classes. One is called `Landscape`, which expresses the whole landscape. The class take two fields: an integer to store the dimention, and a 2-D array implemented by a vector of vectors. Each element in this 2-D array is a unit of point.

```c
class Landscape {
  int dimention;
  vector<vector<Point>> grids;

  void initialize(const char *path, int d);
  bool is_all_dry();
}
```

The other class is `Point`, representing the unit to simulate. It includes the elevation, location, the amount of water to absort and trickle.

```c
class Point {
  int x;
  int y;
  int elevation;
  float trickle;
  float absorbed;
  float rest;
  vector<Point *> lower_points;

  void set_lower_points(Point *up, Point *down, Point *left, Point *right);
  void absorb(float rate);
  void do_trickle();
}
```

A very important method of `Landscape` class is `Landscape::initialize()`. It initializes each point in the grid by setting their postion in the grid. Besides, it reads elevation values from the input file and save them to each point. Moreover, it sets their neighbor that will be trickled to based on the elevation by calling method of `Point`. The method accepts its neighbors as references. And the result neighbors are saved as pointers in the `Point` object. In this way, the program does not need to iterare all the neighbors for every step.

In addition to the `set_lower_points()` mentioned above, `Point` class also has the ability to give itself a new rain drop, absorb, calculate amount to trickle and trickle the water to the neighbors that is lower than itself. 

#### Sequential algorithm ####
After reading arguments from the command line, the program initialize the landscape object and all the grids inside. Then a function called `simulation()` is executed, which basically simulate the whole process of raining. Thus, we calculate the running time of this function and finally print the time, total elapsed time steps and absorbed water amount of each point in the landscape.

For the simulation process, in first several time steps, all the points recieve one rain drop, absorb and calculate amount to tricle. Only after all of them complete absorbing, we start trickling operation. Later when rain stops, we only do absorbing and trickling, until all the points become dry.

Since trickling can affect rest water amount of both itself and neighbor points, we iterate the landscape and decide if all the points are dry after all of them finish trickling.

Therefore, you may find that there should at least be three loops over all the landscape. First one rainning and absorbing, the second one trickling and the last one check the dryness.

<br/>

### Parallelization Strategy ###
After profiling, we found that of course, the program spent most of its executing time on `simulation ()` function. So we let the threads each take on one part of the landscape and execute the three loops through out all the time steps. In the parallel version, we shared one landscape object across all threads. Other shared variables include thread number, rain step(M), absorption rate(A), dimension(N), step count(current step), and a boolean variable called `is_all_dry` to determine when the calculation should stop. We also have a `simulation()` function, but this time it would create a number of threads according to command line arguments, and let each of the thread execute a `worker()` function. Each of the thread is given a thread id and passed into the worker function. They will calculate their respective start and end index according to their thread id and only work on [start, end) part of the outer loop. In each time step, all thread would finish the first loop(rain and/or absorb) before continuing to the trickle loop.

#### Comparing to other method ####
In the earlier stage of working on this assignment we used a different parallelization strategy. At first we let the main thread to handle the check dry loop and seperated the rain/absorb loop and the trickle loop and used `pthread_join()` to make sure the second loop is always after all threads had finished executing the first loop. But this means that in every time step, we have to first set up threads to do the first loop, terminate all of them at `pthread_join()`, then set up another set of threads to work on the second loop and terminate them. This route would increase the time the program took to set up and terminate the threads which would add a lot of overhead to the overall executing time. Thus we went for our current method, which would only set up threads once, and used barriers to synchronize. In practice, we saw the execution time roughly cut to half comparing to the previous strategy.

#### Synchronization ####
Several synchronization mechanisms of pthread were used in the parallel version to maintain the correctness of the program.

**pthread_barrier**
As is mentioned in the previous part, barriers were used between the first loop and the second loop to make sure that all threads finish executing the raining and/or absorbing part before proceeding to the trickle. After that we have another two barriers:
```c
pthread_barrier_wait(&barrier);
if (check_dry(start, end) == false) {
  is_all_dry = false;
}
pthread_barrier_wait(&barrier);
```

The first barrier in the above snippet is to make sure all the points in the landscape are finished updating their rain drop amount before checking if their working set of the landscape is dry:
```c
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
```

The last barrier is used to let threads wait until all the work in one time step is finished before going on to the next time step.

**pthread_mutex**
Since all threads are only working on part of the landscape, and doing trickle involves updating rain drop amount of the neighboring points, situations where multiple threads are trying to access and update the same points are bound to happen when the threads are working on points on the edge of their working set. To make sure no race condition would happen, we updated `Point` class and `Landscape` class to record each point's x and y indexes in the landscape. Before going in the three loops, we would first initialize a matrix of mutexes, with each corresponding to a point in the landscape. Before any thread would enter the line of code where the neighbor points of the current point are updated, it would first acquire a mutex lock using the x and y indexes recorded in the neighbor point object. The lock will be released right after the update is complete:
```c
float each = landscape.grids[i][j].trickle /
                       landscape.grids[i][j].trickle_neighbor_num;
for (Point *point : landscape.grids[i][j].lower_points) {
  pthread_mutex_lock(&mutexes[point->x][point->y]);
  point->rest += each;
  pthread_mutex_unlock(&mutexes[point->x][point->y]);
}
```

**pthread_join()**
This is used at the end of the `simulation()` function to terminate all worker threads before the main thread can print out the results.

<br/>

### Performance ###
| Dimention | Sequential | #Thread=1 | #Thread=2 | #Thread=4 | #Thread=8 |
|:---------:| ----------:| ---------:| ---------:| ---------:| ---------:|
|     4     |   0.000018 |   0.001372|   0.006736|   0.027980|   0.036522|
|    16     |   0.000277 |   0.002109|   0.015717|   0.036965|   0.057127|
|    32     |   0.001306 |   0.010222|   0.020560|   0.045276|   0.073217|
|   128     |   0.241277 |   0.610389|   0.378333|   0.351836|   0.482281|
|   512     |   0.516849 |   1.626579|   0.928636|   0.577103|   0.464496|
|   2048    |  54.274821 | 108.917930|  57.821887|  32.273980|  25.043794|
|   4096    | 351.732521 | 673.231015| 351.922579| 203.241073| 146.192630|

<img src="table.png" width="750"/>

<br/>
<br/>

**Sequential Code: Meet requirement**

The runtime of sequential code increases with dimention of the landscape because larger dimention means more calculation for each time step. When dimention reaches 4096, it takes about 6 minutes to complete simulation, which is within the assignment requirement. So we can say that the sequential version is not too slow for following discussion on the effect of parallelism.

**Smaller Dimention: Parallelism doesn't help**

From the result, we can observe that when dimention is ratively small, i.e. smaller than 512, parallelism doesn't help speeding up the program. As more threads are used, the runtime actually increases. It does not match our expectation, because more workers are working on the program and thus parallelism is supposed to speed up the program.

Since each thread only works on part of the landscape, a possible reason is that the workload is too light and takes too little time, while additional work comes with parallelism is relatively expensive. This includes initializing threads, locks for each point, barrier; communication between threads about locks and barrier, etc. The more the threads used, the more extra work is required. It should be clear if you campare the runtime of sequential version and parallel version of only one thread. The latter one can be 2~50 times larger than the former one. Although both code use one thread, overhead of parallelism greatly slow down the whole process.  

**Larger Dimention: more threads, more speed up**

Now we know parallel version of code is not faster than that of the sequential version when dimention is small. But even when dimention is relatively big, i.e. larger than 512, parallel code is still slower if the number of threads is 1 or 2. However, opposite to the case of small dimention, when the number of threads increases, the runtime reduces. This matches our expectation, since more workers are used to solve a problem of same size.

This phenomenon can also be explained by overhead introduced by multithreading. Suppose that the overhead is proportional to the thread number, and the ratio is fixed. Also considering we are paralleling the whole simulation process, the runtime should firstly decreases and then increases as the number of thread increases. 

Now in our experiment, we can observe that when more than 2 threads are used, the program is sped up compared with sequential version. This shows the advantage of parallelism to large program.


