#include "gpioController.h"
#include "libsoc_board.h"
#include "libsoc_gpio.h"

Gpins Gpins::Create(shared_ptr<board_config> config, gpio_mode mode,
                    gpio_edge edge, gpio_direction direction, gpio_level level,
                    callback call_back, Gpins::Pin pin) {
  Gpins pgpio;
  pgpio.mode = mode;
  pgpio.edge = edge;
  pgpio.direction = direction;
  pgpio.level = level;
  pgpio.gpio_interrupt_callback = call_back;
  pgpio.pin = pin;
  gpio_p = unique_ptr<gpio>(libsoc_gpio_request(
      libsoc_board_gpio_id(config.get(),
                           pin_to_mxm3[static_cast<unsigned char>(pin)]),
      mode));

  return pgpio;
}

void SystemSleepController::Init(shared_ptr<board_config> config) {
  if (!config) {
    throw runtime_error{"Couldn't initialize sleep controller, config is null"};
  }

  gpins_ = gpins_.Create(
      config, LS_SHARED, BOTH, INPUT, LOW,
      [](void *x) {
        this_thread::sleep_for(chrono::milliseconds(50));
        auto pr = static_cast<SystemSleepController *>(x);
        if (!pr->value_)
          return 0;

        pr->value_ = false;
        std::ofstream file("/sys/power/state");
        if (!file) {
          cerr << "GpioController::SystemSleep Error : can not open file"
               << endl;
        }
        file << "mem";
        file.close();
        return 0;
      },
      Gpins::Pin::kPin16);

  uint32_t ret =
      libsoc_gpio_set_direction(gpins_.gpio_p.get(), gpins_.direction);
  if (ret == EXIT_FAILURE) {
    throw runtime_error{"Failed to set gpio direction"};
  }
  // Set Intrrupt Mode
  ret = libsoc_gpio_set_edge(gpins_.gpio_p.get(), gpins_.edge);
  if (ret == EXIT_FAILURE) {
    throw runtime_error{"Failed to set gpio edge"};
  }

  // Set intrrupt callback
  ret = libsoc_gpio_callback_interrupt(gpins_.gpio_p.get(),
                                       gpins_.gpio_interrupt_callback, this);
  if (ret == EXIT_FAILURE) {
    throw runtime_error{"Failed to set gpio callback"};
  }
}
void SystemSleepController::Close() {
  if (gpins_.gpio_p) {
    uint32_t ret = libsoc_gpio_free(gpins_.gpio_p.get());
    if (ret == EXIT_FAILURE)
      cerr << "SleepController close: Couldn't free gpio" << endl;
  }
}
void ToogleBtn::Init(shared_ptr<board_config> config) {
  if (!config) {
    throw runtime_error{"Couldn't initialize sleep controller, config is null"};
  }

  gpins_ = gpins_.Create(
      config, LS_SHARED, RISING, INPUT, LOW,
      [](void *x) {
        auto pr = static_cast<ToogleBtn *>(x);
        pr->value_ = !pr->value_;
        return 1;
      },
      Gpins::Pin::kPin14);

  uint32_t ret =
      libsoc_gpio_set_direction(gpins_.gpio_p.get(), gpins_.direction);
  if (ret == EXIT_FAILURE) {
    throw runtime_error{"Failed to set gpio direction"};
  }

  // Set Intrrupt Mode
  ret = libsoc_gpio_set_edge(gpins_.gpio_p.get(), gpins_.edge);
  if (ret == EXIT_FAILURE) {
    throw runtime_error{"Failed to set gpio edge"};
  }

  // Set intrrupt callback
  ret = libsoc_gpio_callback_interrupt(gpins_.gpio_p.get(),
                                       gpins_.gpio_interrupt_callback, this);
  if (ret == EXIT_FAILURE) {
    throw runtime_error{"Failed to set gpio callback"};
  }
}

void ToogleBtn::Close() {
  if (gpins_.gpio_p) {
    uint32_t ret = libsoc_gpio_free(gpins_.gpio_p.get());
    if (ret == EXIT_FAILURE)
      cerr << "Couldn't free gpio" << endl;
  }
}

GpioController::GpioController() {
  config_ =
      unique_ptr<board_config>(libsoc_board_init()); // initialize libsoc once
  if (config_ == NULL) {
    throw runtime_error{"Error: Couldn't init board"};
  }
}

GpioController::~GpioController() {
  for (auto &item : clist_) {
    try {
      item->Close();
    } catch (const std::exception &ex) {
      cerr << ex.what();
    }
  }
}

void GpioController::AddController(unique_ptr<IController> controller) {
  try {
    controller->Init(config_);
    clist_.push_back(std::move(controller));
  } catch (const std::exception &ex) {
    cerr << ex.what();
  }
}
