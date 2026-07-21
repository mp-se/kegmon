<template>
  <h5>Explore the file system</h5>
  <div class="row gy-4">
    <div class="col-md-3">
      <button
        @click="listFilesView"
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
        &nbsp;List files</button
      >&nbsp;
    </div>

    <div class="col-md-6">
      <div class="button-group">
        <template v-for="(f, index) in filesView" :key="index">
          <button
            type="button"
            @click.prevent="viewFile(f)"
            class="btn btn-outline-primary"
            href="#"
            :disabled="global.disabled"
          >
            {{ f }}</button
          >&nbsp;
        </template>
      </div>
    </div>
  </div>

  <div v-if="filesystemUsage > 0" class="col-md-12">
    <h6>File system usage</h6>
    <BsProgress :progress="filesystemUsage"></BsProgress>
    <p>{{ filesystemUsageText }}</p>
  </div>

  <div v-if="fileData !== null" class="col-md-12">
    <h6>File contents</h6>
    <pre class="border p-2">{{ fileData }}</pre>
  </div>
</template>

<script setup>
import { ref } from 'vue'
import { global, config } from '@/modules/pinia'
import { isValidJson, isValidFormData, isValidMqttData } from '@/modules/utils'

const filesystemUsage = ref(null)
const filesystemUsageText = ref(null)
const filesView = ref([])
const fileData = ref(null)

const viewFile = (f) => {
  global.disabled = true
  global.clearMessages()

  fileData.value = null

  var data = {
    command: 'get',
    file: f
  }

  config.sendFilesystemRequest(data, (success, text) => {
    if (success) {
      if (isValidJson(text)) fileData.value = JSON.stringify(JSON.parse(text), null, 2)
      else if (isValidFormData(text)) fileData.value = text.replaceAll('&', '&\n\r')
      else if (isValidMqttData(text)) fileData.value = text.replaceAll('|', '|\n\r')
      else fileData.value = text
    }

    global.disabled = false
  })
}

const listFilesView = () => {
  global.disabled = true
  global.clearMessages()

  filesView.value = []

  var data = {
    command: 'dir'
  }

  config.sendFilesystemRequest(data, (success, text) => {
    if (success) {
      var json = JSON.parse(text)
      filesystemUsage.value = (json.used / json.total) * 100
      filesystemUsageText.value =
        'Total space ' +
        json.total / 1024 +
        'kb, Free space ' +
        json.free / 1024 +
        'kb, Used space ' +
        json.used / 1024 +
        'kb'

      for (var f in json.files) {
        filesView.value.push(json.files[f].file)
      }
    }

    global.disabled = false
  })
}
</script>
