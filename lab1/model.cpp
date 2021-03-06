#include "model.h"

Queue::Queue()
{
  // DO NOTHING
}

Queue::~Queue()
{
  // DO NOTHING
}

void Queue::enque_task(Event *e)
{
  if (e == nullptr) {
    throw std::invalid_argument("Cannot enqueue empty event.");
  }
  queue.push_back(e);
}

Event* Queue::try_pop(unsigned int free_cores, unsigned int free_memory,
		       Time system_time)
{
  auto iter = *(queue.begin());
  if (iter->memory <= free_memory && iter->n_cores <= free_cores) {
    Event *e = iter;
    queue.remove(e);
    stat.push_back(system_time - e->time());
    return e;
  }

  return nullptr;

}

std::vector<Time> Queue::get_stat()
{
  return stat;
}

size_t Queue::size()
{
  return queue.size();
}

Sl_queue::Sl_queue()
{
  // DO NOTHING
}

Sl_queue::~Sl_queue()
{
  // DO NOTHING
}

void Sl_queue::enque_task(Event *e)
{
  for (std::list<Event *>::iterator iter = queue.begin(); iter != queue.end();
       ++iter) {
    if (e->execution_time < (*iter)->execution_time) {
      queue.insert(iter, e);
      return;
    }
  }

  queue.push_back(e);
}

Sf_queue::Sf_queue()
{
  // DO NOTHING
}

Sf_queue::~Sf_queue()
{
  // DO NOTHING
}

void Sf_queue::enque_task(Event *e)
{
  for (std::list<Event *>::iterator iter = queue.begin(); iter != queue.end();
       ++iter) {
    if (e->execution_time > (*iter)->execution_time) {
      queue.insert(iter, e);
      return;
    }
  }

  queue.push_back(e);
}

Sl_queue_opt::Sl_queue_opt()
{
  // DO NOTHING
}

Sl_queue_opt::~Sl_queue_opt()
{
  // DO NOTHING
}

Event* Sl_queue_opt::try_pop(unsigned int free_cores, unsigned int free_memory,
		 Time system_time)
{
  for (auto iter : queue) {
    if (iter->memory <= free_memory && iter->n_cores <= free_cores) {
      Event *e = iter;
      queue.remove(e);
      stat.push_back(system_time - e->time());
      return e;
    }
  }

  return nullptr;
}

Sf_queue_opt::Sf_queue_opt()
{
  // DO NOTHING
}

Sf_queue_opt::~Sf_queue_opt()
{
  // DO NOTHING
}

Event* Sf_queue_opt::try_pop(unsigned int free_cores, unsigned int free_memory,
		 Time system_time)
{
  for (auto iter : queue) {
    if (iter->memory <= free_memory && iter->n_cores <= free_cores) {
      Event *e = iter;
      queue.remove(e);
      stat.push_back(system_time - e->time());
      return e;
    }
  }

  return nullptr;
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
#ifdef DEBUG_LOG
  std::cout << "Executing Task with " << memory << " memory and " << n_cores <<
    " cores\n";
#endif

  queue->enque_task(this);
  unsigned int new_busy_cores = 0;
  unsigned int new_busy_memory = 0;
  while (queue->size() != 0) {
    Event *e = queue->try_pop(cs->cores_available() - new_busy_cores, cs->memory_available() - new_busy_memory,
			      *system_time);
    if (e != nullptr) {
      if (schedule(new Execute_task(*system_time, e->execution_time,
				e->n_cores, e->memory), calendar, queue,
		   cs, system_time) == ERROR) {
	std::cerr << "Cannot schedule event\n";
	exit(-1);
      }
      new_busy_cores += e->n_cores;
      new_busy_memory += e->memory;
      delete e;
    } else {
      break;
    }
  }
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
#ifdef DEBUG_LOG
  std::cout << "Executing Cancel_task with " << memory << " memory and "
	    << n_cores << " cores\n";
#endif

  cs->cores_free(n_cores, *system_time);
  cs->mem_free(memory, *system_time);
  unsigned int new_busy_cores = 0;
  unsigned int new_busy_memory = 0;
  while (queue->size() != 0) {
    Event *e = queue->try_pop(cs->cores_available() - new_busy_cores, cs->memory_available() - new_busy_memory,
			      *system_time);
    if (e != nullptr) {
      if(schedule(new Execute_task(*system_time, e->execution_time,
				e->n_cores, e->memory), calendar, queue,
		  cs, system_time) == ERROR) {
	std::cerr << "Cannot schedule event\n";
	exit(-1);
      }
      delete e;
      new_busy_cores += e->n_cores;
      new_busy_memory += e->memory;
    } else {
      break;
    }
  }
}

Execute_task::Execute_task(Time stime, Time etime, unsigned int ncores, unsigned int mem)
  : Event(stime, etime, ncores, mem, "Execute task")
{
  // DO NOTHING
}

Execute_task::~Execute_task()
{
  // DO NOTHING
}

void Execute_task::execute(std::list<Event *> *calendar, Queue *queue,
	       CS *cs, Time *system_time)
{
#ifdef DEBUG_LOG
  std::cout << "Executing Execute task with " << memory << " memory and " << n_cores <<
    " cores\n";
#endif
  cs->mem_alloc(memory, *system_time);
  cs->cores_alloc(n_cores, *system_time);
  Event *ct = new Cancel_task(*system_time + execution_time,
			      n_cores, memory);
  if (schedule(ct, calendar, queue, cs, system_time) == ERROR) {
    std::cerr << "Cannot schedule event\n";
    exit(-1);
  }
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

void print_calendar(std::list<Event *> *calendar)
{
  for (auto iter = calendar->rbegin(); iter != calendar->rend(); ++iter) {
    std::cout << (*iter)->name << ' ';
  }
  std::cout << std::endl;
}
