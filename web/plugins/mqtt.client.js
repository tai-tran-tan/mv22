import Mqtt from "paho-mqtt";

const userId = 'web-cockpit'
const client = new Mqtt.Client("192.168.1.5", 9001, userId);

const handlers = {}

const connect = (user) => {
    console.log(`connecting user ${user}`);
    // connect the client
    client.connect({
        onSuccess: onConnect,
        onFailure: onConnectionFailure,
        mqttVersion: 4, //v3.1.1
        reconnect: true,
    });

    client.onMessageArrived = onMessageArrived;
    client.onConnectionLost = reconnect;

    // called when the client loses its connection
};

const reconnect = (responseObject) => {
    console.log("onConnectionLost:", responseObject);
    connected = false
    connect(userId);
};
let connected = false
const onConnect = () => {
    console.log("onConnect");
    connected = true
    // Once a connection has been made, make a subscription.
    console.log(`${userId} have connected`);
};

const onConnectionFailure = (ctx, code, msg) => {
    console.error("connection failed", msg);
};

// called when a message arrives
const onMessageArrived = (message) => {
    console.log("onMessageArrived:", message.topic, message.payloadString);
    const messageHandler = handlers[message.topic]
    messageHandler(message.payloadString);
};

    

const publishMqtt = async (topic, message) => {
    if (message != undefined) {
        if (connected) {
            const msg = new Mqtt.Message(
                typeof message === "string" ? message : `${message}`
            );
            msg.destinationName = topic;
            client.send(msg);
        } else {
            setTimeout(() => publishMqtt(topic, message), 100)
        }
    } else {
        console.log("Emty message is ignored!");
    }
};

const subscribeMqtt = async (topic, handler) => {
    if (connected) {
        client.subscribe(topic)
        handlers[topic] = handler
        console.log('Subscribed to', topic)
    } else {
        setTimeout(() => subscribeMqtt(topic, handler), 100)
    }
}

connect(userId);

export default defineNuxtPlugin(nuxtApp => {
    return {
        hooks: {
            // You can directly register Nuxt app hooks here
            'render:done'() {
                console.log('Hello from plugin hooks')
            },
        },
        provide: {
            publishMqtt, subscribeMqtt
        }
    }
})