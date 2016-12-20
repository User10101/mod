#ifndef _MODEL_INCLUDED_
#define _MODEL_INCLUDED_

//#define DEBUG_LOG

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

/**
 * Очередь FIFO.
 */
class Queue
{
public:
  Queue();
  virtual ~Queue();

  virtual void enque_task(Event *e);
  virtual Event* try_pop(unsigned int free_cores, unsigned int free_memory,
		 Time system_time);

  size_t size();

  std::vector<Time> get_stat();
protected:
  std::list<Event *> queue;
  std::vector<Time> stat;
};

/**
 * Очередь, из которой самый тяжёлый элемент извлекается последним.
 */
class Sl_queue : public Queue
{
public:
  Sl_queue();
  virtual ~Sl_queue();

  void enque_task(Event *e) override;
};

/**
 * Очередь, из которой самый тяжёлый элемент извлекается первым.
 */
class Sf_queue : public Queue
{
public:
  Sf_queue();
  virtual ~Sf_queue();

  void enque_task(Event *e) override;
};

/**
 * Очередь, из которой первым извлекается самый лёгкий подходящий элемент.
 */
class Sl_queue_opt : public Sl_queue
{
public:
  Sl_queue_opt();
  virtual ~Sl_queue_opt();

  Event* try_pop(unsigned int free_cores, unsigned int free_memory,
		 Time system_time) override;
};


/**
 * Очередь, из которой первым извлекается самый тяжёлый подходящий элемент.
 */
class Sf_queue_opt : public Sl_queue
{
public:
  Sf_queue_opt();
  virtual ~Sf_queue_opt();

  Event* try_pop(unsigned int free_cores, unsigned int free_memory,
		 Time system_time) override;
};

/**
 * Класс реализует модель вычислительной системы. Предоставляет методы для
 * выделения.освобождения ядер и памяти.
 */
class CS
{
public:
  CS(unsigned int memory_size, unsigned int cores);
  ~CS();

  /**
   * Класс для запоминания текущего состояния ВС. Используется для накопления
   * статистики.
   */
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

/**
 * Абстрактный класс, представляющий собой произвольное событие.
 */
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

/**
 * Событие - приход новой задачи в ВС.
 */
class Task : public Event
{
public:
  Task(Time stime, Time etime, unsigned int ncores, unsigned int mem);
  ~Task();

  void execute(std::list<Event *> *calendar, Queue *queue,
	       CS *cs, Time *system_time) override;
};

/**
 * Событие - завершение выполнения задачи в ВС.
 */
class Cancel_task : public Event
{
public:
  Cancel_task(Time stime, unsigned int ncores, unsigned int mem);
  ~Cancel_task();

  void execute(std::list<Event *> *calendar, Queue *queue,
	       CS *cs, Time *system_time) override;
};

/**
 * Событие - постановка задачи на выполнение.
 */
class Execute_task : public Event
{
public:
  Execute_task(Time stime, Time etime, unsigned int ncores, unsigned int mem);
  ~Execute_task();

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
