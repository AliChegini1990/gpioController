#ifndef GPIO_CONTROLLER_H_
#define GPIO_CONTROLLER_H_

#include <fstream>
#include <iostream>
#include <memory>
#include <thread>
#include <vector>

#include "libsoc_board.h"
#include "libsoc_gpio.h"

using namespace std;

class Gpins {
public:
  Gpins() : gpio_interrupt_callback{nullptr} {}
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

  using callback = int(*)(void *);
  callback gpio_interrupt_callback;
  Gpins Create(shared_ptr<board_config> config, gpio_mode mode = LS_SHARED,
               gpio_edge edge = NONE, gpio_direction direction = OUTPUT,
               gpio_level level = HIGH, callback call_back = nullptr,
               Gpins::Pin pin = Gpins::Pin::kPin13);

  unique_ptr<gpio> gpio_p = nullptr;
  gpio_direction direction;
  gpio_level level;
  gpio_edge edge;
  gpio_mode mode;
  Pin pin;
};

class IController {
public:
  virtual void Init(shared_ptr<board_config> config) = 0;
  virtual void Close(void) = 0;

  virtual ~IController() {}
};

class SystemSleepController : public IController {
private:
  Gpins gpins_;
  volatile bool value_;

public:
  SystemSleepController() : value_{false} {}
  ~SystemSleepController() { Close(); }
  void Init(shared_ptr<board_config> config) override;
  void Close();
};

class ToogleBtn : public IController {
private:
  Gpins gpins_;
  volatile bool value_;

public:
  ToogleBtn() : value_{false} {}
  ~ToogleBtn() { Close(); }

  void Init(shared_ptr<board_config> config) override;
  void Close();
};

class GpioController {
public:
  GpioController(const GpioController &) = delete;            // cc
  GpioController &operator=(const GpioController &) = delete; // ca
  GpioController(GpioController &&) = delete;                 // mc
  GpioController &operator=(GpioController &&) = delete;      // ma
  GpioController();
  ~GpioController();
  void AddController(unique_ptr<IController> controller);

private:
  shared_ptr<board_config> config_;
  vector<unique_ptr<IController>> clist_;
};

#endif
