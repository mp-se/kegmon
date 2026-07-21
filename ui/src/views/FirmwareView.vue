<template>
  <div class="container">
    <p></p>
    <p class="h3">Firmware Upload</p>
    <hr />

    <div class="row">
      <form @submit.prevent="upload">
        <div style="col-md-12">
          <p>
            Selet the firmware file that matches your device. Platform:
            <span class="badge bg-secondary">{{ status.platform }}</span>
            , Version: <span class="badge bg-secondary">{{ status.app_ver }}</span> ({{
              status.app_build
            }})
          </p>
        </div>

        <div class="col-md-12">
          <BsFileUpload
            name="upload"
            id="upload"
            label="Select firmware file"
            accept=".bin"
            help="Choose the firmware file that will be used to update the device"
            :disabled="global.disabled"
          >
          </BsFileUpload>
        </div>

        <div class="col-md-3">
          <p></p>
          <button
            type="submit"
            class="btn btn-primary"
            id="upload-btn"
            value="upload"
            data-bs-toggle="tooltip"
            title="Update the device with the selected firmware"
            :disabled="global.disabled"
          >
            <span
              class="spinner-border spinner-border-sm"
              role="status"
              aria-hidden="true"
              :hidden="!global.disabled"
            ></span>
            &nbsp;Flash firmware
          </button>
        </div>

        <div v-if="progress > 0" class="col-md-12">
          <p></p>
          <BsProgress :progress="progress"></BsProgress>
        </div>
      </form>
    </div>
  </div>
</template>

<script setup>
import { ref } from 'vue'
import { global, status } from '@/modules/pinia'
import { logDebug, logError } from '@/modules/logger'

const progress = ref(0)

function upload() {
  const fileElement = document.getElementById('upload')

  function errorAction(e) {
    logError('FirmwareView.upload()', e.type)
    global.messageFailed = 'File upload failed!'
    global.disabled = false
  }

  if (fileElement.files.length === 0) {
    global.messageFailed = 'You need to select one file with firmware to upload'
  } else {
    global.disabled = true
    logDebug('FirmwareView.upload()', 'Selected file: ' + fileElement.files[0].name)

    const xhr = new XMLHttpRequest()
    xhr.timeout = 1000 * 180 // 180 s
    progress.value = 0

    xhr.onabort = function (e) {
      errorAction(e)
    }
    xhr.onerror = function (e) {
      errorAction(e)
    }
    xhr.ontimeout = function (e) {
      errorAction(e)
    }

    xhr.onloadstart = function () {}

    xhr.onloadend = function () {
      progress.value = 100
      if (xhr.status == 200) {
        global.messageSuccess =
          'File upload completed, waiting for device to restart before doing refresh!'
        global.messageFailed = ''
      }
      setTimeout(() => {
        location.href = location.href.replace('/other/firmware', '')
      }, 10000)
    }

    // The update only seams to work when loaded from the device (i.e. when CORS is not used)
    xhr.upload.addEventListener(
      'progress',
      (e) => {
        progress.value = (e.loaded / e.total) * 100
      },
      false
    )

    const fileData = new FormData()
    fileData.onprogress = function (e) {
      logDebug('FirmwareView.upload()', 'progress2: ' + e.loaded + ',' + e.total + ',' + xhr.status)
    }

    fileData.append('file', fileElement.files[0])

    xhr.open('POST', global.baseURL + 'api/firmware')
    xhr.setRequestHeader('Authorization', global.token)
    xhr.send(fileData)
  }
}
</script>
