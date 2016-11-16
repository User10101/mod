#define CATCH_CONFIG_MAIN

#include "model.h"

#include "catch.hpp"

#define MEMORY_SIZE (1024 * 1024 * 1024)
#define CORES_COUNT 30

CS cs(MEMORY_SIZE, CORES_COUNT);
std::list<Event *> calendar;
Time system_time = 0;

TEST_CASE("Queue", "FIFO") {
  Queue q;
  Event *e = new Task(5, 1, 3, 1024);
  q.enque_task(e);
  system_time = 0.5;
  Event *e1 = new Task(7, 2, 4, 2048);
  q.enque_task(e1);
  system_time = 0.7;
  Event *e2 = new Task(7.5, 2, 2, 512);
  q.enque_task(e2);

  Op_result r;
  Event *tmp;
  system_time = 5;
  tmp = q.top_fifo();
  REQUIRE(tmp->execution_time == 1);
  REQUIRE(tmp->n_cores == 3);
  REQUIRE(tmp->memory == 1024);
  q.pop_fifo(system_time);

  system_time = 10.5;
  tmp = q.top_fifo();
  REQUIRE(tmp->execution_time == 2);
  REQUIRE(tmp->n_cores == 4);
  REQUIRE(tmp->memory == 2048);
  q.pop_fifo(system_time);

  system_time = 11;
  tmp = q.top_fifo();
  REQUIRE(tmp->execution_time == 2);
  REQUIRE(tmp->n_cores == 2);
  REQUIRE(tmp->memory == 512);
  q.pop_fifo(system_time);

  tmp = q.top_fifo();
  REQUIRE(tmp == nullptr);
  q.pop_fifo(system_time);

  std::vector<Time> stat = q.get_stat();
  std::vector<Time> exp_stat = { 0, 3.5, 3.5 };
  for (size_t i = 0; i < stat.size(); ++i) {
    REQUIRE(Approx(stat[i]) == exp_stat[i]);
  }
}

TEST_CASE("Sl_queue", "Sl")
{
  Sl_queue q;
  Event *e = new Task(5, 1, 3, 1024);
  q.enque_task(e);
  Event *e1 = new Task(7, 2, 4, 2048);
  q.enque_task(e1);
  Event *e2 = new Task(7.5, 2, 2, 512);
  q.enque_task(e2);

  Op_result r;
  Event *tmp;
  system_time = 7.5;
  tmp = q.top_fifo();
  REQUIRE(tmp->execution_time == 2);
  REQUIRE(tmp->n_cores == 2);
  REQUIRE(tmp->memory == 512);
  q.pop_fifo(system_time);

  system_time = 10.5;
  tmp = q.top_fifo();
  REQUIRE(tmp->execution_time == 1);
  REQUIRE(tmp->n_cores == 3);
  REQUIRE(tmp->memory == 1024);
  q.pop_fifo(system_time);

  system_time = 11;
  tmp = q.top_fifo();
  REQUIRE(tmp->execution_time == 2);
  REQUIRE(tmp->n_cores == 4);
  REQUIRE(tmp->memory == 2048);
  q.pop_fifo(system_time);

  tmp = q.top_fifo();
  REQUIRE(tmp == nullptr);
  q.pop_fifo(system_time);

  std::vector<Time> stat = q.get_stat();
  std::vector<Time> exp_stat = { 0, 5.5, 4 };
  for (size_t i = 0; i < stat.size(); ++i) {
    REQUIRE(Approx(stat[i]) == exp_stat[i]);
  }
}

TEST_CASE("CS Test", "Main") {
  REQUIRE(cs.cores_free(1, 0) == ERROR);

  REQUIRE(cs.mem_alloc(MEMORY_SIZE + 1, 0) == ERROR);
  REQUIRE(cs.mem_alloc(MEMORY_SIZE - 1, 0) == SUCCESS);
  REQUIRE(cs.mem_alloc(1, 0) == SUCCESS);
  REQUIRE(cs.mem_alloc(1, 0) == ERROR);
  REQUIRE(cs.mem_free(1, 0) == SUCCESS);
  REQUIRE(cs.mem_alloc(1, 0) == SUCCESS);
  REQUIRE(cs.mem_alloc(1, 0) == ERROR);

  REQUIRE(cs.cores_alloc(CORES_COUNT + 1, 0) == ERROR);
  REQUIRE(cs.cores_alloc(CORES_COUNT - 1, 0) == SUCCESS);
  REQUIRE(cs.cores_alloc(1, 0) == SUCCESS);
  REQUIRE(cs.cores_alloc(1, 0) == ERROR);
  REQUIRE(cs.cores_free(1, 0) == SUCCESS);
  REQUIRE(cs.cores_alloc(1, 0) == SUCCESS);
  REQUIRE(cs.cores_alloc(1, 0) == ERROR);
}
