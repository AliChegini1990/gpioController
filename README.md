# GPIO Controller

A simple GPIO controller.


# Diagram

```
+----------------------+
|   gpioController     |
|                      |
|   +--------------+   |
|   |  IControlle  |   |
|   +------^-------+   |
|          |           |
|   +------+-------+   |
|   |  controller  |   |
|   |              |   |
|   |  +--------+  |   |
|   |  | Gpins  |  |   |
|   |  +--------+  |   |
|   |              |   |
|   +--------------+   |
+-----------+----------+
            |           
            |           
+-----------v----------+
|         libsoc       |
+----------------------+
```

# How to use

```c++
// Add some pins to the Gpin's class if you need.

class Led : public IController {
  // override functions

  void Init(shared_ptr<board_config> config) override {
    if (!config) {
      throw runtime_error{
          "Couldn't initialize sleep controller, config is null"};
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

    // Set intrrupt callback
    ret = libsoc_gpio_callback_interrupt(gpins_.gpio_p.get(),
                                         gpins_.gpio_interrupt_callback, this);
    if (ret == EXIT_FAILURE) {
      throw runtime_error{"Failed to set gpio callback"};
    }
  }
  void Close() override {
    if (gpins_.gpio_p) {
      uint32_t ret = libsoc_gpio_free(gpins_.gpio_p.get());
      if (ret == EXIT_FAILURE)
        cerr << "SleepController close: Couldn't free gpio" << endl;
    }
  }
};

int main() {
  GpioController gc;
  gc.AddController(unique_ptr<IController>(new Led()));
}

```

# Requirements
* [libsoc](https://github.com/jackmitch/libsoc)


# Build

```
mkdir build
cd build
cmake ..
make
```


# Test

```
./test/test

```
