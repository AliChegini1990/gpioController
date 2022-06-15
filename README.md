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

class Led: public IController{
  // override functions
};

int main(){
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
