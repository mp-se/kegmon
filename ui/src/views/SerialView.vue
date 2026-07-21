<template>
  <div class="container">
    <p></p>
    <p class="h3">Serial console ({{ connected }})</p>
    <hr />

    <pre>{{ serial }}</pre>

    <div class="row gy-2">
      <div class="col-md-12">
        <hr />
      </div>
      <div class="col-md-12">
        <button @click="clear" type="button" class="btn btn-primary w-2" :disabled="!isConnected">
          Clear</button
        >&nbsp;

        <button
          @click="connect"
          type="button"
          class="btn btn-secondary w-2"
          :disabled="isConnected"
        >
          Connect
        </button>
      </div>
    </div>
  </div>
</template>

<script setup>
import { ref, onMounted, onUnmounted, computed } from 'vue'
import { global } from '@/modules/pinia'

const socket = ref(null)
const serial = ref('')
const maxLines = 50

function clear() {
  serial.value = ''
}

onUnmounted(() => {
  if (socket.value) socket.value.close()
  socket.value = null
})

const isConnected = computed(() => {
  return socket.value === null ? false : true
})

const connected = computed(() => {
  return socket.value === null ? 'Not connected' : 'Connected'
})

function connect() {
  serial.value = 'Attempting to connect to websocket\n'
  var host = global.baseURL.replaceAll('http://', 'ws://')
  socket.value = new WebSocket(host + 'serialws')

  socket.value.onopen = function () {
    serial.value += 'Websocket established\n'
  }

  socket.value.onmessage = function (event) {
    var list = serial.value.split('\n')

    while (list.length > maxLines) {
      list.shift()
    }

    serial.value = list.join('\n')
    serial.value += event.data
  }

  socket.value.onclose = function () {
    serial.value += 'Socket closed\n'
    socket.value = null
  }
}

onMounted(() => {
  connect()
})
</script>
