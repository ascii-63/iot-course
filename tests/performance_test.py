import paho.mqtt.client as mqtt
import json
import time

MQTT_HOST = 'armadillo.rmq.cloudamqp.com'  # MQTT server host
MQTT_PORT = 1883  # MQTT server port
MQTT_KEEP_ALIVE = 60  # Seconds
MQTT_USR = 'pvevbtsi:pvevbtsi'
MQTT_PWD = 'Ed1HOeWlt1dHOO-pkuHNR3tZpWshwbX-'
MQTT_STATS_TOPIC = 'node_stats'

mqtt_client = mqtt.Client()
mqtt_client.username_pw_set(MQTT_USR, MQTT_PWD)
mqtt_client.connect(host=MQTT_HOST, port=MQTT_PORT, keepalive=MQTT_KEEP_ALIVE)

NUM_PUBLISHER = 10

stats_dict = {"voltage": 220,
              "current": 1,
              "power": 220,
              "energy": 100,
              "status": True}

while (True):
    for idx in range(NUM_PUBLISHER):
        node_id = "node" + str(idx)
        stats_dict["node_id"] = node_id
        stats_msg = json.dumps(stats_dict)

        result = mqtt_client.publish(
            topic=MQTT_STATS_TOPIC,
            payload=stats_msg)

        if result[0] == 0:
            print(f"Published: {stats_dict}")

    time.sleep(1)
