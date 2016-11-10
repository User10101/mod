#define CATCH_CONFIG_MAIN

#include "model.h"

#include "catch.hpp"

CS cs(MEMORY_SIZE, CORES_COUNT);
std::list<Event *> calendar;
Time system_time = 0;

TEST_CASE("Queue", "main") {
  Queue q(5);
  q.enque_task(10, 3, system_time);
  system_time = 0.5;
  q.enque_task(8, 4, system_time);
  system_time = 0.7;
  q.enque_task(4, 11, system_time);

  Op_result r;
  unsigned int m, c;
  system_time = 1.1;
  r = q.top_fifo(&m, &c);
  REQUIRE(m == 10);
  REQUIRE(c == 3);
  REQUIRE(r == SUCCESS);
  q.pop_fifo();

  system_time = 1.2;
  r = q.top_fifo(&m, &c);
  REQUIRE(m == 8);
  REQUIRE(c == 4);
  REQUIRE(r == SUCCESS);
  q.pop_fifo();

  system_time = 1.4;
  r = q.top_fifo(&m, &c);
  REQUIRE(m == 4);
  REQUIRE(c == 11);
  REQUIRE(r == SUCCESS);
  q.pop_fifo();

  r = q.top_fifo(&m, &c);
  REQUIRE(m == -1);
  REQUIRE(c == -1);
  REQUIRE(r == ERROR);
  q.pop_fifo();

  std::vector<Time> stat = q.get_stat();
  std::vector<Time> exp_stat = { 1.1, 0.7, 0.7 };
}

TEST_CASE("CS Test", "Main") {
  REQUIRE(cs.cores_free(1) == ERROR);

  REQUIRE(cs.mem_alloc(MEMORY_SIZE + 1) == ERROR);
  REQUIRE(cs.mem_alloc(MEMORY_SIZE - 1) == SUCCESS);
  REQUIRE(cs.mem_alloc(1) == SUCCESS);
  REQUIRE(cs.mem_alloc(1) == ERROR);
  REQUIRE(cs.mem_free(1) == SUCCESS);
  REQUIRE(cs.mem_alloc(1) == SUCCESS);
  REQUIRE(cs.mem_alloc(1) == ERROR);

  REQUIRE(cs.cores_alloc(CORES_COUNT + 1) == ERROR);
  REQUIRE(cs.cores_alloc(CORES_COUNT - 1) == SUCCESS);
  REQUIRE(cs.cores_alloc(1) == SUCCESS);
  REQUIRE(cs.cores_alloc(1) == ERROR);
  REQUIRE(cs.cores_free(1) == SUCCESS);
  REQUIRE(cs.cores_alloc(1) == SUCCESS);
  REQUIRE(cs.cores_alloc(1) == ERROR);
}
