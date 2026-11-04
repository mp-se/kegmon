<template>
  <div class="container">
    <p></p>
    <p class="h3">Taps - Settings</p>
    <hr />

    <form @submit.prevent="save" class="needs-validation" novalidate>
      <div class="row">
        <div class="col-md-6">
          <BsInputNumber
            v-model="config.keg_weight1"
            width="5"
            min="0.0"
            max="10.0"
            step="0.01"
            label="Tap 1 - Empty keg weight"
            :unit="config.weight_unit"
            :disabled="global.disabled"
          />
        </div>
        <div class="col-md-6">
          <BsInputNumber
            v-model="config.keg_weight2"
            width="5"
            min="0.0"
            max="10.0"
            step="0.01"
            label="Tap 2 - Empty keg weight"
            :unit="config.weight_unit"
            :disabled="global.disabled"
          />
        </div>

        <div class="col-md-6">
          <BsSelect
            v-model="config.glass_volume1"
            width="8"
            :options="glassOptions"
            label="Tap 1 - Glass size"
            :disabled="global.disabled"
          />
        </div>
        <div class="col-md-6">
          <BsSelect
            v-model="config.glass_volume2"
            width="8"
            :options="glassOptions"
            label="Tap 2 - Glass size"
            :disabled="global.disabled"
          />
        </div>

        <div class="col-md-6">
          <BsSelect
            v-model="config.keg_volume1"
            width="8"
            :options="kegOptions"
            label="Tap 1 - Keg volume"
            :disabled="global.disabled"
          />
        </div>
        <div class="col-md-6">
          <BsSelect
            v-model="config.keg_volume1"
            width="8"
            :options="kegOptions"
            label="Tap 2 - Keg volume"
            :disabled="global.disabled"
          />
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
            &nbsp;Save
          </button>
        </div>
      </div>
    </form>
  </div>
</template>

<script setup>
import { ref } from 'vue'
import { validateCurrentForm } from '@/modules/utils'
import { global, config } from '@/modules/pinia'

const glassOptions = ref([
  { label: '20 cl / 7.0 imperial ounces', value: 0.2 },
  { label: '25 cl / 8.8 imperial ounces', value: 0.25 },
  { label: '30 cl / 11 imperial ounces', value: 0.3 },
  { label: '33 cl / 12 imperial ounces', value: 0.33 },
  { label: '40 cl / 14 imperial ounces', value: 0.4 },
  { label: '50 cl / 18 imperial ounces', value: 0.5 },
  { label: 'Imperial pint, 568 ml / 20 imp fl oz', value: 0.568 },
  { label: 'Half Imperial pint, 284 ml / 10 imp fl oz', value: 0.284 },
  { label: 'Quater Imperial pint, 142 ml / 5 imp fl oz', value: 0.142 },
  { label: 'US pint, 468 ml / 16 US fl oz', value: 0.468 }
])

const kegOptions = ref([
  { label: 'Oxebar 4l', value: 4 },
  { label: 'Mini 5l / 1.32gal / 169oz', value: 5 },
  { label: 'Oxebar 8l', value: 8 },
  { label: 'Cornelious 9l', value: 9 },
  { label: 'Mini 10l', value: 10 },
  { label: 'Cornelius 18l', value: 18 },
  { label: 'Cornelius 19l / 5gal / 640oz', value: 19 },
  { label: 'Keykeg 20l', value: 20 },
  { label: 'Sixth Barrel 5.16gal / 640oz', value: 19.5 },
  { label: 'Quarter Barrel 7.75gal / 992oz', value: 29.3 },
  { label: 'Unittank 29l', value: 29 },
  { label: 'Half Barrel 15.5gal / 1984oz', value: 58.7 },
  { label: 'Kegmenter 58l', value: 58 }
])

const save = () => {
  if (!validateCurrentForm()) return

  config.saveAll()
}
</script>
