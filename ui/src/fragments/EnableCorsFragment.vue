<template>
  <h5>Developer settings</h5>
  <div class="row gy-4">
    <div class="col-md-3">
      <button
        @click="enableCors"
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
        &nbsp;Enable CORS</button
      >&nbsp;
    </div>
  </div>
</template>

<script setup>
import { global } from '@/modules/pinia'
import { logInfo, logError } from '@/modules/logger'

const enableCors = () => {
  global.disabled = true
  global.clearMessages()

  var data = {
    cors_allowed: true
  }

  fetch(global.baseURL + 'api/config', {
    method: 'POST',
    headers: {
      'Content-Type': 'application/json',
      Authorization: global.token
    },
    body: JSON.stringify(data),
    signal: AbortSignal.timeout(global.fetchTimout)
  })
    .then((res) => {
      global.disabled = false
      if (res.status != 200) {
        logError('EnableCorsFragment.enableCors()', 'Sending /api/config failed', res.status)
        global.messageError = 'Failed to enable CORS.'
      } else {
        logInfo('EnableCorsFragment.enableCors()', 'Sending /api/config completed')
        global.messageSuccess = 'CORS enabled in configuration, reboot to take effect.'
      }
      global.disabled = false
    })
    .catch((err) => {
      logError('EnableCorsFragment.enableCors()', err)
      global.disabled = false
    })
}
</script>
