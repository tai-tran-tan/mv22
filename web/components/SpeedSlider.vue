<script setup >
import Mqtt from "paho-mqtt";

const props = defineProps({
  topic: String,
  userId: String,
  label: String,
});

const { topic, userId } = props;

const speed = ref(0);
let client;

watch(speed, async (s) => {
  console.log("Slider change", s);
  sendMessage(topic, s);
});
onMounted(() => {
  client = new Mqtt.Client("192.168.1.5", 9001, userId);
  connect(userId);
});
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
  connect(userId);
};
const onConnect = () => {
  console.log("onConnect");
  // Once a connection has been made, make a subscription.
  client.subscribe(topic);
  console.log(`${userId} have joined ${topic}`);
};

const onConnectionFailure = (ctx, code, msg) => {
  console.error("connection failed", msg);
};

// called when a message arrives
const onMessageArrived = (message) => {
  console.log("onMessageArrived:", message.topic, message.payloadString);
  messageHandler(message.payloadString);
};

const messageHandler = (msg) => {
  try {
    console.log("Message received", msg);
    speed.value = Number(msg)
  } catch (e) {
    console.warn(`Can not process message '${msg}'. Ignored!`, e);
  }
};

const sendMessage = async (topic, message) => {
  if (message != undefined) {
    let msg = new Mqtt.Message(
      typeof message === "string" ? message : `${message}`
    );
    msg.destinationName = topic;
    client.send(msg);
  } else {
    console.log("Emty message is ignored!");
  }
};
</script>
<template>
  <fieldset class="inline-flex gap-2">
    <legend>{{label}}</legend>
    <Slider
      v-model="speed"
      :step="10"
      class="w-1rem h-10rem p-3"
      orientation="vertical"
      :min="0"
      :max="180"
    />
    <InputNumber
      v-model.number="speed"
      :min="0"
      :max="180"
      class="flex-1"
      showButtons
      buttonLayout="vertical"
      style="width: 4rem"
      decrementButtonClassName="p-button-secondary"
      incrementButtonClassName="p-button-secondary"
      incrementButtonIcon="pi pi-plus"
      decrementButtonIcon="pi pi-minus"
    />
  </fieldset>
</template>