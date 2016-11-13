#include "model.h"

Queue::Queue()
{
  // TODO
}

Queue::~Queue()
{
  // TODO
}

void Queue::enque_task(Event *e)
{
  if (e == nullptr) {
    throw std::invalid_argument("Cannot enqueue empty event.");
  }
  queue.push_back(e);
}

Event* Queue::top_fifo()
{
  if (queue.size() == 0) {
    return nullptr;
  }

  return queue.front();
}

void Queue::pop_fifo(Time system_time)
{
  if (queue.size() != 0) {
    stat.push_back(system_time - (queue.front())->time());
    queue.pop_front();
  }
}

Op_result Queue::deque_sl()
{
  // TODO
}

Op_result Queue::dequeq_sf()
{
  // TODO
}

std::vector<Time> Queue::get_stat()
{
  return stat;
}

size_t Queue::size()
{
  return queue.size();
}

CS::CS(unsigned int memory, unsigned int cores)
  : memory_size(memory), core_num(cores)
{
  if (0 == memory_size) {
    throw std::invalid_argument("Memory size must be greater than zero.");
  }
  if (0 == core_num) {
    throw std::invalid_argument("Cores count must be greater than zero.");
  }

  free_memory = memory_size;
  free_cores = core_num;

  stat.emplace_back(0., free_memory, free_cores);
}

CS::~CS()
{
  // DO NOTHING
}

Op_result CS::mem_alloc(unsigned int size, Time t)
{
  if (free_memory >= size) {
    free_memory -= size;
    stat.emplace_back(t, free_memory, free_cores);
    return SUCCESS;
  }

  return ERROR;
}

Op_result CS::mem_free(unsigned int size, Time t)
{
  if (free_memory + size <= memory_size) {
    free_memory += size;
    stat.emplace_back(t, free_memory, free_cores);
    return SUCCESS;
  }

  return ERROR;
}

Op_result CS::cores_alloc(unsigned int num, Time t)
{
  if (free_cores >= num) {
    free_cores -= num;
    stat.emplace_back(t, free_memory, free_cores);
    return SUCCESS;
  }

  return ERROR;
}

Op_result CS::cores_free(unsigned int num, Time t)
{
  if (free_cores + num <= core_num) {
    free_cores += num;
    stat.emplace_back(t, free_memory, free_cores);
    return SUCCESS;
  }

  return ERROR;
}

unsigned int CS::memory_available() const
{
  return free_memory;
}

unsigned int CS::cores_available() const
{
  return free_cores;
}

std::vector<CS::State> CS::get_stat() const
{
  return stat;
}

Event::Event(Time t, Time et, unsigned int ncores, unsigned int mem,
	     std::string n)
  : _time(t), execution_time(et), n_cores(ncores), memory(mem), name(n)
{
  // DO NOTHING
}

Event::~Event()
{
  // DO NOTHING
}

Time Event::time() const
{
  return _time;
}

Time Event::time(Time t)
{
  if (t < 0) {
    throw std::invalid_argument("Cannot set negative time.");
  }

  _time = t;
}

Task::Task(Time stime, Time etime, unsigned int ncores, unsigned int mem)
  : Event(stime, etime, ncores, mem, "Task")
{
  if (0 == memory) {
    throw std::invalid_argument("Memory size must be greater than zero.");
  }
  if (0 == n_cores) {
    throw std::invalid_argument("Cores count must be greater than zero.");
  }
}

Task::~Task()
{
  // DO NOTHING
}

void Task::execute(std::list<Event *> *calendar, Queue *queue,
	       CS *cs, Time *system_time)
{
  std::cout << "Executing Task with " << memory << " memory and " << n_cores <<
    " cores\n";
  queue->enque_task(this);
  dequeue_and_execute(calendar, queue, cs, system_time);
}

Cancel_task::Cancel_task(Time stime, unsigned int ncores, unsigned int mem)
  : Event(stime, 0, ncores, mem, "Cancel task")
{
  // DO NOTHING
}

Cancel_task::~Cancel_task()
{
  // DO_NOTHING
}

void Cancel_task::execute(std::list<Event *> *calendar, Queue *queue,
			  CS *cs, Time *system_time)
{
  std::cout << "Executing Cancel_task with " << memory << " memory and "
	    << n_cores << " cores\n";
  cs->cores_free(n_cores, *system_time);
  cs->mem_free(memory, *system_time);
  dequeue_and_execute(calendar, queue, cs, system_time);
}

Op_result schedule(Event *e, std::list<Event *> *calendar, Queue *queue,
	       CS *cs, Time *system_time)
{
  if (e->time() < *system_time) {
    return ERROR;
  }

  for (auto iter = calendar->begin(); iter != calendar->end(); ++iter) {
    if ((*iter)->time() > e->time()) {
      calendar->insert(iter, e);
      return SUCCESS;
    }
  }

  calendar->push_back(e);
  return SUCCESS;
}

Op_result cancel(Event *e, std::list<Event *> *calendar, Queue *queue,
	       CS *cs, Time *system_time)
{

}

void simulate(std::list<Event *> *calendar, Queue *queue,
	       CS *cs, Time *system_time)
{
  // for (auto event : calendar) {
  //   event->execute(calendar, queue, cs, system_time);
  //   calendar.remove(event);
  // }
  for (auto iter = calendar->begin(); iter != calendar->end();
       iter = calendar->begin()) {
    Event *e = *iter;
    calendar->remove(*iter);
    *system_time = e->time();
    e->execute(calendar, queue, cs, system_time);
  }
}

void dequeue_and_execute(std::list<Event *> *calendar, Queue *queue,
			 CS *cs, Time *system_time)
{
  while (queue->size() != 0) {
    Event *e = queue->top_fifo();
    if (e->memory <= cs->memory_available() &&
	e->n_cores <= cs->cores_available()) {
      queue->pop_fifo(*system_time);
      e->time(*system_time);
      cs->mem_alloc(e->memory, *system_time);
      cs->cores_alloc(e->n_cores, *system_time);
      Event *ct = new Cancel_task(*system_time + e->execution_time,
				  e->n_cores, e->memory);
      delete e;
      schedule(ct, calendar, queue, cs, system_time);
    } else {
      break;
    }
  }
}

void print_calendar(std::list<Event *> *calendar)
{
  for (auto iter = calendar->rbegin(); iter != calendar->rend(); ++iter) {
    std::cout << (*iter)->name << ' ';
  }
  std::cout << std::endl;
}
