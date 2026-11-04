<template>
  <h5>Upload files to file system</h5>
  <div class="row gy-4">
    <form @submit.prevent="upload">
      <div class="col-md-12">
        <BsFileUpload
          name="upload"
          id="upload"
          label="Select firmware file"
          accept=""
          help="Choose a file to upload to the file system"
          :disabled="global.disabled"
        >
        </BsFileUpload>
      </div>
      <div class="col-md-3">
        <p></p>
        <button
          type="submit"
          class="btn btn-secondary"
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
          &nbsp;Upload file
        </button>
      </div>
      <div v-if="progress > 0" class="col-md-12">
        <p></p>
        <BsProgress :progress="progress"></BsProgress>
      </div>
    </form>
  </div>

  <div class="row gy-4">
    <p></p>
    <hr />
  </div>

  <h5>Delete files from file system</h5>
  <div class="row gy-4">
    <div class="col-md-3">
      <button
        @click="listFilesDelete"
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
        &nbsp;List files
      </button>
    </div>
    <div class="col-md-6">
      <div class="button-group">
        <template v-for="(f, index) in filesDelete" :key="index">
          <button
            type="button"
            @click.prevent="deleteFile(f)"
            class="btn btn-outline-primary"
            href="#"
            :disabled="global.disabled"
          >
            {{ f }}</button
          >&nbsp;
        </template>
      </div>
    </div>

    <BsModalConfirm
      :callback="confirmDeleteCallback"
      :message="confirmDeleteMessage"
      id="deleteFile"
      title="Delete file"
      :disabled="global.disabled"
    />
  </div>
</template>

<script setup>
import { ref } from 'vue'
import { global, config } from '@/modules/pinia'
import { logDebug, logError } from '@/modules/logger'

const fileData = ref(null)
const filesDelete = ref([])

const confirmDeleteMessage = ref(null)
const confirmDeleteFile = ref(null)

const confirmDeleteCallback = (result) => {
  logDebug('AdancedFilesFragment.confirmDeleteCallback()', result)

  if (result) {
    global.disabled = true
    global.clearMessages()

    fileData.value = null

    var data = {
      command: 'del',
      file: confirmDeleteFile.value
    }

    config.sendFilesystemRequest(data, (success, text) => {
      logDebug('AdancedFilesFragment.confirmDeleteCallback()', success), text
      filesDelete.value = []
      global.disabled = false
    })
  }
}

const deleteFile = (f) => {
  confirmDeleteMessage.value = 'Do you really want to delete file ' + f
  confirmDeleteFile.value = f
  document.getElementById('deleteFile').click()
}

const listFilesDelete = () => {
  global.disabled = true
  global.clearMessages()

  filesDelete.value = []

  var data = {
    command: 'dir'
  }

  config.sendFilesystemRequest(data, (success, text) => {
    if (success) {
      var json = JSON.parse(text)
      for (var f in json.files) {
        filesDelete.value.push(json.files[f].file)
      }
    }

    global.disabled = false
  })
}

const progress = ref(0)

function upload() {
  const fileElement = document.getElementById('upload')

  function errorAction(e) {
    logError('AdancedFilesFragment.upload()', e.type)
    global.messageFailed = 'File upload failed!'
    global.disabled = false
  }

  if (fileElement.files.length === 0) {
    global.messageFailed = 'You need to select one file with firmware to upload'
  } else {
    global.disabled = true
    logDebug('AdancedFilesFragment.upload()', 'Selected file: ' + fileElement.files[0].name)

    const xhr = new XMLHttpRequest()
    xhr.timeout = 40000 // 40 s
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
        global.messageSuccess = 'File upload completed!'
        global.messageFailed = ''
      }

      global.disabled = false
      filesDelete.value = []
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
      logDebug(
        'AdancedFilesFragment.upload()',
        'progress2: ' + e.loaded + ',' + e.total + ',' + xhr.status
      )
    }

    fileData.append('file', fileElement.files[0])

    xhr.open('POST', global.baseURL + 'api/filesystem/upload')
    xhr.setRequestHeader('Authorization', global.token)
    xhr.send(fileData)
  }
}
</script>
