<template>
  <BsInputBase :width="width" :label="label" :help="help" :badge="badge">
    <textarea
      v-model="model"
      class="form-control"
      type="text"
      v-bind="$attrs"
      data-bs-toggle="tooltip"
      data-bs-custom-class="custom-tooltip"
      :data-bs-title="help"
    ></textarea>
  </BsInputBase>
</template>

<script setup>
/**
 * 2024-05-28 Bootstrap VueJS wrapper, Magnus Persson
 */

/**
 * Purpose: Use as a building block for providing a text area that can allow for multiple lines of text.
 */
defineOptions({
  inheritAttrs: false
})
/**
 * This is the v-model field that will be used to bind the component to (required).
 */
const model = defineModel()
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
