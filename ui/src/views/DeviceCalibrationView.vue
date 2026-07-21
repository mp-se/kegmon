<template>
  <div class="container">
    <p></p>
    <p class="h2">Device - Calibration</p>
    <hr />

    <div class="row">
      <div class="col-md-12">
        <p class="h3">Step 1 - Select scale</p>
      </div>
      <div class="col-md-12">
        <BsInputRadio
          v-model="scale"
          :options="scaleOptions"
          label="Select which scale to calibrate"
          width=""
          :disabled="global.disabled || state > 1"
        ></BsInputRadio>
      </div>
      <div class="row gy-2" v-if="state == 1">
        <div class="col-md-2">
          <button
            @click="step1()"
            type="button"
            class="btn btn-secondary"
            :disabled="global.disabled"
          >
            Select scale
          </button>
        </div>
      </div>
      <p></p>
    </div>

    <div class="row" v-if="state > 1">
      <hr />
      <div class="col-md-12">
        <p class="h3">Step 2 - Tare scale</p>
      </div>
      <div class="col-md-12">
        <p>Remove any object from the scale and press the tare button to reset it to zero.</p>
      </div>
      <div class="row gy-2" v-if="state == 2">
        <div class="col-md-12">
          <button
            @click="step2()"
            type="button"
            class="btn btn-secondary"
            :disabled="global.disabled"
          >
            Tare scale</button
          >&nbsp;

          <button
            @click="back()"
            type="button"
            class="btn btn-secondary"
            :disabled="global.disabled"
          >
            Back
          </button>
        </div>
      </div>
      <p></p>
    </div>

    <div class="row" v-if="state > 2">
      <hr />
      <div class="col-md-12">
        <p class="h3">Step 3 - Calibrate scale</p>
      </div>
      <div class="col-md-12">
        <p>
          Place a known weight on the scale and press the factor button. The scale will calculate a
          factor and complete calibration. Use any known weight.
        </p>
      </div>

      <div class="col-md-12">
        <BsInputNumber
          v-model="weight"
          width="5"
          label="Calibration weight"
          min="0"
          max="25"
          step=".01"
          :unit="config.weight_unit"
          :disabled="global.disabled || state != 3"
        />
      </div>

      <div class="row gy-2" v-if="state == 3">
        <div class="col-md-12">
          <button
            @click="step3()"
            type="button"
            class="btn btn-secondary"
            :disabled="global.disabled"
          >
            Calculate factor</button
          >&nbsp;

          <button
            @click="back()"
            type="button"
            class="btn btn-secondary"
            :disabled="global.disabled"
          >
            Back
          </button>
        </div>
        <p></p>
      </div>
    </div>

    <div class="row" v-if="state > 3">
      <hr />
      <div class="col-md-12">
        <p class="h3">Step 4 - Validate</p>
      </div>
      <div class="col-md-12">
        <p>
          The entered weight was <b>{{ new Number(weight).toFixed(2) }}</b> and the measured weight
          after calibration was <b>{{ new Number(scaleStatus.weight).toFixed(2) }}</b> (Deviation:
          <b>{{ new Number(((scaleStatus.weight - weight) / weight) * 100).toFixed(0) }}</b> %), if
          you want to change then use the back button.
        </p>
      </div>
      <div class="row gy-2">
        <div class="col-md-12">
          <button
            @click="back()"
            type="button"
            class="btn btn-secondary"
            :disabled="global.disabled"
          >
            Back</button
          >&nbsp;

          <button
            @click="begin()"
            type="button"
            class="btn btn-secondary"
            :disabled="global.disabled"
          >
            Start over
          </button>
        </div>
      </div>
    </div>

    <div class="row p-3">
      <div class="col-md-12">
        <p></p>
      </div>

      <div :class="getClass()">Offset: {{ scaleStatus.offset }}</div>
      <div :class="getClass()">Factor: {{ scaleStatus.factor }}</div>
      <div :class="getClass()">Raw: {{ scaleStatus.raw }}</div>
      <div :class="getClass()">Weight: {{ scaleStatus.weightString }}</div>
    </div>
  </div>
</template>

<script setup>
import { onBeforeMount, ref } from 'vue'
import { weightKgToLbs } from '@/modules/utils'
import { global, config } from '@/modules/pinia'
import { logDebug, logError, logInfo } from '@/modules/logger'

const state = ref(1)
const scale = ref(1)
const weight = ref(0)
const scaleStatus = ref({
  offset: 0,
  factor: 0,
  weight: 0,
  weightString: '',
  raw: 0
})

const scaleOptions = ref([
  { label: 'Scale 1', value: 1 },
  { label: 'Scale 2', value: 2 }
])

function getClass() {
  if (config.dark_mode) return 'col-md-3 p-3 border bg-dark text-white'
  return 'col-md-3 p-3 border bg-light'
}

function back() {
  global.clearMessages()
  state.value = state.value - 1
}

