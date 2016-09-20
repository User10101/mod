#include "gnuplot-iostream.h"

#include <iostream>
#include <cstdlib>
#include <vector>
#include <cmath>
#include <set>

typedef unsigned short Time;

class Task
{
public:
  Task();
  ~Task();
private:
  Time time;
  unsigned short n_cores;
  unsigned int memory;
};

enum Op_result { ERROR = -1, SUCCESS = 0 };

class CS
{
public:
  CS(unsigned int memory_size, unsigned int cores);
  ~CS();

  Op_result mem_alloc(unsigned int size);
  Op_result mem_free(unsigned int size);

  Op_result cores_alloc(unsigned int num);
  Op_result cores_free(unsigned int num);
private:
  unsigned int free_memory;
  unsigned int free_cores;

  const unsigned int memory_size;
  const unsigned int core_num;
};
