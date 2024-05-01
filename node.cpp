#include <iostream>
#include <cstdlib>
#include <string>
#include <thread>
#include <atomic>
#include <chrono>
#include <cstring>
#include <stdexcept>

#include "mqtt/async_client.h"
#include "modbus/modbus.h"

/////////////////////////////////////

#define PARITY 78 // N
#define DATA_BIT 8
#define STOP_BIT 1

#define TIMEOUT_SEC 5
#define TIMEOUT_MSEC

#define SLAVE_ID 2

#define REQUEST_BYTES_LENGTH 8
#define SIZEOF_REQUEST 8

const char *DEV = "/dev/ttyUSB0";
const int BAUDRATE = 9600;

const uint8_t read_stats[REQUEST_BYTES_LENGTH] = {0x02, 0x03, 0x01, 0x48, 0x00, 0x0A, 0x45, 0xE8};
const uint8_t open_relay0[REQUEST_BYTES_LENGTH] = {0x01, 0x05, 0x00, 0x00, 0xFF, 0x00, 0x8C, 0x3A};
const uint8_t close_relay0[REQUEST_BYTES_LENGTH] = {0x01, 0x05, 0x00, 0x00, 0x00, 0x00, 0xCD, 0xCA};

class Electrometer
{
public:
    float voltage;
    float current;
    float power;
    float energy;
    bool status;

public:
    Electrometer(){};
    Electrometer(float _vol, float _cur, float _pow, float _ene, bool _sta) : voltage(_vol),
                                                                              current(_cur),
                                                                              power(_pow),
                                                                              energy(_ene),
                                                                              status(_sta){};
    ~Electrometer(){};
};

/////////////////////////////////////

/*******************************************************************/

// Send a request to device, get response in bytes
bool sendRequest_getResponse(modbus_t *_ctx, const uint8_t *_bytes, uint8_t *_response)
{
    // Send request to the device
    int req_res = modbus_send_raw_request(_ctx, _bytes, SIZEOF_REQUEST);

    if (req_res == -1)
    {
        std::cerr << "Send request failed: " << std::string(modbus_strerror(errno)) << std::endl;
        return false;
    }
    else
    {
        uint8_t response_buffer[MODBUS_TCP_MAX_ADU_LENGTH];
        int response_length = modbus_receive_confirmation(_ctx, response_buffer);
        if (response_length == 0)
            _response = nullptr;
        else
            try
            {
                memcpy(_response, response_buffer, sizeof(response_buffer));
            }
            catch (const std::exception &exc)
            {
                _response = nullptr;
                return false;
            }
        return true;
    }
}

// Read Stats from Modbus device
Electrometer readStats(modbus_t *_ctx)
{
    uint8_t raw_stats[MODBUS_TCP_MAX_ADU_LENGTH];
    bool result = sendRequest_getResponse(_ctx, read_stats, raw_stats);
    if (!result)
        return Electrometer();

    try
    {
        unsigned short voltage_raw = (raw_stats[3] << 8) | raw_stats[4];
        unsigned short current_raw = (raw_stats[5] << 8) | raw_stats[6];
        unsigned short power_raw = (raw_stats[7] << 8) | raw_stats[8];
        unsigned int energy_raw = (raw_stats[9] << 24) | (raw_stats[10] << 16) | (raw_stats[11] << 8) | raw_stats[12];

        float voltage = (float)voltage_raw / 100;
        float current = (float)current_raw / 1000;
        float power = (float)power_raw;
        float energy = (float)energy_raw / 100;

        
    }
    catch (const std::exception &exc)
    {
        std::cerr << "Exception while read raw stats buffer: " << exc.what() << std::endl;
        return Electrometer();
    }
}

int main(int argc, char *argv[])
{
    modbus_t *ctx;

    // Create a new RTU context
    ctx = modbus_new_rtu(DEV, BAUDRATE, PARITY, DATA_BIT, STOP_BIT);
    if (ctx == nullptr)
    {
        std::cerr << "Unable to create the libmodbus context" << std::endl;
        return EXIT_FAILURE;
    }

    // Set response timeout
    modbus_set_response_timeout(ctx, int(TIMEOUT_SEC), int(TIMEOUT_MSEC));

    // Connect to the Modbus device
    if (modbus_connect(ctx) == -1)
    {
        std::cerr << "Connection failed: " << std::string(modbus_strerror(errno)) << std::endl;
        modbus_free(ctx);
        // return EXIT_FAILURE;
    }

    // Set slave ID (device address)
    modbus_set_slave(ctx, SLAVE_ID);

    /////////////////////////////////////////////////////////////////////////

    return EXIT_SUCCESS;
}