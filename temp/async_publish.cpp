#include <iostream>
#include <cstdlib>
#include <string>
#include <thread>
#include <atomic>
#include <chrono>
#include <cstring>
#include "mqtt/async_client.h"
#include "mqtt/client.h"

const std::string SERVER_ADDRESS = "tcp://armadillo.rmq.cloudamqp.com:1883";
const std::string USERNAME = "pvevbtsi:pvevbtsi";
const std::string PASSWORD = "Ed1HOeWlt1dHOO-pkuHNR3tZpWshwbX-";

const std::string CLIENT_ID = "node1";
const std::string PERSIST_DIR = "./persist";
const std::string TOPIC = "node1_stats";

const char *PAYLOAD1 = "Example message";
const int QOS = 1;
const auto TIMEOUT = std::chrono::seconds(10);
const bool USE_SIMPLE_LOGIN = false;

/*******************************************************************/

// A callback class for use with the main MQTT client.
class callback : public virtual mqtt::callback
{
public:
    void connection_lost(const std::string &cause) override
    {
        std::cout << "\nConnection lost" << std::endl;
        if (!cause.empty())
            std::cout << "\tCause: " << cause << std::endl;
    }
};

// Create and connect to an MQTT Server, and then return the client
mqtt::async_client_ptr createMQTTClient()
{
    std::cout << "Initializing for server '" << SERVER_ADDRESS << "'..." << std::endl;
    mqtt::async_client_ptr client = std::make_shared<mqtt::async_client>(SERVER_ADDRESS, CLIENT_ID, PERSIST_DIR);

    callback cb;
    client->set_callback(cb);

    auto connOpts = mqtt::connect_options_builder()
                        .clean_session()
                        .finalize();
    if (!USE_SIMPLE_LOGIN)
    {
        connOpts.set_user_name(USERNAME);
        connOpts.set_password(PASSWORD);
    }

    try
    {
        std::cout << "Connecting..." << std::endl;
        mqtt::token_ptr conntok = client->connect(connOpts);
        std::cout << "Waiting for the connection..." << std::endl;
        conntok->wait();
        std::cout << "Connected to MQTT broker successfully." << std::endl;
    }
    catch (const mqtt::exception &exc)
    {
        std::cerr << "Error while connect to MQTT server: " << exc.what() << std::endl;
        return nullptr;
    }
    return client;
}

int main()
{
    auto client = createMQTTClient();
    if (client)
    {
        std::string message = "Node #1 Stats";
        mqtt::message_ptr message_ptr = mqtt::make_message(TOPIC, message);
        mqtt::delivery_token_ptr token = client->publish(message_ptr);
        if (token->is_complete())
            std::cerr << "Failed to publish message." << std::endl;
        client->disconnect();
    }
    else
        std::cerr << "Failed to create MQTT client." << std::endl;

    return 0;
}
