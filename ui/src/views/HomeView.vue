<template>
  <div class="container">
    <p></p>

    <div v-if="status" class="container overflow-hidden text-center">
      <div class="row gy-4">
        <div class="col-md-6">
          <BsCard header="Measurement" color="info" :title="config.beer_name1">
            <p class="text-center">
              <BsProgress :progress="tapProgress1"></BsProgress>
            </p>
            <p class="text-center">
              Volume: {{ status.beer_volume1 }} {{ config.getVolumeUnit }} Weight:
              {{ status.beer_weight1 }} {{ config.getWeightUnit }}
            </p>
            <p class="text-center">
              ABV: {{ config.beer_abv1 }}% EBC: {{ config.beer_ebc1 }} IBU: {{ config.beer_ibu1 }}
            </p>
          </BsCard>
        </div>

        <div class="col-md-6">
          <BsCard header="Measurement" color="info" :title="config.beer_name2">
            <p class="text-center">
              <BsProgress :progress="tapProgress2"></BsProgress>
            </p>
            <p class="text-center">
              Volume: {{ status.beer_volume2 }} {{ config.getVolumeUnit }} Weight:
              {{ status.beer_weight2 }} {{ config.getWeightUnit }}
            </p>
            <p class="text-center">
              ABV: {{ config.beer_abv2 }}% EBC: {{ config.beer_ebc2 }} IBU: {{ config.beer_ibu2 }}
            </p>
          </BsCard>
        </div>
        <p></p>
      </div>

      <div class="row gy-4">
        <div class="col-md-6">
          <BsCard header="Measurement" color="info" :title="config.beer_name1">
            <p class="text-center">Glasses left: {{ status.glass1 }}</p>
            <p class="text-center" v-if="status.last_pour_volume1 != 'NaN'">
              Last pour: {{ status.last_pour_volume1 }} {{ config.getVolumeUnit }}
            </p>
          </BsCard>
        </div>

        <div class="col-md-6">
          <BsCard header="Measurement" color="info" :title="config.beer_name2">
            <p class="text-center">Glasses left: {{ status.glass2 }}</p>
            <p class="text-center" v-if="status.last_pour_volume2 != 'NaN'">
              Last pour: {{ status.last_pour_volume2 }} {{ config.getVolumeUnit }}
            </p>
          </BsCard>
        </div>
        <p></p>
      </div>

      <div class="row gy-4">
        <div class="col-md-4" v-if="status.temperature !== 'NaN'">
          <BsCard header="Measurement" color="info" title="Temperature">
            <p class="text-center">{{ status.temperature }} {{ config.getTempUnit }}</p>
          </BsCard>
        </div>

        <div class="col-md-4" v-if="status.ha != {} && config.mqtt_target != ''">
          <BsCard header="Push" color="secondary" title="Home Assistant">
            <p class="text-center">{{ pushHomeAssistant }}</p>
          </BsCard>
        </div>

        <div
          class="col-md-4"
          v-if="status.brewspy != {} && config.brewspy_token1 != '' && config.brewspy_token2 != ''"
        >
          <BsCard header="Push" color="secondary" title="Brewspy">
            <p class="text-center">{{ pushBrewspy }}</p>
          </BsCard>
        </div>

        <div class="col-md-4" v-if="status.barhelper != {} && config.barhelper_apikey != ''">
          <BsCard header="Push" color="secondary" title="Barhelper">
            <p class="text-center">{{ pushBarhelper }}</p>
          </BsCard>
        </div>

        <div class="col-md-4" v-if="status.brewlogger != {} && config.brewlogger_url != ''">
          <BsCard header="Push" color="secondary" title="BrewLogger">
            <p class="text-center">{{ pushBrewLogger }}</p>
          </BsCard>
        </div>

        <div class="col-md-4">
          <BsCard header="Device" title="WIFI">
            <p class="text-center">{{ status.rssi }} dBm - {{ status.wifi_ssid }}</p>
          </BsCard>
        </div>

        <div class="col-md-4">
          <BsCard header="Device" title="IP Address">
            <p class="text-center">
              {{ status.ip }}
            </p>
          </BsCard>
        </div>

        <div class="col-md-4">
          <BsCard header="Device" title="Memory">
            <p class="text-center">
              Free: {{ status.free_heap }} kb, Total: {{ status.total_heap }} kb
            </p>
          </BsCard>
        </div>

        <div class="col-md-4">
          <BsCard header="Device" title="Software version">
            <p class="text-center">
              Firmware: {{ status.app_ver }} ({{ status.app_build }}) UI: {{ global.uiVersion }} ({{
                global.uiBuild
              }})
            </p>
          </BsCard>
        </div>

        <div class="col-md-4">
          <BsCard header="Device" title="Platform">
            <p class="text-center">{{ status.platform }} (ID: {{ status.id }})</p>
          </BsCard>
        </div>

        <div class="col-md-4">
          <BsCard header="Device" title="Uptime">
            <p class="text-center">
              {{ status.uptime_days }} days {{ status.uptime_hours }} hours
              {{ status.uptime_minutes }} minutes {{ status.uptime_seconds }} seconds
            </p>
          </BsCard>
        </div>
      </div>
    </div>
  </div>
