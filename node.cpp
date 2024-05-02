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
#include "json/json.h"

#define ARGC_NEEDED 3
#define ALWAYS_INLINE inline __attribute__((always_inline))

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
const uint8_t close_relay0[REQUEST_BYTES_LENGTH] = {0x01, 0x05, 0x00, 0x00, 0xFF, 0x00, 0x8C, 0x3A};
const uint8_t open_relay0[REQUEST_BYTES_LENGTH] = {0x01, 0x05, 0x00, 0x00, 0x00, 0x00, 0xCD, 0xCA};

modbus_t *ctx = nullptr;

// A class to store Electrometer stats
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

/*******************************************************************/

// Send a request to device, get response in bytes
bool ALWAYS_INLINE sendRequest_getResponse(modbus_t *_ctx, const uint8_t *_bytes, uint8_t *_response)
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
        int response_length = 0;
        while (response_length == 0)
        {
            response_length = modbus_receive_confirmation(_ctx, response_buffer);
        }

        try
        {
            memcpy(_response, response_buffer, sizeof(response_buffer));
        }
        catch (const std::exception &exc)
        {
            std::cerr << exc.what() << std::endl;
            _response = nullptr;
            return false;
        }
        return true;
    }
}

// Read Stats from Modbus device
Electrometer ALWAYS_INLINE readStats(modbus_t *_ctx)
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
        bool status = true;
        if (voltage == 0)
            status = false;

        return Electrometer(voltage, current, power, energy, status);
    }
    catch (const std::exception &exc)
    {
        std::cerr << "Exception while read raw stats buffer: " << exc.what() << std::endl;
        return Electrometer();
    }
}

// Open Relay 0 from Modbus device
bool ALWAYS_INLINE openRelay(modbus_t *_ctx)
{
    std::this_thread::sleep_for(std::chrono::seconds(2));
    std::cout << ">___________ Open" << std::endl;
    uint8_t dump_buffer[MODBUS_TCP_MAX_ADU_LENGTH];
    bool result = sendRequest_getResponse(_ctx, open_relay0, dump_buffer);
    std::this_thread::sleep_for(std::chrono::seconds(2));
    if (!result)
        return false;
    return true;
}

// Close Relay 0 from Modbus device
bool ALWAYS_INLINE closeRelay(modbus_t *_ctx)
{
    std::this_thread::sleep_for(std::chrono::seconds(2));
    std::cout << ">___________ Close" << std::endl;
    uint8_t dump_buffer[MODBUS_TCP_MAX_ADU_LENGTH];
    bool result = sendRequest_getResponse(_ctx, close_relay0, dump_buffer);
    std::this_thread::sleep_for(std::chrono::seconds(2));

    if (!result)
        return false;
    return true;
}

/////////////////////////////////////

const std::string SERVER_ADDRESS = "tcp://armadillo.rmq.cloudamqp.com:1883";
const std::string USERNAME = "pvevbtsi:pvevbtsi";
const std::string PASSWORD = "Ed1HOeWlt1dHOO-pkuHNR3tZpWshwbX-";

const std::string PERSIST_DIR = "./persist";
const int QOS = 1;
const auto TIMEOUT = std::chrono::seconds(10);

class action_listener : public virtual mqtt::iaction_listener
{
    std::string name_;

    void on_failure(const mqtt::token &tok) override
    {
        std::cout << name_ << " failure";
        if (tok.get_message_id() != 0)
            std::cout << " for token: [" << tok.get_message_id() << "]" << std::endl;
        std::cout << std::endl;
    }

    void on_success(const mqtt::token &tok) override
    {
        std::cout << name_ << " success";
        if (tok.get_message_id() != 0)
            std::cout << " for token: [" << tok.get_message_id() << "]" << std::endl;
        auto top = tok.get_topics();
        if (top && !top->empty())
            std::cout << "\ttoken topic: '" << (*top)[0] << "', ..." << std::endl;
        std::cout << std::endl;
    }

public:
    action_listener(const std::string &name) : name_(name) {}
};

// A callback class for use with the main MQTT client.
class callback : public virtual mqtt::callback, public virtual mqtt::iaction_listener
{
    mqtt::async_client &cli_;
    action_listener subListener_;
    mqtt::connect_options &connOpts_;

public:
    void reconnect()
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(2500));
        try
        {
            cli_.connect(connOpts_, nullptr, *this);
        }
        catch (const mqtt::exception &exc)
        {
            std::cerr << "Error: " << exc.what() << std::endl;
            exit(1);
        }
    }

    void connection_lost(const std::string &cause) override
    {
        std::cout << "\nConnection lost" << std::endl;
        if (!cause.empty())
            std::cout << "\tCause: " << cause << std::endl;
    }

    void message_arrived(mqtt::const_message_ptr msg) override
    {
        std::cout << "Message arrived" << std::endl;
        auto topic = msg->get_topic();
        std::cout << ">___________ topic: " << topic << std::endl;
        if (topic == "relay0")
        {
            std::cout << ">___________ payload: " << msg->get_payload() << std::endl;
            if (msg->get_payload() == "1")
            {

                if (ctx == nullptr)
                    return;
                closeRelay(ctx);
            }
            if (msg->get_payload() == "0")
            {

                if (ctx == nullptr)
                    return;
                openRelay(ctx);
            }
        }
    }

    void connected(const std::string &cause) override
    {

        cli_.subscribe("relay0", QOS, nullptr, subListener_);
    }

    // Re-connection failure
    void on_failure(const mqtt::token &tok) override
    {
        reconnect();
    }

    // (Re)connection success
    // Either this or connected() can be used for callbacks.
    void on_success(const mqtt::token &tok) override {}

