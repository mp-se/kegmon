<template>
  <BsInputBase :width="width" :label="label" :help="help" :badge="badge">
    <textarea
      @click.right.prevent="openContextMenu"
      id="textArea"
      v-model="model"
      class="form-control"
      type="text"
      v-bind="$attrs"
      data-bs-toggle="tooltip"
      data-bs-custom-class="custom-tooltip"
      :data-bs-title="help"
    ></textarea>
  </BsInputBase>

  <div @click="closeContextMenu" id="contextMenu" class="dropdown-menu">
    <template v-for="o in contextMenuOptions" :key="o.value">
      <a class="dropdown-item" @click="insertText(o.value)">{{ o.label }}</a>
    </template>
  </div>
</template>

<script setup>
/**
 * Note! Limitation is one of these components per page since hardcode ID's are used.
 */
import { ref } from 'vue'

/**
 * Purpose: Use as a building block for providing a text area that can allow for multiple lines of text.
 */
defineOptions({
  inheritAttrs: false
})

const contextMenuOptions = ref([
  { label: 'Cancel', value: '' },

  { label: 'Network name, ${mdns}', value: '${mdns}' },
  { label: 'Chip ID, ${id}', value: '${id}' },
  { label: 'Sleep interval, ${sleep-interval}', value: '${sleep-interval}' },
  { label: 'Token, ${token}', value: '${token}' },
  { label: 'Token 2, ${token2}', value: '${token2}' },

  { label: 'Current angle/tilt, ${angle}', value: '${angle}' },
  { label: 'Current angle/tilt, ${tilt}', value: '${tilt}' },

  { label: 'Temperature (C), ${temp-c}', value: '${temp-c}' },
  { label: 'Temperature (F), ${temp-f}', value: '${temp-f}' },
  { label: 'Temperature Unit, ${temp-unit}', value: '${temp-unit}' },

  { label: 'Application version, ${app-ver}', value: '${app-ver}' },
  { label: 'Application build, ${app-build}', value: '${app-build}' },

  { label: 'Battery (V), ${battery}', value: '${battery}' },
  { label: 'Battery (%), ${battery-percent}', value: '${battery-percent}' },

  { label: 'Wifi signal strength, ${rssi}', value: '${rssi}' },
  { label: 'Time for measurement, ${run-time}', value: '${run-time}' },

  { label: 'Gravity, ${gravity}', value: '${gravity}' },
  { label: 'Gravity (SG), ${gravity-sg}', value: '${gravity-sg}' },
  { label: 'Gravity (Plato), ${gravity-plato}', value: '${gravity-plato}' },
  { label: 'Gravity unit, ${gravity-unit}', value: '${gravity-unit}' },

  { label: 'Corrected gravity, ${corr-gravity}', value: '${corr-gravity}' },
  {
    label: 'Corrected Gravity (SG), ${corr-gravity-sg}',
    value: '${corr-gravity-sg}'
  },
  {
    label: 'Corrected Gravity (Plato), ${corr-gravity-plato}',
    value: '${corr-gravity-plato}'
  }
])

function insertText(value) {
  if (value.length > 0) {
    var obj = document.getElementById('textArea')
    model.value =
      obj.value.substring(0, obj.selectionStart) +
      value +
      obj.value.substring(obj.selectionEnd, obj.value.length)
  }

  var menu = document.getElementById('contextMenu')
  menu.style.display = 'none'
}

const openContextMenu = (event) => {
  var menu = document.getElementById('contextMenu')
  menu.style.display = 'block'
  menu.style.left = event.pageX + 'px'
  menu.style.top = event.pageY + 'px'
}

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
