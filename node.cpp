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

/******************************** DEFINE ********************************/

#define ARGC_NEEDED 3
#define ALWAYS_INLINE inline __attribute__((always_inline))

#define PARITY 78              // N
#define DATA_BIT 8             // 1 bytes = 8 bits
#define STOP_BIT 1             // Stop bit in requested bytes
#define TIMEOUT_SEC 5          // Connection timeout in seconds
#define TIMEOUT_MSEC 0         // Connection timeout in ms
#define SLAVE_ID 2             // Slave ID
#define REQUEST_BYTES_LENGTH 8 // Number bytes in a request
#define SIZEOF_REQUEST 8       // Size of request in bytes

#define PAYLOAD_CLOSE_RELAY "1" // Close relay payload on control topic
#define PAYLOAD_OPEN_RELAY "0"  // Open relay payload on control topic

/******************************** CONST ********************************/

const int BAUDRATE = 9600; // Modbus evice baudrate

const uint8_t read_stats[REQUEST_BYTES_LENGTH] = {0x02, 0x03, 0x01, 0x48, 0x00, 0x0A, 0x45, 0xE8};   // Bytes use for read stats
const uint8_t close_relay0[REQUEST_BYTES_LENGTH] = {0x01, 0x05, 0x00, 0x00, 0xFF, 0x00, 0x8C, 0x3A}; // Bytes use for close relay0
const uint8_t open_relay0[REQUEST_BYTES_LENGTH] = {0x01, 0x05, 0x00, 0x00, 0x00, 0x00, 0xCD, 0xCA};  // Bytes use for open relay0

const std::string SERVER_ADDRESS = "tcp://armadillo.rmq.cloudamqp.com:1883"; // MQTT server address
const std::string USERNAME = "pvevbtsi:pvevbtsi";                            // MQTT server username
const std::string PASSWORD = "Ed1HOeWlt1dHOO-pkuHNR3tZpWshwbX-";             // MQTT server password

const std::string PERSIST_DIR = "./persist";   // Persist directory
const int QOS = 1;                             // Quality of Service
const auto TIMEOUT = std::chrono::seconds(10); // Connection timeout

/******************************** GLOBAL VAR ********************************/

modbus_t *ctx = nullptr;   // A modbus pointer
std::string node_id;       // Node ID
std::string stats_topic;   // Stats topic
std::string control_topic; // Control topic

/******************************** MODBUS ********************************/

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

