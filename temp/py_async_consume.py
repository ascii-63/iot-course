import asyncio
import paho.mqtt.client as mqtt


async def on_connect(client, userdata, flags, rc):
    print("Connected with result code "+str(rc))
    # Subscribe to the topic(s) you're interested in
    client.subscribe("node1_stats")


async def on_message(client, userdata, msg):
    print(f"Received message on topic '{msg.topic}': {msg.payload.decode()}")


async def consume_messages():
    client = mqtt.Client()
    client.on_connect = on_connect
    client.on_message = on_message

    # Replace with your MQTT broker's credentials
    client.username_pw_set(username="pvevbtsi:pvevbtsi",
                           password="Ed1HOeWlt1dHOO-pkuHNR3tZpWshwbX-")

    # Replace with your MQTT broker's address
    client.connect("armadillo.rmq.cloudamqp.com", 1883, 60)

    # Start the loop
    client.loop_start()

    try:
        # Keep the loop running indefinitely
        while True:
            await asyncio.sleep(1)
    finally:
        # Stop the loop when done
        client.loop_stop()


async def main():
    await consume_messages()

if __name__ == "__main__":
    asyncio.run(main())
