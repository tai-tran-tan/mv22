<template>
  <div class="gap-2 flex">
    <div class="gap-2">
      <SpeedSlider topic="speed_left" user-id="left-motor" label="Left motor" />
      <SpeedSlider
        topic="speed_right"
        user-id="right-motor"
        label="Right motor"
      />
      <SpeedSlider topic="thrust" user-id="thrust" label="Total thrust" />
    </div>
    <BalanceIndicator :size="{x: 120, y: 120}"/>
  </div>
</template>
<script setup>
const props = defineProps({
  topic: String,
  userId: String,
  label: String,
});

</script>