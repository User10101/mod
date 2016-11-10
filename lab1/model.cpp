#include "model.h"

Queue::Queue(size_t queue_size)
  : max_size(queue_size)
{
  // TODO
}

Queue::~Queue()
{
  // TODO
}

Op_result Queue::enque_task(unsigned int memory, unsigned int ncores,
			    Time system_time)
{
  if (queue.size() + 1 < max_size) {
    queue.push_back(Q_item(memory, ncores, system_time));
  }
}

Op_result Queue::top_fifo(unsigned int *memory, unsigned int *ncores)
{
  if (queue.size() == 0) {
    *memory = -1;
    *ncores = -1;
    return ERROR;
  }
  auto res = queue.front();
  *memory = res.memory;
  *ncores = res.ncores;
  return SUCCESS;
}

void Queue::pop_fifo()
{
  if (queue.size() != 0) {
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
  free_memory = memory_size;
  free_cores = core_num;
}

CS::~CS()
{
  // DO NOTHING
}

Op_result CS::mem_alloc(unsigned int size)
{
  if (free_memory >= size) {
    free_memory -= size;
    return SUCCESS;
  }

  return ERROR;
}

Op_result CS::mem_free(unsigned int size)
{
  if (free_memory + size <= memory_size) {
    free_memory += size;
    return SUCCESS;
  }

  return ERROR;
}

Op_result CS::cores_alloc(unsigned int num)
{
  if (free_cores >= num) {
    free_cores -= num;
    return SUCCESS;
  }

  return ERROR;
}

Op_result CS::cores_free(unsigned int num)
{
  if (free_cores + num <= core_num) {
    free_cores += num;
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

Event::Event(Time t, std::string n)
  : time(t), name(n)
{
  // DO NOTHING
}

Event::~Event()
{
  // DO NOTHING
}

Task::Task(Time stime, Time etime, unsigned int ncores, unsigned int mem)
  : Event(stime, "Task"), execution_time(etime), n_cores(ncores), memory(mem)
{
  // DO NOTHING
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
  if (queue->size() == 0 && memory <= cs->memory_available()
      && n_cores <= cs->cores_available()) {
    cs->mem_alloc(memory);
    cs->cores_alloc(n_cores);
    Event *ct = new Cancel_task(*system_time + time, n_cores, memory);
    schedule(ct, calendar, queue, cs, system_time);
  } else {
    queue->enque_task(memory, n_cores, *system_time);
  }
}

Cancel_task::Cancel_task(Time stime, unsigned int ncores, unsigned int mem)
  : Event(stime, "Cancel task"), n_cores(ncores), memory(mem)
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
  cs->cores_free(n_cores);
  cs->mem_free(memory);

  if (queue->size() != 0) {
    unsigned int m, n;
    while (true) {
      queue->top_fifo(&m, &n);
      if (m <= cs->memory_available() && n <= cs->cores_available()) {
	queue->pop_fifo();
	cs->mem_alloc(m);
	cs->cores_alloc(n);
      } else {
	break;
      }
    }
  }
}

Op_result schedule(Event *e, std::list<Event *> *calendar, Queue *queue,
	       CS *cs, Time *system_time)
{
  if (e->time < *system_time) {
    return ERROR;
  }

  for (auto iter = calendar->begin(); iter != calendar->end(); ++iter) {
    if ((*iter)->time < e->time) {
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
  for (auto iter = calendar->rbegin(); iter != calendar->rend();
       iter = calendar->rbegin()) {
    Event *e = *iter;
    calendar->remove(*iter);
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
