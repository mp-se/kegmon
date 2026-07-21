import { createApp } from 'vue'
import App from './App.vue'

const app = createApp(App)

import piniaInstance from './modules/pinia.js'
app.use(piniaInstance)

import router from './modules/router.js'
app.use(router)

import BsMessage from './components/BsMessage.vue'
import BsCard from './components/BsCard.vue'
import BsFileUpload from './components/BsFileUpload.vue'
import BsProgress from './components/BsProgress.vue'
import BsInputBase from './components/BsInputBase.vue'
import BsInputText from './components/BsInputText.vue'
import BsInputReadonly from './components/BsInputReadonly.vue'
import BsSelect from './components/BsSelect.vue'
import BsInputTextArea from './components/BsInputTextArea.vue'
import BsInputNumber from './components/BsInputNumber.vue'
import BsInputSwitch from './components/BsInputSwitch.vue'
import BsInputRadio from './components/BsInputRadio.vue'
import BsDropdown from './components/BsDropdown.vue'

import BsModal from './components/BsModal.vue'
import BsModalConfirm from './components/BsModalConfirm.vue'
import BsModalSelect from './components/BsModalSelect.vue'

app.component('BsMessage', BsMessage)
app.component('BsDropdown', BsDropdown)
app.component('BsCard', BsCard)
app.component('BsModal', BsModal)
app.component('BsModalConfirm', BsModalConfirm)
app.component('BsFileUpload', BsFileUpload)
app.component('BsProgress', BsProgress)
app.component('BsInputBase', BsInputBase)
app.component('BsInputText', BsInputText)
app.component('BsInputReadonly', BsInputReadonly)
app.component('BsSelect', BsSelect)
app.component('BsInputTextArea', BsInputTextArea)
app.component('BsInputNumber', BsInputNumber)
app.component('BsInputRadio', BsInputRadio)
app.component('BsInputSwitch', BsInputSwitch)
app.component('BsModalSelect', BsModalSelect)

import IconHome from './components/IconHome.vue'
import IconTools from './components/IconTools.vue'
import IconJustify from './components/IconJustify.vue'
import IconCloudUpArrow from './components/IconCloudUpArrow.vue'
import IconCpu from './components/IconCpu.vue'

app.component('IconHome', IconHome)
app.component('IconTools', IconTools)
app.component('IconJustify', IconJustify)
app.component('IconCloudUpArrow', IconCloudUpArrow)
app.component('IconCpu', IconCpu)

import 'bootstrap/dist/css/bootstrap.css'

app.mount('#app')

//import '@popperjs/core/dist/umd/popper.min.js'
import 'bootstrap/dist/js/bootstrap.bundle.js'
//import 'chartjs/chart.js'
