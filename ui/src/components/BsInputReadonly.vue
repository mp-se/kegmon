<template>
  <BsInputBase :width="width" :label="label" :help="help">
    <div class="input-group">
      <input
        v-model="model"
        class="form-control-plaintext"
        readonly
        type="text"
        v-bind="$attrs"
        data-bs-toggle="tooltip"
        data-bs-custom-class="custom-tooltip"
        :data-bs-title="help"
      />
    </div>
  </BsInputBase>
</template>

<script setup>
/**
 * 2024-05-28 Bootstrap VueJS wrapper, Magnus Persson
 */

/**
 * Purpose: Proviude an input field
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
</script>
