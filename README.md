# iot-course

## Requirements:
  1. For Paho MQTT C/CPP library:
```
sudo apt-get install build-essential gcc make cmake cmake-gui cmake-curses-gui \
&& sudo apt-get install libssl-dev \
&& sudo apt-get install doxygen graphviz
```
  2. Pkg-Config:
```
sudo apt-get update 
sudo apt-get install pkg-config
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
  3. JsonCPP:
```
git clone https://github.com/open-source-parsers/jsoncpp.git
cd jsoncpp
mkdir build && cd build
cmake ..
make
sudo make install
```
  4. Modbus Library
```
sudo apt-get install libmodbus-dev
```