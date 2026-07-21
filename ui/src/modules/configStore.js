import { defineStore } from 'pinia'
import { global, saveConfigState, getConfigChanges } from '@/modules/pinia'
import { getErrorString } from '@/modules/utils'
import { logDebug, logError, logInfo } from '@/modules/logger'

export const useConfigStore = defineStore('config', {
  state: () => {
    return {
      // Device
      id: '',
      mdns: '',
      temp_format: '',
      weight_unit: '',
      volume_unit: '',
      dark_mode: false,
      // Hardware
      ota_url: '',
      display_layout: 0,
      temp_sensor: 0,
      brewpi_url: '',
      chamberctrl_url: '',
      scale_sensor: 0,
      display_driver: 0,
      pin_display_data: 0,
      pin_display_clock: 0,
      pin_scale1_data: 0,
      pin_scale1_clock: 0,
      pin_scale2_data: 0,
      pin_scale2_clock: 0,
      pin_temp_data: 0,
      pin_temp_power: 0,
      scale_deviation_increase: 0,
      scale_deviation_decrease: 0,
      scale_deviation_kalman: 0,
      scale_read_count: 0,
      scale_read_count_calibration: 0,
      scale_stable_count: 0,
      // Wifi
      wifi_portal_timeout: 0,
      wifi_connect_timeout: 0,
      wifi_ssid: '',
      wifi_ssid2: '',
      wifi_pass: '',
      wifi_pass2: '',
      // Push - Generic
      push_timeout: 10,
      brewfather_apikey: '',
      brewfather_userkey: '',
      brewspy_token1: '',
      brewspy_token2: '',
      barhelper_apikey: '',
      barhelper_monitor1: '',
      barhelper_monitor2: '',
      brewlogger_url: '',
      // Push - Http Post 1
      http_post_target: '',
      http_post_header1: '',
      http_post_header2: '',
      // Push - Http Post 2
      http_post2_target: '',
      http_post2_header1: '',
      http_post2_header2: '',
      // Push - Http Get
      http_get_target: '',
      http_get_header1: '',
      http_get_header2: '',
      // Push - Influx
      influxdb2_target: '',
      influxdb2_org: '',
      influxdb2_bucket: '',
      influxdb2_token: '',
      // Push - MQTT
      mqtt_target: '',
      mqtt_port: '',
      mqtt_user: '',
      mqtt_pass: '',
      // Keg 1
      scale_temp_formula1: '',
      scale_factor1: 0,
      scale_offset1: 0,
      keg_weight1: 0,
      keg_volume1: 0,
      glass_volume1: 0,
      beer_name1: '',
      beer_id1: '',
      beer_abv1: 0,
      beer_fg1: 0,
      beer_ebc1: 0,
      beer_ibu1: 0,
      // Keg 2
      scale_temp_formula2: '',
      scale_factor2: 0,
      scale_offset2: 0,
      keg_weight2: 0,
      keg_volume2: 0,
      glass_volume2: 0,
      beer_name2: '',
      beer_id2: '',
      beer_abv2: 0,
      beer_fg2: 0,
      beer_ebc2: 0,
      beer_ibu2: 0
    }
  },
  getters: {
    getVolumeUnit() {
      return this.volume_unit == 'cl' ? 'cl' : 'fl. oz'
    },
    getWeightUnit() {
      return this.weight_unit == 'kg' ? 'kg' : 'lbs'
    },
    getTempUnit() {
      return this.temp_format == 'C' ? '°C' : '°F'
    }
  },
  actions: {
    toJson() {
      logInfo('configStore.toJSON()')
      var dest = {}

      for (var key in this.$state) {
        if (!key.startsWith('$')) {
          dest[key] = this[key]
        }
      }

      logInfo('configStore.toJSON()', dest)
      return JSON.stringify(dest, null, 2)
    },
    load(callback) {
      global.disabled = true
      logInfo('configStore.load()', 'Fetching /api/config')
      fetch(global.baseURL + 'api/config', {
        method: 'GET',
        headers: { Authorization: global.token },
        signal: AbortSignal.timeout(global.fetchTimout)
      })
        .then((res) => res.json())
        .then((json) => {
          logDebug('configStore.load()', json)
          global.disabled = false
          this.id = json.id
          // Device
          this.mdns = json.mdns
          this.temp_format = json.temp_format
          this.weight_unit = json.weight_unit
          this.volume_unit = json.volume_unit
          this.dark_mode = json.dark_mode
          // Hardware
          this.ota_url = json.ota_url
          this.display_layout = json.display_layout
          this.temp_sensor = json.temp_sensor
          this.brewpi_url = json.brewpi_url
          this.chamberctrl_url = json.chamberctrl_url
          this.scale_sensor = json.scale_sensor
          this.display_driver = json.display_driver
          this.pin_display_data = json.pin_display_data
          this.pin_display_clock = json.pin_display_clock
          this.pin_scale1_data = json.pin_scale1_data
          this.pin_scale1_clock = json.pin_scale1_clock
          this.pin_scale2_data = json.pin_scale2_data
          this.pin_scale2_clock = json.pin_scale2_clock
          this.pin_temp_data = json.pin_temp_data
          this.pin_temp_power = json.pin_temp_power
          this.scale_deviation_increase = json.scale_deviation_increase
          this.scale_deviation_decrease = json.scale_deviation_decrease
          this.scale_deviation_kalman = json.scale_deviation_kalman
          this.scale_read_count = json.scale_read_count
          this.scale_read_count_calibration = json.scale_read_count_calibration
          this.scale_stable_count = json.scale_stable_count
          // Wifi
          this.wifi_portal_timeout = json.wifi_portal_timeout
          this.wifi_connect_timeout = json.wifi_connect_timeout
          this.wifi_ssid = json.wifi_ssid
          this.wifi_ssid2 = json.wifi_ssid2
          this.wifi_pass = json.wifi_pass
          this.wifi_pass2 = json.wifi_pass2
          // Push - Generic
          this.push_timeout = json.push_timeout
          this.brewfather_apikey = json.brewfather_apikey
          this.brewfather_userkey = json.brewfather_userkey
          this.brewspy_token1 = json.brewspy_token1
          this.brewspy_token2 = json.brewspy_token2
          this.barhelper_apikey = json.barhelper_apikey
          this.barhelper_monitor1 = json.barhelper_monitor1
          this.barhelper_monitor2 = json.barhelper_monitor2
          this.brewlogger_url = json.brewlogger_url
          // Push - Http Post 1
          this.http_post_target = json.http_post_target
          this.http_post_header1 = json.http_post_header1
          this.http_post_header2 = json.http_post_header2
          // Push - Http Post 2
          this.http_post2_target = json.http_post2_target
          this.http_post2_header1 = json.http_post2_header1
          this.http_post2_header2 = json.http_post2_header2
          // Push - Http Get
          this.http_get_target = json.http_get_target
          this.http_get_header1 = json.http_get_header1
          this.http_get_header2 = json.http_get_header2
          // Push - Influx
          this.influxdb2_target = json.influxdb2_target
          this.influxdb2_org = json.influxdb2_org
          this.influxdb2_bucket = json.influxdb2_bucket
          this.influxdb2_token = json.influxdb2_token
          // Push - MQTT
          this.mqtt_target = json.mqtt_target
          this.mqtt_port = json.mqtt_port
          this.mqtt_user = json.mqtt_user
          this.mqtt_pass = json.mqtt_pass
          // Keg 1
          this.scale_temp_formula1 = json.scale_temp_formula1
          this.scale_factor1 = json.scale_factor1
          this.scale_offset1 = json.scale_offset1
          this.keg_weight1 = json.keg_weight1
          this.keg_volume1 = json.keg_volume1
          this.glass_volume1 = json.glass_volume1
          this.beer_name1 = json.beer_name1
          this.beer_abv1 = json.beer_abv1
          this.beer_fg1 = json.beer_fg1
          this.beer_ebc1 = json.beer_ebc1
          this.beer_ibu1 = json.beer_ibu1
          // Keg 2
          this.scale_temp_formula2 = json.scale_temp_formula2
          this.scale_factor2 = json.scale_factor2
          this.scale_offset2 = json.scale_offset2
          this.keg_weight2 = json.keg_weight2
          this.keg_volume2 = json.keg_volume2
          this.glass_volume2 = json.glass_volume2
          this.beer_name2 = json.beer_name2
          this.beer_abv2 = json.beer_abv2
          this.beer_fg2 = json.beer_fg2
          this.beer_ebc2 = json.beer_ebc2
          this.beer_ibu2 = json.beer_ibu2
          callback(true)
        })
        .catch((err) => {
          global.disabled = false
          logError('configStore.load()', err)
          callback(false)
        })
    },
    sendConfig(callback) {
      global.disabled = true
      logInfo('configStore.sendConfig()', 'Sending /api/config')

      var data = getConfigChanges()
      logDebug('configStore.sendConfig()', data)

      if (JSON.stringify(data).length == 2) {
        logInfo('configStore.sendConfig()', 'No config data to store, skipping step')
        global.disabled = false
        callback(true)
        return
      }

      fetch(global.baseURL + 'api/config', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json', Authorization: global.token },
        body: JSON.stringify(data),
        signal: AbortSignal.timeout(global.fetchTimout)
      })
        .then((res) => {
          global.disabled = false
          if (res.status != 200) {
            logError('configStore.sendConfig()', 'Sending /api/config failed', res.status)
            callback(false)
          } else {
            logInfo('configStore.sendConfig()', 'Sending /api/config completed')
            callback(true)
          }
        })
        .catch((err) => {
          logError('configStore.sendConfig()', err)
          callback(false)
          global.disabled = false
        })
    },
    sendPushTest(data, callback) {
      global.disabled = true
      logInfo('configStore.sendPushTest()', 'Sending /api/push')
      fetch(global.baseURL + 'api/push', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json', Authorization: global.token },
        body: JSON.stringify(data),
        signal: AbortSignal.timeout(global.fetchTimout)
      })
        .then((res) => {
          if (res.status != 200) {
            logError('configStore.sendPushTest()', 'Sending /api/push failed')
            callback(false)
          } else {
            logInfo('configStore.sendPushTest()', 'Sending /api/push completed')
            callback(true)
          }
        })
        .catch((err) => {
          logError('configStore.sendPushTest()', err)
          callback(false)
        })
    },
    getPushTestStatus(callback) {
      logInfo('configStore.getPushTest()', 'Fetching /api/push/status')
      fetch(global.baseURL + 'api/push/status', {
        signal: AbortSignal.timeout(global.fetchTimout)
      })
        .then((res) => res.json())
        .then((json) => {
          logDebug('configStore.getPushTest()', json)
          logInfo('configStore.getPushTest()', 'Fetching /api/push/status completed')
          callback(true, json)
        })
        .catch((err) => {
          logError('configStore.getPushTest()', err)
          callback(false, null)
        })
    },
    sendWifiScan(callback) {
      global.disabled = true
      logInfo('configStore.sendWifiScan()', 'Sending /api/wifi')
      fetch(global.baseURL + 'api/wifi', {
        headers: { Authorization: global.token },
        signal: AbortSignal.timeout(global.fetchTimout)
      })
        .then((res) => {
          if (res.status != 200) {
            logError('configStore.sendWifiScan()', 'Sending /api/wifi failed')
            callback(false)
          } else {
            logInfo('configStore.sendWifiScan()', 'Sending /api/wifi completed')
            callback(true)
          }
        })
        .catch((err) => {
          logError('configStore.sendWifiScan()', err)
          callback(false)
        })
    },
    getWifiScanStatus(callback) {
      logInfo('configStore.getWifiScanStatus()', 'Fetching /api/wifi/status')
      fetch(global.baseURL + 'api/wifi/status', {
        method: 'GET',
        headers: { Authorization: global.token },
        signal: AbortSignal.timeout(global.fetchTimout)
      })
        .then((res) => res.json())
        .then((json) => {
          logDebug('configStore.getWifiScanStatus()', json)
          logInfo('configStore.getWifiScanStatus()', 'Fetching /api/wifi/status completed')
          callback(true, json)
        })
        .catch((err) => {
          logError('configStore.getWifiScanStatus()', err)
          callback(false, null)
        })
    },
    sendHardwareScan(callback) {
      global.disabled = true
      logInfo('configStore.sendHardwareScan()', 'Sending /api/hardware')
      fetch(global.baseURL + 'api/hardware', {
        headers: { Authorization: global.token },
        signal: AbortSignal.timeout(global.fetchTimout)
      })
        .then((res) => {
          if (res.status != 200) {
            logError('configStore.sendHardwareScan()', 'Sending /api/hardware failed')
            callback(false)
          } else {
            logInfo('configStore.sendHardwareScan()', 'Sending /api/hardware completed')
            callback(true)
          }
        })
        .catch((err) => {
          logError('configStore.sendHardwareScan()', err)
          callback(false)
        })
    },
    getHardwareScanStatus(callback) {
      logInfo('configStore.getHardwareScanStatus()', 'Fetching /api/hardware/status')
      fetch(global.baseURL + 'api/hardware/status', {
        method: 'GET',
        headers: { Authorization: global.token },
        signal: AbortSignal.timeout(global.fetchTimout)
      })
        .then((res) => res.json())
        .then((json) => {
          logDebug('configStore.getHardwareScanStatus()', json)
          logInfo('configStore.getHardwareScanStatus()', 'Fetching /api/hardware/status completed')
          callback(true, json)
        })
        .catch((err) => {
          logError('configStore.getHardwareScanStatus()', err)
          callback(false, null)
        })
    },
    saveAll() {
      global.clearMessages()
      global.disabled = true
      this.sendConfig((success) => {
        if (!success) {
          global.disabled = false
          global.messageError = 'Failed to store configuration to device'
        } else {
          global.messageSuccess = 'Configuration has been saved to device'
          saveConfigState()
        }
      })
    },
    sendFilesystemRequest(data, callback) {
      global.disabled = true
      logInfo('configStore.sendFilesystemRequest()', 'Sending /api/filesystem')
      fetch(global.baseURL + 'api/filesystem', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json', Authorization: global.token },
        body: JSON.stringify(data),
        signal: AbortSignal.timeout(global.fetchTimout)
      })
        .then((res) => res.text())
        .then((text) => {
          logDebug('configStore.sendFilesystemRequest()', text)
          callback(true, text)
        })
        .catch((err) => {
          logError('configStore.sendFilesystemRequest()', err)
          callback(false, '')
        })
    },
    runPushTest(data, callback) {
      global.disabled = true
      this.sendPushTest(data, (success) => {
        if (success) {
          var check = setInterval(() => {
            this.getPushTestStatus((success, data) => {
              if (success) {
                if (data.status) {
                  // test is still running, just wait for next check
                } else {
                  global.disabled = false
                  if (!data.push_enabled) {
                    global.messageWarning =
                      'No endpoint is defined for this target. Cannot run test.'
                  } else if (!data.success) {
                    global.messageError =
                      'Test failed with error code ' + getErrorString(data.last_error)
                  } else {
                    global.messageSuccess = 'Test was successful'
                  }

                  callback(true)
                  clearInterval(check)
                }
              } else {
                global.disabled = false
                global.messageError = 'Failed to get push test status'
                callback(false)
                clearInterval(check)
              }
            })
          }, 2000)
        } else {
          global.messageError = 'Failed to start push test'
          global.disabled = false
          callback(false)
        }
      })
    },
    runWifiScan(callback) {
      global.disabled = true
      this.sendWifiScan((success) => {
        if (success) {
          var check = setInterval(() => {
            this.getWifiScanStatus((success, data) => {
              if (success) {
                if (data.status) {
                  // test is still running, just wait for next check
                } else {
                  global.disabled = false
                  callback(data.success, data)
                  clearInterval(check)
                }
              } else {
                global.disabled = false
                global.messageError = 'Failed to get wifi scan status'
                callback(false)
                clearInterval(check)
              }
            })
          }, 2000)
        } else {
          global.disabled = false
          global.messageError = 'Failed to start wifi scan'
          callback(false)
        }
      })
    },
    runHardwareScan(callback) {
      global.disabled = true
      this.sendHardwareScan((success) => {
        if (success) {
          var check = setInterval(() => {
            this.getHardwareScanStatus((success, data) => {
              if (success) {
                if (data.status) {
                  // test is still running, just wait for next check
                } else {
                  global.disabled = false
                  callback(data.success, data)
                  clearInterval(check)
                }
              } else {
                global.disabled = false
                global.messageError = 'Failed to get hardware scan status'
                callback(false)
                clearInterval(check)
              }
            })
          }, 2000)
        } else {
          global.disabled = false
          global.messageError = 'Failed to start hardware scan'
          callback(false)
        }
      })
    }
  }
})
