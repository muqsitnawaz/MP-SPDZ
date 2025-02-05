/*
 * Machine.h
 *
 */

#ifndef MACHINE_H_
#define MACHINE_H_

#include "Processor/BaseMachine.h"
#include "Processor/Memory.h"
#include "Processor/Program.h"
#include "Processor/OnlineOptions.h"

#include "Processor/Online-Thread.h"

#include "GC/Machine.h"

#include "Tools/time-func.h"

#include <vector>
#include <map>
#include <atomic>
using namespace std;

template<class sint, class sgf2n>
class Machine : public BaseMachine
{
  /* The mutex's lock the C-threads and then only release
   * then we an MPC thread is ready to run on the C-thread.
   * Control is passed back to the main loop when the
   * MPC thread releases the mutex
   */

  vector<thread_info<sint, sgf2n>> tinfo;
  vector<pthread_t> threads;

  int my_number;
  Names& N;
  typename sint::mac_key_type alphapi;
  typename sgf2n::mac_key_type alpha2i;
  typename sint::bit_type::mac_key_type alphabi;

  // Keep record of used offline data
  DataPositions pos;

  int tn,numt;
  bool usage_unknown;

  void load_program(string threadname, string filename);

  public:

  vector<pthread_mutex_t> t_mutex;
  vector<pthread_cond_t>  client_ready;
  vector<pthread_cond_t>  server_ready;
  vector<Program>  progs;

  Memory<sgf2n> M2;
  Memory<sint> Mp;
  Memory<Integer> Mi;
  GC::Memories<typename sint::bit_type> bit_memories;

  vector<Timer> join_timer;
  Timer finish_timer;
  
  string prep_dir_prefix;

  bool direct;
  int opening_sum;
  bool parallel;
  bool receive_threads;
  int max_broadcast;
  bool use_encryption;
  bool live_prep;

  OnlineOptions opts;

  atomic<size_t> data_sent;

  Machine(int my_number, Names& playerNames, string progname,
      string memtype, int lg2, bool direct, int opening_sum, bool parallel,
      bool receive_threads, int max_broadcast, bool use_encryption, bool live_prep,
      OnlineOptions opts);

  const Names& get_N() { return N; }

  DataPositions run_tape(int thread_number, int tape_number, int arg, int line_number);
  void join_tape(int thread_number);
  void run();

  string memory_filename();

  // Only for Player-Demo.cpp
  Machine(Names& N = *(new Names())): N(N) {}

  void reqbl(int n);

  typename sint::bit_type::mac_key_type get_bit_mac_key() { return alphabi; }
};

#endif /* MACHINE_H_ */
