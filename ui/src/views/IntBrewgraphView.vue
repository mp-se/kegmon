<template>
  <div class="container">
    <p></p>
    <p class="h3">Integration - BrewGraph</p>
    <hr />

    <form @submit.prevent="save" class="needs-validation" novalidate>
      <div class="row gy-2">
        <div class="col-md-12">
          <BsInputText
            v-model="config.brewgraph_url"
            type="url"
            maxlength="120"
            label="BrewGraph URL"
            :disabled="global.disabled"
          />
        </div>
        <div class="col-md-12">
          <BsInputText
            v-model="config.brewgraph_token1"
            type="password"
            maxlength="120"
            label="Token keg 1"
            :disabled="global.disabled"
          />
        </div>
        <div class="col-md-12">
          <BsInputText
            v-model="config.brewgraph_token2"
            type="password"
            maxlength="120"
            label="Token keg 2"
            :disabled="global.disabled"
          />
        </div>
        <div class="col-md-12">
          <BsInputText
            v-model="config.brewgraph_token3"
            type="password"
            maxlength="120"
            label="Token keg 3"
            :disabled="global.disabled"
          />
        </div>
        <div class="col-md-12">
          <BsInputText
            v-model="config.brewgraph_token4"
            type="password"
            maxlength="120"
            label="Token keg 4"
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

  config.brewgraph_url = config.brewgraph_url.replace(/\/+$/, '')
  config.saveAll()
}
</script>
