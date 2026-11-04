<template>
  <div class="container">
    <p></p>
    <p class="h3">Device - Hardware</p>
    <hr />

    <form @submit.prevent="save" class="needs-validation" novalidate>
      <div class="row">
        <div class="col-md-6">
          <BsInputRadio
            v-model="config.display_driver"
            :options="displayDriverOptions"
            label="Display driver"
            width=""
            :disabled="global.disabled"
          ></BsInputRadio>
        </div>
        <div class="col-md-6">
          <BsInputRadio
            v-model="config.scale_sensor"
            :options="scaleSensorOptions"
            label="Scale driver"
            width=""
            :disabled="global.disabled"
          ></BsInputRadio>
        </div>
        <div class="col-md-12">
          <BsInputRadio
            v-model="config.temp_sensor"
            :options="tempSensorOptions"
            label="Temperature sensor"
            width=""
            :disabled="global.disabled"
          ></BsInputRadio>
        </div>

        <div class="col-md-6" v-if="config.temp_sensor == 3">
          <BsInputText
            v-model="config.brewpi_url"
            type="url"
            maxlength="120"
            label="BrewPI URL"
            :disabled="global.disabled"
          />
        </div>
        <div class="col-md-6" v-if="config.temp_sensor == 4">
          <BsInputText
            v-model="config.chamberctrl_url"
            type="url"
            maxlength="120"
            label="Chamber Controller URL"
            :disabled="global.disabled"
          />
        </div>

        <div class="col-md-12">
          <BsInputRadio
            v-model="config.display_layout"
            :options="displayLayoutOptions"
            label="Display layout"
            width=""
            help="Selects the layout and data on the displays"
            :disabled="global.disabled"
          ></BsInputRadio>
        </div>

        <!-- TODO: Add example on how display looks like -->

        <div class="col-md-12">
          <hr />
        </div>

        <div class="col-md-4">
          <BsInputNumber
            v-model="config.scale_deviation_increase"
            label="Scale deviation increase"
            min=".05"
            max="1.0"
            step=".05"
            :unit="config.weight_unit"
            help="Default 0.5 kg"
            :disabled="global.disabled"
          />
        </div>
        <div class="col-md-4">
          <BsInputNumber
            v-model="config.scale_deviation_decrease"
            label="Scale deviation decrease"
            min=".05"
            max="1.0"
            step=".05"
            :unit="config.weight_unit"
            help="Default 0.1 kg"
            :disabled="global.disabled"
          />
        </div>
        <div class="col-md-4">
          <BsInputNumber
            v-model="config.scale_deviation_kalman"
            label="Scale deviation kalman"
            min=".01"
            max="0.1"
            step=".01"
            :unit="config.weight_unit"
            help="Default 0.04 kg"
            :disabled="global.disabled"
          />
        </div>
        <div class="col-md-4">
          <BsInputNumber
            v-model="config.scale_stable_count"
            label="Scale stable count"
            min="6"
            max="30"
            step="1"
            help="Number of stable readings required for a new level to be set"
            :disabled="global.disabled"
          />
        </div>

        <div class="col-md-4">
          <BsInputNumber
            v-model="config.scale_read_count"
            label="Scale sample count"
            min="1"
            max="50"
            step="1"
            help="Number of samples from the scale driver"
            :disabled="global.disabled"
          />
        </div>

        <div class="col-md-4">
          <BsInputNumber
            v-model="config.scale_read_count_calibration"
            label="Scale sample count during calibration"
            min="1"
            max="100"
            step="1"
            help="Number of samples from the scale driver when doing a calibration"
            :disabled="global.disabled"
          />
        </div>

        <div class="col-md-12">
          <hr />
        </div>

        <div class="col-md-3">
          <BsSelect
            v-model="config.pin_display_data"
            :options="
              status.platform == 'esp8266'
                ? pinEsp8266Options
                : status.platform == 'esp32s3'
                  ? pinEsp32s3Options
                  : pinEsp32s2Options
            "
            label="Display/I2C - Data"
            width="4"
            :disabled="global.disabled || !enablePin"
          ></BsSelect>
        </div>
        <div class="col-md-3">
          <BsSelect
            v-model="config.pin_display_clock"
            :options="
              status.platform == 'esp8266'
                ? pinEsp8266Options
                : status.platform == 'esp32s3'
                  ? pinEsp32s3Options
                  : pinEsp32s2Options
            "
            label="Display/I2C - Clock"
            width="4"
            :disabled="global.disabled || !enablePin"
          ></BsSelect>
        </div>

        <div class="col-md-3">
          <BsSelect
            v-model="config.pin_temp_data"
            :options="
              status.platform == 'esp8266'
                ? pinEsp8266Options
                : status.platform == 'esp32s3'
                  ? pinEsp32s3Options
                  : pinEsp32s2Options
            "
            label="Temperature - Data"
            width="4"
            :disabled="global.disabled || !enablePin"
          ></BsSelect>
        </div>
        <div class="col-md-3">
          <BsSelect
            v-model="config.pin_temp_power"
            :options="
              status.platform == 'esp8266'
                ? pinEsp8266Options
                : status.platform == 'esp32s3'
                  ? pinEsp32s3Options
                  : pinEsp32s2Options
            "
            label="Temperature - Power"
            width="4"
            :disabled="global.disabled || !enablePin"
          ></BsSelect>
        </div>

        <div class="col-md-3">
          <BsSelect
            v-model="config.pin_scale1_data"
            :options="
              status.platform == 'esp8266'
                ? pinEsp8266Options
                : status.platform == 'esp32s3'
                  ? pinEsp32s3Options
                  : pinEsp32s2Options
            "
            label="Scale 1 - Data"
            width="4"
            :disabled="global.disabled || !enablePin"
          ></BsSelect>
        </div>
        <div class="col-md-3">
          <BsSelect
            v-model="config.pin_scale1_clock"
            :options="
              status.platform == 'esp8266'
                ? pinEsp8266Options
                : status.platform == 'esp32s3'
                  ? pinEsp32s3Options
                  : pinEsp32s2Options
            "
            label="Scale 1 - Clock"
            width="4"
            :disabled="global.disabled || !enablePin"
          ></BsSelect>
        </div>

        <div class="col-md-3">
          <BsSelect
            v-model="config.pin_scale2_data"
            :options="
              status.platform == 'esp8266'
                ? pinEsp8266Options
                : status.platform == 'esp32s3'
                  ? pinEsp32s3Options
                  : pinEsp32s2Options
            "
            label="Scale 2 - Data"
            width="4"
            :disabled="global.disabled || !enablePin"
          ></BsSelect>
        </div>
        <div class="col-md-3">
          <BsSelect
            v-model="config.pin_scale2_clock"
            :options="
              status.platform == 'esp8266'
                ? pinEsp8266Options
                : status.platform == 'esp32s3'
                  ? pinEsp32s3Options
                  : pinEsp32s2Options
            "
            label="Scale 2 - Clock"
            width="4"
            :disabled="global.disabled || !enablePin"
          ></BsSelect>
        </div>

        <div class="col-md-12">
          <BsInputSwitch
            v-model="enablePin"
            label="Enable pin configuration"
            width=""
            :disabled="global.disabled"
          ></BsInputSwitch>
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
import { ref } from 'vue'
import { validateCurrentForm, restart } from '@/modules/utils'
import { global, config, status } from '@/modules/pinia'

