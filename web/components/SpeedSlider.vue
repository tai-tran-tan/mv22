<script setup >
import Mqtt from "paho-mqtt";
const { $publishMqtt, $subscribeMqtt } = useNuxtApp()

const props = defineProps({
  label: String,
  topic: String
});

const speed = ref(0);

watch(speed, async (s) => {
  console.log("Slider change", s);
  $publishMqtt(props.topic, s);
});

const handler = msg => {
  try {
    const nr = Number(msg)
    if (!Number.isNaN(nr)) {
        speed.value = nr
    }
    } catch (e) {
        console.warn(`Can not process message '${msg}'. Ignored!`, e);
    }
}
onMounted(() => {
  $subscribeMqtt(props.topic, handler)
})


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