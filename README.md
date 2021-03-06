# State Machine Tool

SMT is a implementation of HSM (Hierachical State Machine) in c language.

## Features

* Unlimited statemachine hierarchies
* Deep history
* App defined Entry/Exit function on states
* App defined Guard/Action function on transitions
* statemachine destroy hook
* App defined context be passed through to all Entry/Exit/Guard/Action/Destroy hooks
* App defined method to allocate and free memory
* Easy to define events, states and transitions

## Build SMT

SMT use [CMake](https://cmake.org/) as default build system. Once you cloned the [smt repo](https://github.com/greenlaw110/smt), you can follow these steps to build SMT:

```
cd <your-smt-dir>
mkdir build
cd build
cmake ..
make
make install
```

This will install the libsmt.so into your `/usr/lib` dir


## Sample Apps

* [Helloworld](sample/hello_world/README.md)
* [Microwave](sample/microwave/README.md)

