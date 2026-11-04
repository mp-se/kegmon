<template>
  <div class="container-fluid">
    <div style="height: 20px"></div>
    <div class="text-light text-center rounded-pill bg-primary" style="height: 30px">
      {{ text }}
    </div>
  </div>
</template>

<script setup>
/**
 * 2024-05-28 Bootstrap VueJS wrapper, Magnus Persson
 */

/**
 * Purpose: Provide a generic footer
 */

/**
 * Text to show in the footer (required).
 */
const text = defineModel('text')
</script>
