import paho.mqtt.client as mqtt
import json
import redis

MQTT_STATS_TOPIC = 'node_stats'
MQTT_HOST = 'armadillo.rmq.cloudamqp.com'  # MQTT server host
MQTT_PORT = 1883  # MQTT server port
MQTT_KEEP_ALIVE = 60  # Seconds
MQTT_USR = 'pvevbtsi:pvevbtsi'
MQTT_PWD = 'Ed1HOeWlt1dHOO-pkuHNR3tZpWshwbX-'

mqtt_client = mqtt.Client()

########################

REDIS_HOST = 'localhost'  # Redis server host
REDIS_PORT = 6379  # Redis server port
REDIS_DB = 0  # Redis server database

r = redis.Redis(host='localhost', port=6379, db=0)

########################


def on_connect(client: mqtt.Client, userdata, flags, rc):
    """Define callback function"""
    client.subscribe(MQTT_STATS_TOPIC)


def on_message(client: mqtt.Client, userdata, msg):
    message = str(msg.payload)
    replaced_message = message[2:-1]
    replaced_message = replaced_message.replace("\\n", "").replace("\\t", "")
    result = json.loads(replaced_message)

    r.set(result["node_id"], replaced_message)


if __name__ == "__main__":
    mqtt_client.username_pw_set(MQTT_USR, MQTT_PWD)
    mqtt_client.on_connect = on_connect
    mqtt_client.on_message = on_message

    mqtt_client.connect(host=MQTT_HOST, port=MQTT_PORT,
                        keepalive=MQTT_KEEP_ALIVE)

    # Start the loop to process incoming messages
    mqtt_client.loop_forever()
