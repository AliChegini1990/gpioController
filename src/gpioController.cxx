#include "gpioController.h"
Gpins Gpins::Create(
    shared_ptr<board_config> config,
    gpio_mode mode = LS_GPIO_SHARED,
    gpio_edge edge = NONE_edge,
    gpio_direction direction = OUTPUT,
    gpio_level level = HIGH,
    gpio_controller_callback call_back = nullptr,
    Gpins::Pin pin = Gpins::Pin::pin13) 
{
  Gpins pgpio;
  pgpio.mode = mode;
  pgpio.edge = edge;
  pgpio.direction = direction;
  pgpio.level = level;
  pgpio.gpio_interrupt_callback = call_back;
  pgpio.pin = pin;
  gpio_p = shared_ptr<gpio>(
      libsoc_gpio_request(
        libsoc_board_gpio_id(config.get(), pin_to_mxm3[static_cast<unsigned char>(pin)]),
        mode)
      );

  return pgpio;
}






