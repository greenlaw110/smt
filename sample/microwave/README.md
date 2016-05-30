# Microwave Demo App

Demonstrate how to use SMT to implement a history enabled Microwave statemachine documented in [this pdf](http://www.state-machine.com/doc/Pattern_History.pdf):

![image](/img/2CmLgW11.png)

## Build app

The application is composed with two parts:

* The statemachine app
* The main app use the statemachine app

We are using [cmake](https://cmake.org) to implement the build system

### Build statemachine app

Below is the steps to build statemachine app:

```bash
cd smt/sample/microwave/statemachine
mkdir build
cd build
cmake ..
make
```

This will generate the `libmicrowave.so` file.

### Build main app

```bash
cd smt/sample/microwave/app
mkdir build
cd build
cmake ..
make
```

This will generate the `mw_app` executable file in place. You can type `./mw_app` to launch the sample app. When it launchs it looks like:

```
Microwave State Machine Initialized

available commands: 
        0|open - open door
        1|close - close door
        2|off - turn off operation
        3|bake - bake
        4|toast - toast
        255|shutdown - shutdown machine

MICROWAVE INFO
        state: doorClosed
        lamp: off
        heater: off

microwave>
```

### Inside Microwave sample app

TBD...

