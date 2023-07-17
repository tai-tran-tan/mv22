<template>
  <div class="flex flex-column">
    <canvas
      id="myCanvas"
      :width="size.x"
      :height="size.y"
      style="border: 1px solid black; background-color: #818ea1"
    />
    <span
    > {{ angle.toFixed(1) }} deg / {{ radian.toFixed(1) }} rad </span>
  </div>
</template>

<script setup>
const { $publishMqtt, $subscribeMqtt } = useNuxtApp();
const props = defineProps({
  size: Object,
});
const skyColor = "#609af8";
const groundColor = "#eec137";
const radian = ref(0);
const toDegrees = (r) => (r * 180) / Math.PI;
const angle = computed(() => toDegrees(radian.value));
onMounted(() => {
  paint();
  $subscribeMqtt("angle", (msg) => {
    const angle = Number(msg);
    if (!Number.isNaN(angle)) {
      radian.value = angle;
      paint()
    } else {
      console.log("Ignored", msg);
    }
  });
});
const paint = () => {
  const canvas = document.getElementById("myCanvas");
  const center = { x: props.size.x / 2, y: props.size.y / 2 };
  const radix = center.x - 5;
  const ctx = canvas.getContext("2d");

  ctx.beginPath();
  ctx.arc(center.x, center.y, radix, 0, 2 * Math.PI);
  ctx.fillStyle = skyColor;
  ctx.fill();
  ctx.stroke();
  ctx.closePath();

  ctx.beginPath();
  ctx.arc(center.x, center.y, radix, 0 + radian.value, Math.PI + radian.value);
  ctx.fillStyle = groundColor;
  ctx.fill();
  ctx.moveTo(0, center.y);
  ctx.lineTo(2 * center.x, center.y);
  ctx.stroke();
  ctx.closePath();
};
</script>

<style>
</style>