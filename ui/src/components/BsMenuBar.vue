<template>
  <nav class="navbar navbar-expand-lg navbar-dark bg-primary">
    <div class="container-fluid align-center">
      <button
        class="navbar-toggler"
        type="button"
        data-bs-toggle="collapse"
        data-bs-target="#navbar"
        aria-controls="navbarNav"
        aria-expanded="false"
        aria-label="Toggle navigation"
      >
        <span class="navbar-toggler-icon"></span>
      </button>

      <div class="navbar-brand">
        {{ brand }}
      </div>

      <div class="vr d-none d-lg-flex h-200 mx-lg-2 text-white"></div>

      <div class="collapse navbar-collapse" id="navbar">
        <ul class="navbar-nav">
          <!-- 
                        The drop-downs here are in conflict with the Vue so that dropdowns are not always closed. 
                        This is a known issue with BS5 and Vue...
                    -->
          <template v-for="(item, index) in items" :key="index">
            <li v-if="!item.subs.length" class="nav-item">
              <router-link
                :class="[
                  'nav-link',
                  $router.currentRoute.value.path.split('/')[1] === item.path.split('/')[1]
                    ? ' active fw-bold'
                    : ''
                ]"
                :to="item.path"
                :disabled="disabled"
              >
                <component
                  v-if="item.icon !== undefined"
                  :is="item.icon"
                  width="1rem"
                  height="1rem"
                  style="color: white"
                ></component>

                {{ item.label }}
              </router-link>
            </li>
            <li v-else class="nav-item dropdown">
              <a
                @click="menuClicked"
                :class="[
                  'nav-link',
                  'dropdown-toggle',
                  $router.currentRoute.value.path.split('/')[1] === item.path.split('/')[1]
                    ? ' active fw-bold'
                    : ''
                ]"
                :id="'navbarDropdown' + item.label"
                role="button"
                data-bs-toggle="dropdown"
                aria-expanded="false"
                data-bs-auto-close="true"
                :disabled="disabled"
              >
                <component
                  v-if="item.icon !== undefined"
                  :is="item.icon"
                  width="1rem"
                  height="1rem"
                  style="color: white"
                ></component>

                {{ item.label }}
                <span
                  v-if="item.badge !== undefined && item.badge() > 0"
                  class="badge text-bg-danger rounded-circle"
                  >{{ item.badge() }}</span
                >
              </a>
              <ul class="dropdown-menu" :aria-labelledby="'navbarDropdown' + item.label">
                <li v-for="dn in item.subs" :key="dn.path">
                  <router-link
                    @click="subMenuClicked"
                    class="dropdown-item"
                    :to="dn.path"
                    :disabled="disabled"
                    >{{ dn.label }}
                    <span
                      v-if="dn.badge !== undefined && dn.badge() > 0"
                      class="badge text-bg-danger rounded-circle"
                      >{{ dn.badge() }}</span
                    ></router-link
                  >
                </li>
              </ul>
            </li>
          </template>
        </ul>
      </div>

      <div class="vr d-none d-lg-flex h-200 mx-lg-2 text-white"></div>

      <div class="text-white">
        {{ config.mdns }}
      </div>

      <div class="vr d-none d-lg-flex h-200 mx-lg-2 text-white"></div>

      <div v-if="global.configChanged">
        <span class="badge bg-danger fs-6">Save needed &nbsp;</span>
      </div>

      <div class="vr d-none d-lg-flex h-200 mx-lg-2 text-white" v-if="global.configChanged"></div>

      <div class="p-2">
        <div class="spinner-border gx-4" role="status" style="color: white" :hidden="!disabled">
          <span class="visually-hidden">Loading...</span>
        </div>
      </div>
      <div class="p-2">
        <div class="form-check form-switch">
          &nbsp;<input
            v-model="config.dark_mode"
            class="form-check-input"
            type="checkbox"
            role="switch"
            style="border-color: white"
          />
        </div>
      </div>
    </div>
  </nav>
</template>

<script setup>
/**
 * 2024-05-28 Bootstrap VueJS wrapper, Magnus Persson
 */

import { config, global } from '@/modules/pinia'
import { onMounted, watch } from 'vue'
import { storeToRefs } from 'pinia'
import { items } from '@/modules/router'
/**
 * Purpose: Provide an meny with dropdown options, dark mode toggle and spinner.
 *
 * Note! Item structure is stored in the router file.
 */

/**
 * Ref that steers if this component is enabled or not (required)
 */
const disabled = defineModel('disabled')

/**
 * Ref that contains the brand name
 */
const brand = defineModel('brand')

/**
 * Darkmode ref is imported from the config store (required).
 */
const { dark_mode } = storeToRefs(config)

onMounted(() => {
  setMode()
})

watch(dark_mode, () => {
  setMode()
})

function setMode() {
  if (config.dark_mode) {
    document.documentElement.setAttribute('data-bs-theme', 'dark')
  } else {
    document.documentElement.setAttribute('data-bs-theme', 'light')
  }
}

const subMenuClicked = () => {
  // Added to fix the issue with dropdown-menu shown after select due to conflict with VueJS and BootstrapJS
  const dnList = document.getElementsByClassName('dropdown-menu show')
  for (var i = 0; i < dnList.length; i++) {
    dnList[i].classList.remove('show')
  }
}
</script>