// Send a request to device, get response in bytes
bool ALWAYS_INLINE sendRequest_getResponse(const uint8_t *_bytes, uint8_t *_response)
{
    // Send request to the device
    int req_res = modbus_send_raw_request(ctx, _bytes, SIZEOF_REQUEST);

    if (req_res == -1)
    {
        std::cerr << "Send request failed: " << std::string(modbus_strerror(errno)) << std::endl;
        return false;
    }
    else
    {
        uint8_t response_buffer[MODBUS_TCP_MAX_ADU_LENGTH];
        int response_length = 0;
        while (response_length == 0) // Wait for the response before return
        {
            response_length = modbus_receive_confirmation(ctx, response_buffer);
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
Electrometer ALWAYS_INLINE readStats()
{
    uint8_t raw_stats[MODBUS_TCP_MAX_ADU_LENGTH];
    bool result = sendRequest_getResponse(read_stats, raw_stats);
    if (!result)
    {
        return Electrometer();
    }

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
        std::cerr << "Exception: " << exc.what() << std::endl;
        return Electrometer();
    }
}

// Open Relay 0 from Modbus device
bool ALWAYS_INLINE openRelay()
{
    std::this_thread::sleep_for(std::chrono::seconds(2));

    uint8_t dump_buffer[MODBUS_TCP_MAX_ADU_LENGTH];
    bool result = sendRequest_getResponse(open_relay0, dump_buffer);

    std::this_thread::sleep_for(std::chrono::seconds(2));

    if (!result)
        return false;
    return true;
}

// Close Relay 0 from Modbus device
bool ALWAYS_INLINE closeRelay()
{
    std::this_thread::sleep_for(std::chrono::seconds(2));

    uint8_t dump_buffer[MODBUS_TCP_MAX_ADU_LENGTH];
    bool result = sendRequest_getResponse(close_relay0, dump_buffer);

    std::this_thread::sleep_for(std::chrono::seconds(2));

    if (!result)
        return false;
    return true;
}

/******************************** MQTT ********************************/

// A listener class for use in callback function
class action_listener : public virtual mqtt::iaction_listener
{
    std::string name_;

    void on_failure(const mqtt::token &tok) override
    {
        std::cout << "Debug: " << name_ << " failure";
        if (tok.get_message_id() != 0)
            std::cout << " for token: [" << tok.get_message_id() << "]" << std::endl
                      << std::endl;
    }

    void on_success(const mqtt::token &tok) override
    {
        std::cout << "Debug: " << name_ << " success";
        if (tok.get_message_id() != 0)
            std::cout << " for token: [" << tok.get_message_id() << "]" << std::endl;
        auto top = tok.get_topics();
        if (top && !top->empty())
            std::cout << "\ttoken topic: '" << (*top)[0] << "', ..." << std::endl
                      << std::endl;
    }

public:
    action_listener(const std::string &name) : name_(name) {}
};

/**
 * Local callback & listener class for use with the client connection.
 * This is primarily intended to receive messages, but it will also monitor
 * the connection to the broker. If the connection is lost, it will attempt
 * to restore the connection and re-subscribe to the topic.
 */
class callback : public virtual mqtt::callback,
                 public virtual mqtt::iaction_listener
{
    mqtt::async_client &cli_;         // The MQTT client
    action_listener subListener_;     // An action listener to display the result of actions
    mqtt::connect_options &connOpts_; // Options to use if we need to reconnect
    std::string subListenerName_;     // Sub-listener name

public:
    void connected(const std::string &cause) override
    {
        cli_.subscribe(control_topic, QOS, nullptr, subListener_);
    }

    // This deomonstrates manually reconnecting to the broker by calling connect() again.
    void reconnect()
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(2500));
        try
        {
            cli_.connect(connOpts_, nullptr, *this);
        }
        catch (const mqtt::exception &exc)
        {
            std::cerr << "Exception: " << exc.what() << std::endl;
            exit(EXIT_FAILURE);
        }
    }

    // Callback for when the connection is lost.
    // This will initiate the attempt to manually reconnect.
    void connection_lost(const std::string &cause) override
    {
        std::cout << "\nConnection lost. Reconnecting..." << std::endl;
        if (!cause.empty())
        {
            std::cout << "\tCause: " << cause << std::endl;
        }

        reconnect();
    }

    // Re-connection failure
    void on_failure(const mqtt::token &tok) override
    {
        reconnect();
    }

    // (Re)connection success
    void on_success(const mqtt::token &tok) override {}

    // Callback for when a message arrives.
    void message_arrived(mqtt::const_message_ptr msg) override
    {
        auto topic = msg->get_topic(); // Message topic
        if (topic == control_topic)
        {
            if (msg->get_payload() == PAYLOAD_CLOSE_RELAY)
            {
                if (ctx == nullptr)
                    return;
                closeRelay();
            }
            if (msg->get_payload() == PAYLOAD_OPEN_RELAY)
            {
                if (ctx == nullptr)
                    return;
                openRelay();
            }
        }
    }

public:
    callback(mqtt::async_client &cli, mqtt::connect_options &connOpts) : cli_(cli),
                                                                         connOpts_(connOpts),
                                                                         subListener_("Subscription") {}
};

/******************************** GENERAL ********************************/

// Create Json message from Electrometer stats
std::string createMessage(const Electrometer &_stats)
{
    Json::Value json_stats;

    json_stats["node_id"] = node_id;
    json_stats["voltage"] = _stats.voltage;
    json_stats["current"] = _stats.current;
    json_stats["power"] = _stats.power;
    json_stats["energy"] = _stats.energy;
    json_stats["status"] = _stats.status;

    std::string str_stats = json_stats.toStyledString();
    return str_stats;
}

int main(int argc, char *argv[])
{
    if (argc < ARGC_NEEDED)
    {
        std::cerr << "Usage: ./" << std::string(argv[0]) << " [node_id] [device]\n";
        exit(EXIT_FAILURE);
    }

    node_id = std::string(argv[1]);
    stats_topic = node_id + "_stats";
    control_topic = node_id + "_control";
    char *device = strdup(argv[2]); // Modbus device address

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
        mqtt::token_ptr conntok = client.connect(connOpts);
        conntok->wait();
        std::cout << ">_ Connected to MQTT broker successfully." << std::endl;
    }
    catch (const mqtt::exception &exc)
    {
        std::cerr << "Exception: " << exc.what() << std::endl;
    }

    /////////////////////////////////////////

    while (true)
    {
        auto current_stats = readStats();
        if (current_stats.voltage < 150)
        {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            continue;
        }

        std::string message = createMessage(current_stats);
        mqtt::message_ptr message_ptr = mqtt::make_message(stats_topic, message);
        mqtt::delivery_token_ptr token = client.publish(message_ptr);
        if (token->is_complete())
            std::cerr << "Failed to publish message." << std::endl;

        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}