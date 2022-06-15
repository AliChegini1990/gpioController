#include "gpioController.h"
#include <cassert>
#include <iostream>
#include <stdexcept>

#define massert(MSG, EX) assert((MSG, EX))
using namespace gpio_controller;

void test1() {
  try {

    GpioController gp1;
    GpioController gp2;

    // gp1 = gp2; // Error, they are not copyable

    gp1.AddController(unique_ptr<IController>(new ToogleBtn()));
    gp1.AddController(unique_ptr<IController>(new SystemSleepController()));
  } catch (const std::exception &ex) {
    cerr << ex.what() << endl;
  }
}

int main() {
  test1();
  return 0;
}
