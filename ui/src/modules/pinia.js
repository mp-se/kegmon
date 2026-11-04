import { ref } from 'vue'
import { createPinia } from 'pinia'
import { useGlobalStore } from '@/modules/globalStore'
import { useStatusStore } from '@/modules/statusStore'
import { useConfigStore } from '@/modules/configStore'
import { logInfo } from '@/modules/logger'

const piniaInstance = createPinia()

export default { piniaInstance }

const config = useConfigStore(piniaInstance)
const global = useGlobalStore(piniaInstance)
const status = useStatusStore(piniaInstance)

export { global, status, config }

const configCompare = ref(null)

const saveConfigState = () => {
  logInfo('pinia.saveConfigState()', 'Saving state')

  configCompare.value = {}
  for (var key in config) {
    if (typeof config[key] !== 'function' && key !== '$id') {
      configCompare.value[key] = config[key]
    }
  }

  logInfo('pinia.saveConfigState()', 'Saved state: ', configCompare.value)
  global.configChanged = false
}

const getConfigChanges = () => {
  var changes = {}

  if (configCompare.value === null) {
    logInfo('pinia.getConfigChanges()', 'configState not saved')
    return changes
  }

  for (var key in configCompare.value) {
    if (configCompare.value[key] != config[key]) {
      changes[key] = config[key]
    }
  }

  return changes
}

config.$subscribe(() => {
  if (!global.initialized) return

  var changes = getConfigChanges()
  logInfo('pinia.subscribe()', 'State change on configStore', changes)

  if (JSON.stringify(changes).length > 2) {
    global.configChanged = true
    logInfo('pinia.subscribe()', 'Changed properties:', changes)
  } else {
    global.configChanged = false
  }
})

export { saveConfigState, getConfigChanges }
