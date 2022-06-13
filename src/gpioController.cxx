#include "gpioController.h"
#include "libsoc_board.h"
#include "libsoc_gpio.h"

Gpins Gpins::Create(
    shared_ptr<board_config> config,
    gpio_mode mode,
    gpio_edge edge,
    gpio_direction direction,
    gpio_level level,
    callback call_back,
    Gpins::Pin pin) 
{
  Gpins pgpio;
  pgpio.mode = mode;
  pgpio.edge = edge;
  pgpio.direction = direction;
  pgpio.level = level;
  pgpio.gpio_interrupt_callback = call_back;
  pgpio.pin = pin;
  gpio_p = unique_ptr<gpio>(
      libsoc_gpio_request(
        libsoc_board_gpio_id(config.get(), pin_to_mxm3[static_cast<unsigned char>(pin)]),
        mode)
      );

  return pgpio;
}






