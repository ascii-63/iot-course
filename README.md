# iot-course
IoT Course-electeic meter control

## Pre-require:
 - libmodbus
```bash
sudo apt install libmodbus-dev
```
- gcc
## Usage: 
- To compile the source code, in elecrtometer, run:
```bash 
gcc read-data.c -o read-data -lmodbus
```
- After compile , you will get a "read-data" execute file, assume that the modbus device loacate at /dev/ttyUSB0, run the execute file with sudo: 
```bash
sudo ./read-data 
```
- Enter 1 to request data 
- Enter 2 to close relay 0
- Enter 3 to open relay 0 
- Close the relay 0 before request data 