public:
    callback(mqtt::async_client &cli, mqtt::connect_options &connOpts)
        : cli_(cli), connOpts_(connOpts), subListener_("Subscription") {}
};

/*******************************************************************/

// // Create and connect to an MQTT Server, and then return the client
// mqtt::async_client createMQTTClient(const std::string _client_id)
// {
//     std::cout << "Initializing for server '" << SERVER_ADDRESS << "'..." << std::endl;
//     // mqtt::async_client_ptr  client = std::make_shared<mqtt::async_client>(SERVER_ADDRESS, _client_id, PERSIST_DIR);
//     mqtt::async_client client(SERVER_ADDRESS, _client_id, PERSIST_DIR);

//     auto connOpts = mqtt::connect_options_builder()
//                         .clean_session()
//                         .finalize();

//     connOpts.set_user_name(USERNAME);
//     connOpts.set_password(PASSWORD);

//     callback cb(client, connOpts);
//     client.set_callback(cb);

//     try
//     {
//         std::cout << "Connecting..." << std::endl;
//         mqtt::token_ptr conntok = client.connect(connOpts);
//         std::cout << "Waiting for the connection..." << std::endl;
//         conntok->wait();
//         std::cout << "Connected to MQTT broker successfully." << std::endl;
//     }
//     catch (const mqtt::exception &exc)
//     {
//         std::cerr << "Error while connect to MQTT server: " << exc.what() << std::endl;

//         return mqtt::async_client();
//     }
//     return client;
// }

std::string createMessage(const std::string _node_id, const Electrometer &_stats)
{
    Json::Value json_stats;
    json_stats["node_id"] = _node_id;
    json_stats["voltage"] = _stats.voltage;
    json_stats["current"] = _stats.current;
    json_stats["power"] = _stats.power;
    json_stats["energy"] = _stats.energy;
    json_stats["status"] = _stats.status;

    std::string str_stats = json_stats.toStyledString();
    return str_stats;
}

/*******************************************************************/

int main(int argc, char *argv[])
{
    if (argc < ARGC_NEEDED)
    {
        std::cerr << "Usage: ./" << std::string(argv[0]) << " [node_id] [device]\n";
        exit(EXIT_FAILURE);
    }

    const std::string node_id = std::string(argv[1]);
    const std::string topic = node_id + "_stats";
    const char *device = strdup(argv[2]);

    /////////////////////////////////////////

    // Create a new RTU context
    ctx = modbus_new_rtu(device, BAUDRATE, PARITY, DATA_BIT, STOP_BIT);
    if (ctx == nullptr)
    {
        std::cerr << "Unable to create the libmodbus context" << std::endl;
        exit(EXIT_FAILURE);
    }

    // Set response timeout
    modbus_set_response_timeout(ctx, int(TIMEOUT_SEC), int(TIMEOUT_MSEC));

    // Connect to the Modbus device
    if (modbus_connect(ctx) == -1)
    {
        std::cerr << "Connection failed: " << std::string(modbus_strerror(errno)) << std::endl;
        modbus_free(ctx);
        exit(EXIT_FAILURE);
    }

    // Set slave ID (device address)
    modbus_set_slave(ctx, SLAVE_ID);

    /////////////////////////////////////////

    // auto client = createMQTTClient(node_id);
    // if (!client)
    // {
    //     std::cerr << "Failed to create MQTT client." << std::endl;
    //     exit(EXIT_FAILURE);
    // }
    std::cout << "Initializing for server '" << SERVER_ADDRESS << "'..." << std::endl;
    // mqtt::async_client_ptr  client = std::make_shared<mqtt::async_client>(SERVER_ADDRESS, _client_id, PERSIST_DIR);
    mqtt::async_client client(SERVER_ADDRESS, node_id, PERSIST_DIR);

    auto connOpts = mqtt::connect_options_builder()
                        .clean_session()
                        .finalize();

    connOpts.set_user_name(USERNAME);
    connOpts.set_password(PASSWORD);

    callback cb(client, connOpts);
    client.set_callback(cb);

    try
    {
        std::cout << "Connecting..." << std::endl;
        mqtt::token_ptr conntok = client.connect(connOpts);
        std::cout << "Waiting for the connection..." << std::endl;
        conntok->wait();
        std::cout << "Connected to MQTT broker successfully." << std::endl;
    }
    catch (const mqtt::exception &exc)
    {
        std::cerr << "Error while connect to MQTT server: " << exc.what() << std::endl;
    }

    /////////////////////////////////////////

    // std::this_thread::sleep_for(std::chrono::seconds(2));
    // openRelay(ctx);
    // std::this_thread::sleep_for(std::chrono::seconds(2));
    // closeRelay(ctx);
    // std::this_thread::sleep_for(std::chrono::seconds(4));

    while (true)
    {
        auto current_stats = readStats(ctx);
        if (current_stats.voltage < 150)
        {
            // std::this_thread::sleep_for(std::chrono::seconds(1));
            std::cout << current_stats.voltage << std::endl;
            // continue;
        }

        std::cout << "Voltage: " << current_stats.voltage << std::endl
                  << "Current: " << current_stats.current << std::endl
                  << "Power: " << current_stats.power << std::endl
                  << "Status: " << current_stats.status << std::endl
                  << std::endl;

        std::string message = createMessage(node_id, current_stats);
        mqtt::message_ptr message_ptr = mqtt::make_message(topic, message);
        mqtt::delivery_token_ptr token = client.publish(message_ptr);
        if (token->is_complete())
            std::cerr << "Failed to publish message." << std::endl;

        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    /////////////////////////////////////////

    // Close connection and free the context
    modbus_close(ctx);
    modbus_free(ctx);

    return EXIT_SUCCESS;
}