const enablePin = ref(false)

const displayLayoutOptions = ref([
  { label: 'Default', value: 0 },
  { label: 'Graphical', value: 1 },
  { label: 'Graphical (Single Unit)', value: 2 },
  { label: 'Hardware statistics', value: 3 }
])

const tempSensorOptions = ref([
  { label: 'DHT22', value: 0 },
  { label: 'DS18B20', value: 1 },
  { label: 'BME280', value: 2 },
  { label: 'BrewPI (Network)', value: 3 },
  { label: 'Chamber Ctrl (Network)', value: 4 }
])

const scaleSensorOptions = ref([
  { label: 'HX711', value: 0 },
  { label: 'NAU782', value: 1 }
])

const displayDriverOptions = ref([
  { label: 'OLED 1306 0.96" 128x64', value: 0 },
  { label: 'LCD 20x4', value: 1 }
])

const pinEsp8266Options = ref([
  { label: 'D0', value: 16 },
  { label: 'D1', value: 5 },
  { label: 'D2', value: 4 },
  { label: 'D3', value: 0 },
  { label: 'D4', value: 2 },
  { label: 'D5', value: 14 },
  { label: 'D6', value: 12 },
  { label: 'D7', value: 13 },
  { label: 'D8', value: 15 },
  { label: 'TX', value: 3 },
  { label: 'RX', value: 1 }
])

const pinEsp32s2Options = ref([
  { label: '3', value: 3 },
  { label: '4', value: 3 },
  { label: '5', value: 5 },
  { label: '7', value: 7 },
  { label: '9', value: 9 },
  { label: '11', value: 11 },
  { label: '12', value: 12 },
  { label: '16', value: 16 },
  { label: '18', value: 18 },
  { label: '33', value: 33 },
  { label: '35', value: 35 },
  { label: '37', value: 37 },
  { label: '39', value: 39 }
])

const pinEsp32s3Options = ref([
  { label: '2', value: 2 },
  { label: '4', value: 4 },
  { label: '10', value: 10 },
  { label: '11', value: 11 },
  { label: '12', value: 12 },
  { label: '13', value: 13 },
  { label: '16', value: 16 },
  { label: '18', value: 18 },
  { label: '35', value: 35 },
  { label: '36', value: 36 },
  { label: '43', value: 43 },
  { label: '44', value: 44 }
])

const save = () => {
  if (!validateCurrentForm()) return

  global.clearMessages()
  config.saveAll()
}
</script>