function begin() {
  global.clearMessages()
  state.value = 1
}

function step1() {
  global.clearMessages()
  state.value = 2
}

function saveScaleValues(json) {
  if (scale.value == 1) {
    scaleStatus.value.offset = json.scale_offset1
    scaleStatus.value.factor = json.scale_factor1
    scaleStatus.value.raw = json.scale_raw1
    scaleStatus.value.weight = json.scale_weight1
  } else {
    scaleStatus.value.offset = json.scale_offset2
    scaleStatus.value.factor = json.scale_factor2
    scaleStatus.value.raw = json.scale_raw2
    scaleStatus.value.weight = json.scale_weight2
  }

  scaleStatus.value.weightString =
    new Number(
      config.weight_unit == 'kg'
        ? scaleStatus.value.weight
        : weightKgToLbs(scaleStatus.value.weight)
    ).toFixed(3) +
    ' ' +
    config.weight_unit
}

/*
 * Reset scale
 */
async function step2() {
  global.disabled = true
  global.clearMessages()

  try {
    await sendTare((success, json) => {
      logDebug('DeviceCalibrationView.step2()', 'Tare', json)
      if (!success) {
        throw new Error('Failed to tare scale ' + scale.value)
      }
    })

    var timer = setInterval(async () => {
      logDebug('DeviceCalibrationView.step2()', 'Checking scale status')

      await getScaleStatus((success, json) => {
        logDebug('DeviceCalibrationView.step2()', 'Status', json)
        if (!success) {
          throw new Error('Failed to fetch scale status ' + scale.value)
        }

        saveScaleValues(json)

        if (!json.scale_busy) {
          global.messageSuccess = 'Scale tare completed'
          clearInterval(timer)
          state.value = 3
          global.disabled = false
        }
      })
    }, 2000)
  } catch (e) {
    global.messageError = e
    global.disabled = false
  }
}

/*
 * Perform calibration
 */
async function step3() {
  global.clearMessages()

  if (weight.value == 0) {
    global.messageError = 'You need to supply a weight larger than 0 (zero)'
    return
  }

  global.disabled = true

  try {
    await sendFactor((success, json) => {
      logDebug('DeviceCalibrationView.step3()', 'Factor', json)
      if (!success) {
        throw new Error('Failed to tare scale ' + scale.value)
      }
    })

    var timer = setInterval(async () => {
      logDebug('DeviceCalibrationView.step3()', 'Checking scale status')

      await getScaleStatus((success, json) => {
        logDebug('DeviceCalibrationView.step3()', 'Status', json)
        if (!success) {
          throw new Error('Failed to fetch scale status ' + scale.value)
        }

        saveScaleValues(json)

        if (!json.scale_busy) {
          global.messageSuccess = 'Scale factor calculation completed'
          clearInterval(timer)
          state.value = 4
          global.disabled = false
        }
      })
    }, 2000)
  } catch (e) {
    global.messageError = e
    global.disabled = false
  }
}

onBeforeMount(() => {
  state.value = 1
})

async function sendTare(callback) {
  logInfo('DeviceCalibrationView.sendTare()', 'Sending /api/scale/tare')

  const res = await fetch(global.baseURL + 'api/scale/tare', {
    method: 'POST',
    headers: { 'Content-Type': 'application/json', Authorization: global.token },
    body: JSON.stringify({ scale_index: scale.value }),
    signal: AbortSignal.timeout(global.fetchTimout)
  })

  if (!res.ok) {
    logError('DeviceCalibrationView.sendTare()', 'Failed with', res.ok)
    callback(false, {})
    return
  }

  const json = await res.json()
  callback(true, json)
}

async function sendFactor(callback) {
  logInfo('DeviceCalibrationView.sendFactor()', 'Sending /api/scale/factor')
  const res = await fetch(global.baseURL + 'api/scale/factor', {
    method: 'POST',
    headers: { 'Content-Type': 'application/json', Authorization: global.token },
    body: JSON.stringify({ scale_index: scale.value, weight: weight.value }),
    signal: AbortSignal.timeout(global.fetchTimout)
  })

  if (!res.ok) {
    logError('DeviceCalibrationView.sendFactor()', 'Failed with', res.ok)
    callback(false, {})
    return
  }

  const json = await res.json()
  callback(true, json)
}

async function getScaleStatus(callback) {
  logInfo('DeviceCalibrationView.getScaleStatus()', 'Fetching /api/scale')
  const res = await fetch(global.baseURL + 'api/scale', {
    method: 'GET',
    headers: { 'Content-Type': 'application/json', Authorization: global.token },
    signal: AbortSignal.timeout(global.fetchTimout)
  })

  if (!res.ok) {
    logError('DeviceCalibrationView.getScaleStatus()', 'Failed with', res.ok)
    callback(false, {})
    return
  }

  const json = await res.json()
  callback(true, json)
}
</script>
