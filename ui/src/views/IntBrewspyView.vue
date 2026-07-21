<template>
  <div class="container">
    <p></p>
    <p class="h3">Integration - Brewspy</p>
    <hr />

    <form @submit.prevent="save" class="needs-validation" novalidate>
      <div class="row">
        <div class="col-md-12">
          <BsInputText
            v-model="config.brewspy_token1"
            type="password"
            maxlength="120"
            label="Token tap 1"
            :disabled="global.disabled"
          />
        </div>
        <div class="col-md-12">
          <BsInputText
            v-model="config.brewspy_token2"
            type="password"
            maxlength="120"
            label="Token tap 2"
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
import { validateCurrentForm } from '@/modules/utils'
import { global, config } from '@/modules/pinia'

const save = () => {
  if (!validateCurrentForm()) return

  config.saveAll()
}
</script>
