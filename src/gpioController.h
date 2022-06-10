#ifndef GPIO_CONTROLLER_H_
#define GPIO_CONTROLLER_H_

#include <fstream>
#include <iostream>
#include <memory>
#include <thread>

#include "libsoc_board.h"
#include "libsoc_gpio.h"

using namespace std;

class Gpins {
 public:
  // this class is a move only class
  Gpins(Gpins &&other) = default;
  Gpins &operator=(Gpins &&other) = default;
  Gpins(const Gpins &) = delete;
  Gpins &operator=(const Gpins &) = delete;

  // pin map
  const char *pin_to_mxm3[20] = {
      "MXM3_37" /*wake up*/,
      "MXM3_209" /*i2c_sda*/,
      "MXM3_211" /*i2c_scl*/,
      "MXM3_221" /*spi1_clk*/,
      "MXM3_227" /*spi1_cs*/,
      "MXM3_223" /*spi1_miso*/,
      "MXM3_225" /*spi1_mosi*/,
      "MXM3_1" /*gpio*/,
      "MXM3_3" /*gpio*/,
      "MXM3_5" /*gpio*/,
      "MXM3_7" /*gpio*/,
      "MXM3_11" /*gpio*/,
      "MXM3_13" /*gpio*/,
      "MXM3_15" /*gpio*/,
      "MXM3_17" /*gpio*/
  };

  // convert pin to gpio pin
  const char *pin_to_string[20] = {
      "3", /*wake up*/
      "5" /*i2c_sda*/,
      "6" /*i2c_scl*/,
      "8" /*spi1_clk*/,
      "9" /*spi1_cs*/,
      "10" /*spi1_miso*/,
      "11" /*spi1_mosi*/,
      "13" /*gpio*/,
      "14" /*gpio*/,
      "15" /*gpio*/,
      "16" /*gpio*/,
      "17" /*gpio*/,
      "18" /*gpio*/,
      "19" /*gpio*/,
      "20" /*gpio*/
  };
  enum class Pin {
    kPin13 = 7,
    kPin14 = 8,
    kPin15 = 9,
    kPin16 = 10,
    kPin17 = 11,
    kPin18 = 12,
    kPin19 = 13,
    kPin20 = 14
  };

  static Gpins Create(shared_ptr<board_config> config,
                      gpio_mode mode = LS_GPIO_SHARED,
                      gpio_edge edge = NONE_edge,
                      gpio_direction direction = OUTPUT,
                      gpio_level level = HIGH,
                      gpio_controller_callback call_back = nullptr,
                      Gpins::Pin pin = Gpins::Pin::kPin13);

  typedef int (*callback)(void *);
  callback gpio_interrupt_callback;

  unique_ptr<gpio> gpio_p = nullptr;
  gpio_direction direction;
  gpio_level level;
  gpio_edge edge;
  gpio_mode mode;
  Pin pin;
};

class IController {
 public:
  void Init(shared_ptr<board_config> config) = 0;
  void Close(void) = 0;

  virtual ~IController() {}
};

class SystemSleepController : public IController {
 private:
  Gpins gpins_;
  volatile bool value_;

 public:
  SystemSleep() : value_{false} {}
  ~SystemSleep() { Close(); }
  void Init(shared_ptr<board_config> config) {
    if (!config) {
      throw runtime_error{
          "Couldn't initialize sleep controller, config is null"};
    }

    gpins_ = Gpins::Create(
        config, LS_GPIO_SHARED, BOTH, INPUT, LOW,
        [&value_](void *x) {
          this_thread::sleep_for(chrono::milliseconds(50));
          if (!value_) return 0;

          value_ = false;
          std::ofstream file("/sys/power/state");
          if (!file) {
            cerr << "GpioController::SystemSleep Error : can not open file"
                 << endl;
          }
          file << "mem";
          file.close();
          return 0;
        },
        GpioPin::Pin::kPin16);

    uint32_t ret = libsoc_gpio_set_direction(gpins_.gpio_p, gpins_.direction);
    if (ret == EXIT_FAILURE) {
      throw runtime_error{"Failed to set gpio direction"};
    }
    // Set Intrrupt Mode
    ret = libsoc_gpio_set_edge(gpins_.gpio_p, gpins_.edge);
    if (ret == EXIT_FAILURE) {
      throw runtime_error{"Failed to set gpio edge"};
    }

    // Set intrrupt callback
    ret = libsoc_gpio_callback_interrupt(
        gpins_.gpio_p, gpins_.gpio_interrupt_callback, nullptr);
    if (ret == EXIT_FAILURE) {
      throw runtime_error{"Failed to set gpio callback"};
    }
  }
  void Close() {
    if (gpins_.gpio_p) {
      uint32_t ret = libsoc_gpio_free(gpins_.gpio_p.get());
      if (ret == EXIT_FAILURE)
        cerr << "SleepController close: Couldn't free gpio" << endl;
    }
  }
};

class ToogleBtn : public IController {
 private:
  Gpins gpins_;
  volatile bool value_;

 public:
  SystemSleep() : value_{false} {}
  ~SystemSleep() { Close(); }

  void Init(shared_ptr<board_config> config) {
    if (!config) {
      throw runtime_error{
          "Couldn't initialize sleep controller, config is null"};
    }

    gpins_ = Gpins::Create(
        config, LS_GPIO_SHARED, RISING, INPUT, LOW,
        [&value_](void *x) {
          value_ = !value_;
          return value_;
        },
        GpioPin::Pin::kPin14);

    uint32_t ret = libsoc_gpio_set_direction(gpins_.gpio_p, gpins_.direction);
    if (ret == EXIT_FAILURE) {
      throw runtime_error{"Failed to set gpio direction"};
    }

    // Set Intrrupt Mode
    ret = libsoc_gpio_set_edge(gpins_.gpio_p, gpins_.edge);
    if (ret == EXIT_FAILURE) {
      throw runtime_error{"Failed to set gpio edge"};
    }

    // Set intrrupt callback
    ret = libsoc_gpio_callback_interrupt(
        gpins_.gpio_p, gpins_.gpio_interrupt_callback, nullptr);
    if (ret == EXIT_FAILURE) {
      throw runtime_error{"Failed to set gpio callback"};
    }
  }

  void Close() {
    if (gpins_.gpio_p) {
      uint32_t ret = libsoc_gpio_free(gpins_.gpio_p.get());
      if (ret == EXIT_FAILURE) cerr << "Couldn't free gpio" << endl;
    }
  }
};

class GpioController {
 public:
  GpioController(const GpioController &) = delete;             // cc
  GpioController &operator=(const GpioController &) = delete;  // ca
  GpioController(GpioController &&) = delete;                  // mc
  GpioController &operator=(GpioController &&) = delete;       // ma

  GpioController() {
    config_ = libsoc_board_init();  // initialize libsoc once
    if (config_ == NULL) {
      throw runtime_error{"Error: Couldn't init board"};
    }
  }

  ~GpioController() {
    for (auto &item : clist_) {
      try {
        item->close();
      } catch (const std::exception &ex) {
        cerr << ex.what();
      }
    }
  }

  AddController(unique_ptr<IController> controller) {
    try {
      controller->Init(config_);
      clist.push_back(std::move(controller));
    } catch (const std::exception &ex) {
      cerr << ex.what()
    }
  }

 private:
  shared_ptr<board_config> config_;
  vector<unique_ptr<IController>> clist_;
};

#endif
