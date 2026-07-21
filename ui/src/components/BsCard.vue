<template>
  <div class="card">
    <div :class="headerStyle()">{{ header }}</div>
    <div class="card-body">
      <h5 class="card-title">
        <component v-if="icon !== undefined" :is="icon" width="16" height="16"></component>
        {{ title }}
      </h5>
      <p class="card-text">
        <!-- @slot Content of the card is placed via the slot -->
        <slot></slot>
      </p>
    </div>
  </div>
</template>

<script setup>
/**
 * 2024-05-28 Bootstrap VueJS wrapper, Magnus Persson
 */
/**
 * Purpose: Show a card with header, title and content.
 */
defineOptions({
  inheritAttrs: false
})
/**
 * Text to be displayed in the header of the Card (required)
 */
const header = defineModel('header')
/**
 * Title shown in the top of the card body (required).
 */
const title = defineModel('title')
/**
 * If defined an icon will be shown left of the title (required).
 */
const icon = defineModel('icon')
/**
 * If set the header will be in red
 */
const iserr = defineModel('iserr')
/**
 * Color for the header (if not error)
 */
const headerColor = defineModel('color')

function headerStyle() {
  if (iserr.value !== undefined && iserr.value) return 'card-header bg-danger-subtle'

  if (headerColor.value === undefined) return 'card-header bg-primary-subtle'

  return 'card-header bg-' + headerColor.value + '-subtle'
}
</script>
