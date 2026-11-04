<template>
  <div class="container">
    <p></p>
    <p class="h2">Device - Settings</p>
    <hr />

    <BsMessage v-if="config.mdns === ''" dismissable="true" message="" alert="warning">
      You need to define a mdns name for the device
    </BsMessage>

    <form @submit.prevent="saveSettings" class="needs-validation" novalidate>
      <div class="row">
        <div class="col-md-12">
          <BsInputText
            v-model="config.mdns"
            maxlength="63"
            minlength="1"
            label="MDNS"
            help="Enter device name used on the network, the suffix .local will be added to this name"
            :badge="badge.deviceMdnsBadge()"
            :disabled="global.disabled"
          >
          </BsInputText>
        </div>

        <div class="col-md-12">
          <hr />
        </div>

        <div class="col-md-6">
          <BsInputRadio
            v-model="config.temp_format"
            :options="tempOptions"
            label="Temperature Format"
            width=""
            :disabled="global.disabled"
          ></BsInputRadio>
        </div>
        <div class="col-md-6">
          <BsInputRadio
            v-model="config.weight_unit"
            :options="weightOptions"
            label="Weight Format"
            width=""
            :disabled="global.disabled"
          ></BsInputRadio>
        </div>
        <div class="col-md-6">
          <BsInputRadio
            v-model="config.volume_unit"
            :options="volumeOptions"
            label="Volume Format"
            width=""
            :disabled="global.disabled"
          ></BsInputRadio>
        </div>

        <div class="col-md-12">
          <hr />
        </div>

        <div class="col-md-6">
          <BsInputRadio
            v-model="config.dark_mode"
            :options="uiOptions"
            label="User Interface"
            width=""
            :disabled="global.disabled"
          ></BsInputRadio>
        </div>
      </div>

      <div class="row gy-2">
        <div class="col-md-12">
          <hr />
        </div>
        <div class="col-md-12">
          <button
            type="submit"
            class="btn btn-primary w-2"
            :disabled="global.disabled || !global.configChanged"
          >
            <span
              class="spinner-border spinner-border-sm"
              role="status"
              aria-hidden="true"
              :hidden="!global.disabled"
            ></span>
            &nbsp;Save</button
          >&nbsp;

          <button
            @click="restart()"
            type="button"
            class="btn btn-secondary"
            :disabled="global.disabled"
          >
            <span
              class="spinner-border spinner-border-sm"
              role="status"
              aria-hidden="true"
              :hidden="!global.disabled"
            ></span>
            &nbsp;Restart device</button
          >&nbsp;

          <button
            @click="factory()"
            type="button"
            class="btn btn-secondary"
            :disabled="global.disabled"
          >
            <span
              class="spinner-border spinner-border-sm"
              role="status"
              aria-hidden="true"
              :hidden="!global.disabled"
            ></span>
            &nbsp;Restore factory defaults
          </button>
        </div>
      </div>
    </form>
  </div>
</template>

<script setup>
import { ref } from 'vue'
import { validateCurrentForm, restart } from '@/modules/utils'
import { global, config } from '@/modules/pinia'
import * as badge from '@/modules/badge'
import { logError, logInfo } from '@/modules/logger'

const tempOptions = ref([
  { label: 'Celsius °C', value: 'C' },
  { label: 'Fahrenheit °F', value: 'F' }
])

const volumeOptions = ref([
  { label: 'Centiliters (cl)', value: 'cl' },
  { label: 'US Fluid Ounce', value: 'us-oz' },
  { label: 'Imperial Fluid Ounce', value: 'uk-oz' }
])

const weightOptions = ref([
  { label: 'Kilograms (kg)', value: 'kg' },
  { label: 'Pounds (lbs)', value: 'lbs' }
])

const uiOptions = ref([
  { label: 'Day mode', value: false },
  { label: 'Dark mode', value: true }
])

const factory = () => {
  global.clearMessages()
  logInfo('DeviceSettingsView.factory()', 'Sending /api/factory')
  global.disabled = true
  fetch(global.baseURL + 'api/factory', {
    headers: { Authorization: global.token },
    signal: AbortSignal.timeout(global.fetchTimout)
  })
    .then((res) => res.json())
    .then((json) => {
      if (json.success == true) {
        global.messageSuccess = json.message
        setTimeout(() => {
          location.reload(true)
        }, 2000)
      } else {
        global.messageFailed = json.message
        global.disabled = false
      }
    })
    .catch((err) => {
      logError('DeviceSettingsView.factory()', err)
      global.messageError = 'Failed to do factory restore'
      global.disabled = false
    })
}

const saveSettings = () => {
  if (!validateCurrentForm()) return

  config.saveAll()
}
</script>
