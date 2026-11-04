<template>
  <div class="container">
    <p></p>
    <p class="h3">Backup & Restore</p>
    <hr />

    <div class="row">
      <div class="col-md-12">
        <p>Create a backup of the device configuration and store this in a textfile</p>
      </div>

      <div class="col-md-12">
        <button
          @click="backup"
          type="button"
          class="btn btn-primary w-2"
          data-bs-toggle="tooltip"
          :disabled="global.disabled"
        >
          Create backup
        </button>
      </div>

      <div class="col-md-12">
        <hr />
      </div>

      <div class="col-md-12">
        <p>Restore a previous backup of the device configuration by uploading it.</p>
      </div>
    </div>

    <div class="row">
      <form @submit.prevent="restore">
        <div class="col-md-12">
          <BsFileUpload
            name="upload"
            id="upload"
            label="Select backup file"
            accept=".txt"
            :disabled="global.disabled"
          >
          </BsFileUpload>
        </div>

        <div class="col-md-3">
          <p></p>
          <button
            type="submit"
            class="btn btn-primary"
            value="upload"
            data-bs-toggle="tooltip"
            title="Upload the configuration to the device"
            :disabled="global.disabled"
          >
            <span
              class="spinner-border spinner-border-sm"
              role="status"
              aria-hidden="true"
              :hidden="!global.disabled"
            ></span>
            &nbsp;Restore
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
import { global, config, getConfigChanges } from '@/modules/pinia'
import { logDebug } from '@/modules/logger'

const progress = ref(0)

function backup() {
  var backup = {
    meta: { version: '1.0.0', software: 'KegMon', created: '' },
    config: JSON.parse(config.toJson())
  }

  backup.meta.created = new Date().toISOString().slice(0, 10)

  logDebug('BackupView.backup()', backup)

  var s = JSON.stringify(backup, null, 2)
  var name = config.mdns + '.txt'
  download(s, 'text/plain', name)
  global.messageSuccess = 'Backup file created and downloaded as: ' + name
}

function restore() {
  const fileElement = document.getElementById('upload')
  logDebug('BackupView.restore()')

  if (fileElement.files.length === 0) {
    global.messageFailed = 'You need to select one file to restore configuration from'
  } else {
    global.disabled = true
    logDebug('BackupView.restore()', 'Selected file: ' + fileElement.files[0].name)
    const reader = new FileReader()
    reader.addEventListener('load', function (e) {
      let text = e.target.result
      try {
        const data = JSON.parse(text)
        if (data.meta.software === 'KegMon' && data.meta.version === '1.0.0') {
          doRestore1(data.config)
        } else if (data.meta.software === 'KegMon') {
          doRestore(data)
        } else {
          global.messageFailed = 'Unknown format, unable to process'
        }
      } catch (error) {
        console.error(error)
        global.messageFailed = 'Unable to parse configuration file for KegMon.'
      }
    })
    reader.readAsText(fileElement.files[0])
  }
}

function download(content, mimeType, filename) {
  const a = document.createElement('a')
  const blob = new Blob([content], { type: mimeType })
  const url = URL.createObjectURL(blob)
  a.setAttribute('href', url)
  a.setAttribute('download', filename)
  a.click()
}

function doRestore1(json) {
  logDebug('BackupView.doRestore1()')

  for (var k in json) {
    config[k] = json[k]
  }

  getConfigChanges()
  config.saveAll()
}

function doRestore(json) {
  logDebug('BackupView.doRestore()')

  for (var k in json) {
    var k2 = k.replaceAll('-', '_')
    config[k2] = json[k]
  }

  getConfigChanges()
  config.saveAll()
}
</script>
