<template>
  <div class="has-validation pt-2">
    <label v-if="label !== undefined" class="form-label fw-bold">{{ label }}</label>
    &nbsp;<span v-if="badge" class="badge text-bg-danger rounded-circle">1</span>
    <div :class="[width === undefined ? '' : 'col-' + width]">
      <!-- @slot this is where the main component is located -->
      <slot></slot>
    </div>
    <div class="form-text">{{ help }}</div>
  </div>
</template>

<script setup>
/**
 * 2024-05-28 Bootstrap VueJS wrapper, Magnus Persson
 */

/**
 * Purpose: Basic layout of a form component with label on top and help text below.
 *
 * Note! Should not be used directly, this is a base class for form components.
 */
defineOptions({
  inheritAttrs: false
})
/**
 * This text is shown above the form component (optional).
 */
const label = defineModel('label')
/**
 * Help text is shown below the field to provide user help with input (optional).
 */
const help = defineModel('help')
/**
 * Specify the width to force a specific size (optional).
 */
const width = defineModel('width')
/**
 * Specify if an badge should be shown to guide the user (optional).
 */
const badge = defineModel('badge')
</script>
