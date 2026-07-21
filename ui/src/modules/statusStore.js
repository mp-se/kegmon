import { defineStore } from 'pinia'
import { global } from '@/modules/pinia'
import { logDebug, logError, logInfo } from '@/modules/logger'

export const useStatusStore = defineStore('status', {
  state: () => {
    return {
      // Status
      id: '',
      mdns: '',
      wifi_ssid: '',
      platform: '',
      app_ver: '',
      app_build: '',
      weight_unit: '',
      volume_unit: '',
      temp_format: '',
      temperature: 0,
      humidity: 0,
      pressure: 0,
      total_heap: 0,
      free_heap: 0,
      ip: '',
      rssi: 0,
      wifi_setup: false,
      connected: true,
      scale_busy: false,
      uptime_seconds: 0,
      uptime_minutes: 0,
      uptime_hours: 0,
      uptime_days: 0,

      // Keg 1
      glass1: 0,
      keg_volume1: 0,
      scale_factor1: 0,
      scale_weight1: 0,
      scale_raw1: 0,
      scale_offset1: 0,
      beer_weight1: 0,
      beer_volume1: 0,
      scale_stable_weight1: 0,
      last_pour_weight1: 0,
      last_pour_volume1: 0,

      // Keg 2
      glass2: 0,
      keg_volume2: 0,
      scale_factor2: 0,
      scale_weight2: 0,
      scale_raw2: 0,
      scale_offset2: 0,
      beer_weight2: 0,
      beer_volume2: 0,
      scale_stable_weight2: 0,
      last_pour_weight2: 0,
      last_pour_volume2: 0,

      // Push status
      ha: {},
      brewspy: {},
      brewlogger: {},
      barhelper: {}
    }
  },
  getters: {},
  actions: {
    load(callback) {
      logInfo('statusStore.load()', 'Fetching /api/status')
      fetch(global.baseURL + 'api/status', {
        signal: AbortSignal.timeout(global.fetchTimout)
      })
        .then((res) => res.json())
        .then((json) => {
          logDebug('statusStore.load()', json)
          this.id = json.id
          this.mdns = json.mdns
          this.wifi_ssid = json.wifi_ssid
          this.platform = json.platform
          this.app_ver = json.app_ver
          this.app_build = json.app_build
          this.weight_unit = json.weight_unit
          this.volume_unit = json.volume_unit
          this.temp_format = json.temp_format
          this.temperature = (Math.round(json.temperature * 100) / 100).toFixed(2) // C or F
          this.humidity = json.humidity
          this.pressure = json.pressure
          this.rssi = json.rssi
          this.ip = json.ip
          this.total_heap = Math.round(json.total_heap / 1024).toFixed(0)
          this.free_heap = Math.round(json.free_heap / 1024).toFixed(0)
          this.wifi_setup = json.wifi_setup
          this.scale_busy = json.scale_busy
          this.uptime_seconds = json.uptime_seconds
          this.uptime_minutes = json.uptime_minutes
          this.uptime_hours = json.uptime_hours
          this.uptime_days = json.uptime_days

          // Keg 1
          this.glass1 = new Number(json.glass1).toFixed(0)
          this.keg_volume1 = new Number(json.keg_volume1).toFixed(2)
          this.scale_factor1 = json.scale_factor1
          this.scale_weight1 = new Number(json.scale_weight1).toFixed(3)
          this.scale_raw1 = json.scale_raw1
          this.scale_offset1 = json.scale_offset1
          this.beer_weight1 = new Number(json.beer_weight1).toFixed(2)
          this.beer_volume1 = new Number(json.beer_volume1).toFixed(2)
          this.scale_stable_weight1 = new Number(json.scale_stable_weight1).toFixed(2)
          this.last_pour_weight1 = new Number(json.last_pour_weight1).toFixed(2)
          this.last_pour_volume1 = new Number(json.last_pour_volume1).toFixed(2)

          // Keg 2
          this.glass2 = new Number(json.glass2).toFixed(0)
          this.keg_volume2 = new Number(json.keg_volume2).toFixed(2)
          this.scale_factor2 = json.scale_factor2
          this.scale_weight2 = new Number(json.scale_weight2).toFixed(3)
          this.scale_raw2 = json.scale_raw2
          this.scale_offset2 = json.scale_offset2
          this.beer_weight2 = new Number(json.beer_weight2).toFixed(2)
          this.beer_volume2 = new Number(json.beer_volume2).toFixed(2)
          this.scale_stable_weight2 = new Number(json.scale_stable_weight2).toFixed(2)
          this.last_pour_weight2 = new Number(json.last_pour_weight2).toFixed(2)
          this.last_pour_volume2 = new Number(json.last_pour_volume2).toFixed(2)

          // Push status
          if (Object.prototype.hasOwnProperty.call(json, 'ha')) this.ha = json.ha
          if (Object.prototype.hasOwnProperty.call(json, 'brewspy')) this.brewspy = json.brewspy
          if (Object.prototype.hasOwnProperty.call(json, 'barhelper'))
            this.barhelper = json.barhelper
          if (Object.prototype.hasOwnProperty.call(json, 'brewlogger'))
            this.brewlogger = json.brewlogger

          logInfo('statusStore.load()', 'Fetching /api/status completed')
          callback(true)
        })
        .catch((err) => {
          logError('statusStore.load()', err)
          callback(false)
        })
    },
    auth(callback) {
      logInfo('statusStore.auth()', 'Fetching /api/auth')
      var base = btoa('gravitymon:password')

      fetch(global.baseURL + 'api/auth', {
        method: 'GET',
        headers: { Authorization: 'Basic ' + base },
        signal: AbortSignal.timeout(global.fetchTimout)
      })
        .then((res) => res.json())
        .then((json) => {
          logInfo('statusStore.auth()', 'Fetching /api/auth completed')
          callback(true, json)
        })
        .catch((err) => {
          logError('statusStore.auth()', err)
          callback(false)
        })
    },
    ping() {
      // logInfo("statusStore.ping()", "Fetching /api/ping")
      fetch(global.baseURL + 'api/ping', {
        method: 'GET',
        signal: AbortSignal.timeout(global.fetchTimout)
      })
        .then((res) => res.json())
        .then(() => {
          // logInfo("statusStore.ping()", "Fetching /api/auth completed")
          this.connected = true
        })
        .catch((err) => {
          logError('statusStore.ping()', err)
          this.connected = false
        })
    }
  }
})