</template>

<script setup>
import { ref, computed, onBeforeMount, onBeforeUnmount } from 'vue'
import { status, global, config } from '@/modules/pinia'
import { logDebug, logError } from '@/modules/logger'

// TODO: Add humidity and pressure
// TODO: Show # glasses left for taps
// TODO: Show last pour per tap

const polling = ref(null)

const pushHomeAssistant = computed(() => {
  if (status.ha.push_used) {
    return (
      'Updated ' +
      new Number(status.ha.push_age / 1000).toFixed(0) +
      's ago, ' +
      (status.ha.push_status ? 'Success' : 'Failed, error ' + status.ha.push_code)
    )
  }

  return 'Not updated'
})

const pushBrewspy = computed(() => {
  if (status.brewspy.push_used) {
    return (
      'Updated ' +
      new Number(status.brewspy.push_age / 1000).toFixed(0) +
      's ago, ' +
      (status.brewspy.push_status ? 'Success' : 'Failed, error ' + status.brewspy.push_code)
    )
  }

  return 'Not updated'
})

const pushBrewLogger = computed(() => {
  if (status.brewlogger.push_used) {
    return (
      'Updated ' +
      new Number(status.brewlogger.push_age / 1000).toFixed(0) +
      's ago, ' +
      (status.brewlogger.push_status ? 'Success' : 'Failed, error ' + status.brewlogger.push_code)
    )
  }

  return 'Not updated'
})

const pushBarhelper = computed(() => {
  if (status.barhelper.push_used) {
    var payload = status.barhelper.push_response
    // Check for a faulty payload and fix it so we can parse it.
    if (payload.search('"volume:') != -1) {
      payload = payload.replace('volume:', 'volume":')
      payload = payload.replace('"\n', ',\n')
    }

    var message = ''

    try {
      var json = JSON.parse(payload)
      message = json.message
      logDebug('HomeView.pushBarhelper()', json)
    } catch (e) {
      logError('HomeView.pushBarhelper()', e, payload)
    }

    return (
      'Updated ' +
      new Number(status.barhelper.push_age / 1000).toFixed(0) +
      's ago, ' +
      (status.barhelper.push_status ? 'Success' : 'Failed, error ' + status.barhelper.push_code) +
      ', ' +
      message
    )
  }

  return 'Not updated'
})

const tapProgress1 = computed(() => {
  return Math.round((status.beer_volume1 / status.keg_volume1) * 100)
})

const tapProgress2 = computed(() => {
  return Math.round((status.beer_volume2 / status.keg_volume2) * 100)
})

function refresh() {
  status.load(() => {})
}

onBeforeMount(() => {
  refresh()
  polling.value = setInterval(refresh, 4000)
})

onBeforeUnmount(() => {
  clearInterval(polling.value)
})
</script>
