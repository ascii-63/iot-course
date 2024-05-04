import json
import redis
import paho.mqtt.client as mqtt

REDIS_HOST = 'localhost'  # Redis server host
REDIS_PORT = 6379  # Redis server port
REDIS_DB = 0  # Redis server database

redis_client = redis.Redis(host=REDIS_HOST, port=REDIS_PORT, db=REDIS_DB)

#########################

MQTT_HOST = 'armadillo.rmq.cloudamqp.com'  # MQTT server host
MQTT_PORT = 1883  # MQTT server port
MQTT_KEEP_ALIVE = 60  # Seconds
MQTT_USR = 'pvevbtsi:pvevbtsi'
MQTT_PWD = 'Ed1HOeWlt1dHOO-pkuHNR3tZpWshwbX-'
MQTT_CONTROL_TOPIC = 'node_control'

mqtt_client = mqtt.Client()
mqtt_client.username_pw_set(MQTT_USR, MQTT_PWD)
mqtt_client.connect(host=MQTT_HOST, port=MQTT_PORT, keepalive=MQTT_KEEP_ALIVE)

#########################


def messagePreprocessing(str_msg: str) -> str:
    """Preprocessing the BE node message in `str` before put it on FE"""
    json_msg = json.loads(str_msg)
    json_msg["voltage"] = round(json_msg["voltage"], 1)
    json_msg["current"] = round(json_msg["current"], 1)
    json_msg["power"] = round(json_msg["power"], 1)
    json_msg["energy"] = int(json_msg["energy"])

    return json.dumps(json_msg)


def getNodeList() -> list:
    """Get list of node from redis database"""

    list_node = []

    keys = redis_client.keys('*')
    for raw_key in keys:
        raw_key = str(raw_key)
        if ("node_" not in raw_key):
            continue

        key = raw_key[2:-1]
        value = str(redis_client.get(key))
        new_value = value[2:-1]
        new_value = messagePreprocessing(new_value)

        list_node.append(new_value)

    return list_node


def changeNodeStatus(new_status: bool, node_id: str):
    """ Publish a message to `node_control` topic on MQTT server to change node status"""

    message = node_id
    if new_status:
        message += "/close"
    else:
        message += "/open"

    retry = True
    while retry:
        try:
            result = mqtt_client.publish(
                topic=MQTT_CONTROL_TOPIC, payload=message)

            status = result[0]
            if status == 0:
                retry = False

        except Exception as e:
            pass
