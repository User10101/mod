#include <fstream>
#include <iostream>
#include <cstdlib>
#include <vector>
#include <cmath>
#include <set>
#include <cstdlib>
#include <list>
#include <string>

typedef double Time;

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

class Event
{
public:
  Event(Time t, std::string n = "Noname");
  virtual ~Event();
  virtual void execute() = 0;

  const Time time;
  const std::string name;
};

Event::Event(Time t, std::string n)
  : time(t), name(n)
{
  // DO NOTHING
}

Event::~Event()
{
  // DO NOTHING
}

std::list<Event *> calendar;
Time system_time = 0;

Op_result schedule(Event *e, Time t);
Op_result cancel(Event *e, Time t);
void simulate();

class Task : public Event
{
public:
  Task(Time stime, Time etime, unsigned short ncores, unsigned short mem);
  ~Task();

  void execute() override;

  const Time execution_time;
  const unsigned short n_cores;
  const unsigned int memory;
};

class Cancel_task : public Event
{
public:
  Cancel_task(Time stime, unsigned short ncores, unsigned short mem);
  ~Cancel_task();

  void execute() override;

  const unsigned short n_cores;
  const unsigned int memory;
};

void generate_task_list();

class Queue
{
public:
  
};
