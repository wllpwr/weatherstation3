from signal import signal, SIGTERM, SIGHUP, pause
import time
import threading
import paho.mqtt.client as mqtt
import json
import requests
from ifttt_webhook import IftttWebhook

IFTTT_KEY = 'dETdJuNNCA7BYQf7l9TW9g'

##########################
# no idea, but i need it #
##########################


def safe_exit(signum, frame):
    exit(1)

################
# server stuff #
################
ifttt=IftttWebhook(IFTTT_KEY)
broker = 'localhost'
port = 1883
topic = "arduino"
client_name = "local"

incomingJson = {}
lastTrigger = 501
def setup_mqtt():
    print("Starting MQTT subscription...")
    mqtt_client = mqtt.Client(client_name)
    mqtt_client.connect(broker)

    mqtt_client.loop_start()


    mqtt_client.subscribe(topic)
    mqtt_client.on_message = handle_telemetry

def handle_telemetry(client, userdata, message):
    payload = message.payload.decode()
    print("A message came in:", payload)
    global incomingJson
    incomingJson = json.loads(payload)
    report = {}
    report["humi"] = incomingJson["humi"]
    print(report)
    global lastTrigger
    lastTrigger = lastTrigger + 1
    if report["humi"] > 70 and lastTrigger > 500:
        lastTrigger = 0
        ifttt.trigger("high_humi", value1=report["humi"])
        
########
# main #
########
if __name__ == '__main__':
    serverThread = threading.Thread(target=setup_mqtt())
    serverThread.setDaemon(True)
    serverThread.start()
    try:
        signal(SIGTERM, safe_exit)
        signal(SIGHUP, safe_exit)
        print("Getting data...")
        while True:
            continue
    except KeyboardInterrupt:
        pass
    finally:
        print("Exiting...")
