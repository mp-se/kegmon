<template>
  <div class="container">
    <p></p>
    <p class="h3">Device - WIFI</p>
    <hr />

    <BsMessage
      v-if="scanning"
      :dismissable="false"
      message="Scanning for wifi networks in range"
      alert="info"
    >
    </BsMessage>

    <BsMessage
      v-if="config.wifi_ssid === '' && config.wifi_ssid2 === ''"
      dismissable="true"
      message=""
      alert="warning"
    >
      You need to define at least one wifi network
    </BsMessage>

    <form @submit.prevent="save" class="needs-validation" novalidate>
      <div class="row">
        <div class="col-md-6">
          <BsSelect
            v-model="config.wifi_ssid"
            label="SSID #1"
            :options="networks"
            :badge="badge.deviceWifi1Badge()"
            :disabled="global.disabled"
          />
        </div>
        <div class="col-md-6">
          <BsInputText
            v-model="config.wifi_pass"
            type="password"
            maxlength="50"
            label="Password #1"
            help="Enter password for the first wifi network"
            :disabled="global.disabled"
          ></BsInputText>
        </div>

        <div class="col-md-6">
          <BsSelect
            v-model="config.wifi_ssid2"
            label="SSID #2"
            :options="networks"
            :badge="badge.deviceWifi2Badge()"
            :disabled="global.disabled"
          />
        </div>
        <div class="col-md-6">
          <BsInputText
            v-model="config.wifi_pass2"
            type="password"
            maxlength="50"
            label="Password #2"
            help="Enter password for the first wifi network"
            :disabled="global.disabled"
          ></BsInputText>
        </div>

        <div class="col-md-12">
          <hr />
        </div>

        <div class="col-md-6">
          <BsInputNumber
            v-model="config.wifi_portal_timeout"
            unit="seconds"
            label="Portal timeout"
            min="10"
            max="240"
            step="1"
            width="5"
            help="Max time the wifi portal is idle (10 to 240)"
            :disabled="global.disabled"
          ></BsInputNumber>
        </div>
        <div class="col-md-6">
          <BsInputNumber
            v-model="config.wifi_connect_timeout"
            unit="seconds"
            label="Connection timeout"
            min="1"
            max="60"
            step="1"
            width="5"
            help="Max time waiting for a wifi connection (1 to 60)"
            :disabled="global.disabled"
          >
          </BsInputNumber>
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
            &nbsp;Restart device
          </button>
        </div>
      </div>
    </form>
  </div>
</template>

<script setup>
import { validateCurrentForm, restart } from '@/modules/utils'
import { global, config } from '@/modules/pinia'
import * as badge from '@/modules/badge'
import { onMounted, ref } from 'vue'
import { logDebug } from '@/modules/logger'

const scanning = ref(false)
const networks = ref([])

function wifiName(label, rssi, encr) {
  var l = label
  if (encr) l += ' \u{1f512}'
  if (rssi > -50) l += ' (Excellent)'
  else if (rssi > -60) l += ' (Good)'
  else if (rssi > -67) l += ' (Minium)'
  else l += ' (Poor)'
  return l
}

onMounted(() => {
  scanning.value = true
  config.runWifiScan((success, data) => {
    if (success) {
      networks.value = [{ label: '-blank-', value: '', rssi: 0, encryption: 0, channel: 0 }]
      for (var n in data.networks) {
        var d = data.networks[n]
        var o = {
          label: wifiName(d.wifi_ssid, d.rssi, d.encryption),
          value: d.wifi_ssid,
          rssi: d.rssi,
          encryption: data.networks[n].encryption,
          channel: d.channel
        }

        var f = networks.value.filter((obj) => {
          return obj.value === d.wifi_ssid
        })
        logDebug('DeviceWifiView.onMounted()', 'result:', f, d.wifi_ssid)
        if (f.length === 0) networks.value.push(o)
      }
      scanning.value = false
    }
  })
})

const save = () => {
  if (!validateCurrentForm()) return

  config.saveAll()
  global.messageInfo =
    'If WIFI settings are changed, restart the device and enter the new URL of the device!'
}
</script>
