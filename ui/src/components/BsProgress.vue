<template>
  <div class="progress" style="height: 20px">
    <div class="progress-bar" role="progressbar" :style="progressStyle"></div>
  </div>
</template>

<script setup>
/**
 * 2024-05-28 Bootstrap VueJS wrapper, Magnus Persson
 */

import { computed } from 'vue'
/**
 * Purpose: Show a progress bar
 */

/**
 * Ref that contains the value of the progress bar (0-100) (required).
 */
const progress = defineModel('progress')

const progressStyle = computed(() => {
  return 'width: ' + progress.value + '%'
})
</script>
