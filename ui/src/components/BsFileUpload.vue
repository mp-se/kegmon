<template>
  <BsInputBase :width="width" :label="label" :help="help" :badge="badge">
    <div class="btn-group" role="group">
      <input class="form-control" type="file" v-bind="$attrs" :disabled="disabled" />
    </div>
  </BsInputBase>
</template>

<script setup>
/**
 * 2024-05-28 Bootstrap VueJS wrapper, Magnus Persson
 */

/**
 * Purpose: How a file upload component that allows the user to select a file. s
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
 * Ref that steers if this component is enabled or not (required).
 */
const disabled = defineModel('disabled')
/**
 * Specify if an badge should be shown to guide the user (optional).
 */
const badge = defineModel('badge')
</script>
