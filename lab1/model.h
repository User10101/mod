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

typedef double Time;

enum Op_result { ERROR = -1, SUCCESS = 0 };

class Event;

class Queue
{
public:
  Queue();
  virtual ~Queue();

  virtual void enque_task(Event *e);
  Event* top_fifo();
  void pop_fifo(Time system_time);

  size_t size();

  std::vector<Time> get_stat();
protected:
  std::list<Event *> queue;
  std::vector<Time> stat;
};

class Sl_queue : public Queue
{
public:
  Sl_queue();
  virtual ~Sl_queue();

  void enque_task(Event *e) override;
};

class CS
{
public:
  CS(unsigned int memory_size, unsigned int cores);
  ~CS();

  class State
  {
  public:
    State(Time t, unsigned int m, unsigned int c)
      : time(t), free_memory(m), free_cores(c) {}

    const Time time;
    const unsigned int free_memory;
    const unsigned int free_cores;
  };

  Op_result mem_alloc(unsigned int size, Time t);
  Op_result mem_free(unsigned int size, Time t);

  Op_result cores_alloc(unsigned int num, Time t);
  Op_result cores_free(unsigned int num, Time t);

  unsigned int memory_available() const;
  unsigned int cores_available() const;

  std::vector<State> get_stat() const;
private:
  std::vector<State> stat;

  unsigned int free_memory;
  unsigned int free_cores;

  const unsigned int memory_size;
  const unsigned int core_num;
};

class Event
{
public:
  Event(Time t, Time et, unsigned int ncores, unsigned int mem,
	std::string n = "Noname");
  virtual ~Event();
  virtual void execute(std::list<Event *> *calendar, Queue *queue,
		       CS *cs, Time *system_time) = 0;

  Time time() const;
  Time time(Time t);

  const Time execution_time;
  const unsigned int n_cores;
  const unsigned int memory;

  const std::string name;
protected:
  Time _time;
};

class Task : public Event
{
public:
  Task(Time stime, Time etime, unsigned int ncores, unsigned int mem);
  ~Task();

  void execute(std::list<Event *> *calendar, Queue *queue,
	       CS *cs, Time *system_time) override;
};

class Cancel_task : public Event
{
public:
  Cancel_task(Time stime, unsigned int ncores, unsigned int mem);
  ~Cancel_task();

  void execute(std::list<Event *> *calendar, Queue *queue,
	       CS *cs, Time *system_time) override;
};

Op_result schedule(Event *e, std::list<Event *> *calendar, Queue *queue,
	       CS *cs, Time *system_time);
Op_result cancel(Event *e, std::list<Event *> *calendar, Queue *queue,
	       CS *cs, Time *system_time);
void simulate(std::list<Event *> *calendar, Queue *queue,
	       CS *cs, Time *system_time);

void dequeue_and_execute(std::list<Event *> *calendar, Queue *queue,
			 CS *cs, Time *system_time);
void print_calendar(std::list<Event *> *calendar);
#endif // _MODEL_INCLUDED_
