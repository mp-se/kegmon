/*
 * Project specific data objects, should contain configData and statusData as minimum
 *
 * (c) 2023-2024 Magnus Persson
 */

export var configData = {
  // Device configuration
  id: "7376ef",
  mdns: "tap2",
  temp_format: "C",
  weight_unit: "kg",
  volume_unit: "cl",
  dark_mode: false, 
  // Hardware
  ota_url: "",
  display_layout: 0,
  temp_sensor: 0,
  brewpi_url: "http://192.168.0.23",
  chamberctrl_url: "http://192.168.0.25",
  brewlogger_url: "http://192.168.0.24",
  scale_sensor: 0,
  display_driver: 0,
  pin_display_data: 33,
  pin_display_clock: 35,
  pin_scale1_data: 18,
  pin_scale1_clock: 16,
  pin_scale2_data: 7,
  pin_scale2_clock: 12,
  pin_temp_data: 11,
  pin_temp_power: 9,
  scale_deviation_increase: 1,
  scale_deviation_decrease: 0.1,
  scale_deviation_kalman: 0.05,
  scale_read_count: 5,
  scale_read_count_calibration: 30,
  scale_stable_count: 10,
  // Wifi
  wifi_portal_timeout: 120,
  wifi_connect_timeout: 20,
  wifi_ssid: "network A",
  wifi_ssid2: "",
  wifi_pass: "password",
  wifi_pass2: "mypass",
  // Push _ Generic
  push_timeout: 10,  
  brewfather_apikey: "", // "NipSwcZmguYM4bvuFReRtgAddesRbQkyYMFin7cIGybf4htng61oA5",
  brewfather_userkey: "", // "Urz3aoQ5KJhaH8883DIXWZlUoHpG42",
  brewspy_token1: "",
  brewspy_token2: "",
  barhelper_apikey: '',
  barhelper_monitor1: 'Monitor 1',
  barhelper_monitor2: 'Monitor 2',
// Push _ Http Post 1
  http_post_target: "http://192.168.1.10:9090/api/v1/ZYfjlUNeiuyu9N/telemetry",
  http_post_header1: "Auth: Basic T7IF9DD9fF3RDddE=",
  http_post_header2: "Auth: Advanced T7IF9DD9fF3RDddE=",
  // Push - Http Post 2
  http_post2_target: "http://192.168.1.10/ispindel",
  http_post2_header1: "Header: Second",
  http_post2_header2: "Header: First",
  // Push - Http Get
  http_get_target: "http://192.168.1.10/ispindel",
  http_get_header1: "Header: Fourth",
  http_get_header2: "Header: Fifth",
  // Push - Influx
  influxdb2_target: "http://192.168.1.10:8086",
  influxdb2_org: "hello",
  influxdb2_bucket: "spann",
  influxdb2_token: "OijkU((jhfkh",
  // Push - MQTT
  mqtt_target: "192.168.1.20",
  mqtt_port: 1883,
  mqtt_user: "kegmon",
  mqtt_pass: "testpass",
  // Keg 1
  scale_temp_formula1: "",
  scale_factor1: -21648.68945,
  scale_offset1: 26689,
  keg_weight1: 5,
  keg_volume1: 19,
  glass_volume1: 0.4,
  beer_name1: "West Coast IPA",
  beer_id1: "2",
  beer_abv1: 7,
  beer_fg1: 1.01,
  beer_ebc1: 5,
  beer_ibu1: 26,
  // Keg 2
  scale_temp_formula2: "",
  scale_factor2: -23080.98438,
  scale_offset2: -108721,
  keg_weight2: 5,
  keg_volume2: 19,
  glass_volume2: 0.4,
  beer_name2: "Helles Festbier",
  beer_id2: "3",
  beer_abv2: 5.5,
  beer_fg2: 1.01,
  beer_ebc2: 5,
  beer_ibu2: 28,
}

export var statusData = {
  // Device
  mdns: "tap",
  id: "7376ef",
  wifi_ssid: "@home",
  platform: "esp32s2",
  app_ver: "1.0.0",
  app_build: "beta2",
  weight_unit: "kg",
  volume_unit: "cl",
  temp_format: "C",

  // Data
  temp: 22.4,
  rssi: -76,
  total_heap: 1000,
  free_heap: 500,
  ip: "192.0.0.1",
  wifi_setup: false,

  // Scale 1
  scale_factor1: -21648.68945,
  scale_weight1: 16.39,
  scale_raw1: 0,
  scale_offset1: 26689,
  beer_weight1: 11.37,
  beer_volume1: 1126,
  scale_stable_weight1: 16.27,
  last_pour_weight1: 0.11,
  last_pour_volume1: 11,
  glass1: 27.9,
  keg_volume1: 1900,

  // Scale 2
  scale_factor2: -23080.98438,
  scale_weight2: 0.19,
  scale_raw2: 0,
  scale_offset2: -108721,
  beer_weight2: -4.81,
  beer_volume2: -476,
  scale_stable_weight2: 0.24,
  last_pour_weight2: 14.01,
  last_pour_volume2: 1387,
  glass2: 0,
  keg_volume2: 1900,

  // Push testing
  ha: {
     push_age: 288986,
      push_status: false,
      push_code: 0,
      push_response: "",
      push_used: false
  },
  barhelper: {
      push_age: 288986,
      push_status: false,
      push_code: 0,
      push_response: "",
      push_used: false
  },
  brewspy: {
      push_age: 288986,
      push_status: false,
      push_code: 0,
      push_response: "",
      push_used: false
  },
}

// EOF