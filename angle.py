import paho.mqtt.client as mqtt

topic = "deviceid/team3/evt/angle"
server = "3.84.34.84"

def on_connect(client, userdata, flags, rc):
    print("Connected with RC : " + str(rc))
    client.subscribe(topic)

def on_message(client, userdata, msg):
    print(msg.topic+" "+str(msg.payload.decode('UTF-8')))
    if int(msg.payload.decode('UTF-8')) == 1:
        client.subscribe(topic)
    elif int(msg.payload.decode('UTF-8')) == 2:
        client.subscribe(topic)
    elif int(msg.payload.decode('UTF-8')) == 3:
        client.subscribe(topic)
    elif int(msg.payload.decode('UTF-8')) == 4:
        client.subscribe(topic)
    elif int(msg.payload.decode('UTF-8')) == 5:
        client.subscribe(topic)
    elif int(msg.payload.decode('UTF-8')) == 6:
        client.subscribe(topic)
    elif int(msg.payload.decode('UTF-8')) == 0:
        client.subscribe(topic)
    

client = mqtt.Client()
client.connect(server, 1883, 60)
client.on_connect = on_connect
client.on_message = on_message

client.loop_forever()