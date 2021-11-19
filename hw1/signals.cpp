#include <iostream>
#include <signal.h>
#include "signals.h"
#include "Commands.h"

using namespace std;

void ctrlZHandler(int sig_num) {
	// TODO: Add your implementation
  std::cout << "ctrlZhandler, signum = " << sig_num << std::endl;
}

// void ctrlCHandler(int sig_num) {
//   // TODO: Add your implementation
//   std::cout << "ctrlChandler, signum = " << sig_num << std::endl;
// }

void alarmHandler(int sig_num) {
  // TODO: Add your implementation
  std::cout << "alarmHandler, signum = " << sig_num << std::endl;
}

