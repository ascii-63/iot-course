# iot-course
IoT Course

## Dev Note:
  - Fork this repo, and create your branch before pushing any code.
  - Create a directory for your code; don't push to the root directory.

## Requirements:
  1. For Paho MQTT C/CPP library:
```
sudo apt-get install build-essential gcc make cmake cmake-gui cmake-curses-gui \
&& sudo apt-get install libssl-dev \
&& sudo apt-get install doxygen graphviz
```

## Dependencies:
  1. Paho MQTT C library:
```
git clone https://github.com/eclipse/paho.mqtt.c.git
cd paho.mqtt.c
git checkout v1.3.13

cmake -Bbuild -H. -DPAHO_ENABLE_TESTING=OFF -DPAHO_BUILD_STATIC=ON \
    -DPAHO_WITH_SSL=ON -DPAHO_HIGH_PERFORMANCE=ON
sudo cmake --build build/ --target install
sudo ldconfig
```
  2. Paho MQTT CPP library:
```
git clone https://github.com/eclipse/paho.mqtt.cpp
cd paho.mqtt.cpp

cmake -Bbuild -H. -DPAHO_BUILD_STATIC=ON \
    -DPAHO_BUILD_DOCUMENTATION=ON -DPAHO_BUILD_SAMPLES=ON
sudo cmake --build build/ --target install
sudo ldconfig
```