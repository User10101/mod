#ifndef _MODEL_INCLUDED_
#define _MODEL_INCLUDED_

#include <fstream>
#include <iostream>
#include <cstdlib>
#include <vector>
#include <cmath>
#include <set>
#include <cstdlib>
#include <list>
#include <string>
#include <iomanip>
#include <cstring>

#define MEMORY_SIZE 1024*1024*1024
#define CORES_COUNT 25
#define QUEUE_SIZE 300

typedef double Time;

enum Op_result { ERROR = -1, SUCCESS = 0 };

class Queue
{
public:
  Queue(size_t queue_size);
  ~Queue();

  Op_result enque_task(unsigned int ncores, unsigned int memory,
			    Time system_time);
  Op_result top_fifo(unsigned int *ncores, unsigned int *memory);
  void pop_fifo();
  Op_result deque_sl();
  Op_result dequeq_sf();

  size_t size();

  std::vector<Time> get_stat();
private:
  struct Q_item
  {
    Q_item(unsigned int m, unsigned int n, Time t)
      : memory(m), ncores(n), enque_time(t) {}

    unsigned int memory;
    unsigned int ncores;
    Time enque_time;
  };

  std::list<Q_item> queue;
  const size_t max_size;
  std::vector<double> stat;
};

class CS
{
public:
  CS(unsigned int memory_size, unsigned int cores);
  ~CS();

  Op_result mem_alloc(unsigned int size);
  Op_result mem_free(unsigned int size);

  Op_result cores_alloc(unsigned int num);
  Op_result cores_free(unsigned int num);

  unsigned int memory_available() const;
  unsigned int cores_available() const;
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
  virtual void execute(std::list<Event *> *calendar, Queue *queue,
		       CS *cs, Time *system_time) = 0;

  const Time time;
  const std::string name;
};

class Task : public Event
{
public:
  Task(Time stime, Time etime, unsigned int ncores, unsigned int mem);
  ~Task();

  void execute(std::list<Event *> *calendar, Queue *queue,
	       CS *cs, Time *system_time) override;

  const Time execution_time;
  const unsigned int n_cores;
  const unsigned int memory;
};

class Cancel_task : public Event
{
public:
  Cancel_task(Time stime, unsigned int ncores, unsigned int mem);
  ~Cancel_task();

  void execute(std::list<Event *> *calendar, Queue *queue,
	       CS *cs, Time *system_time) override;

  const unsigned int n_cores;
  const unsigned int memory;
};

Op_result schedule(Event *e, std::list<Event *> *calendar, Queue *queue,
	       CS *cs, Time *system_time);
Op_result cancel(Event *e, std::list<Event *> *calendar, Queue *queue,
	       CS *cs, Time *system_time);
void simulate(std::list<Event *> *calendar, Queue *queue,
	       CS *cs, Time *system_time);

void print_calendar(std::list<Event *> *calendar);
#endif // _MODEL_INCLUDED_
