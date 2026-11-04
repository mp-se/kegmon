<template>
  <div class="container">
    <p></p>
    <p class="h3">Taps - History</p>
    <hr />

    <!--
        <BsMessage v-if="!chart" dismissable="false" message="" alert="danger">
            Unable to load chart.js from https://cdn.jsdelivr.net, check your internet connection
        </BsMessage>
        -->
    <div class="row">
      <canvas id="historyChart"></canvas>
    </div>

    <div class="row">
      <hr />
    </div>

    <div class="row">
      <div class="col-md-5">
        <table class="table table-striped">
          <thead>
            <tr>
              <th scope="col" class="col-sm-1">Keg1, Date</th>
              <th scope="col" class="col-sm-1">Pour</th>
            </tr>
          </thead>

          <tbody v-for="pour in historyPour1" :key="pour.date">
            <tr>
              <td>{{ pour.date }}</td>
              <td>{{ pour.volume }}</td>
            </tr>
          </tbody>
        </table>
      </div>

      <div class="col-md-2"></div>

      <div class="col-md-5">
        <table class="table table-striped">
          <thead>
            <tr>
              <th scope="col" class="col-sm-1">Keg 2, Date</th>
              <th scope="col" class="col-sm-1">Pour</th>
            </tr>
          </thead>

          <tbody v-for="pour in historyPour2" :key="pour.date">
            <tr>
              <td>{{ pour.date }}</td>
              <td>{{ pour.volume }}</td>
            </tr>
          </tbody>
        </table>
      </div>
    </div>
  </div>
</template>

<script setup>
import { onMounted, ref } from 'vue'
import {
  Chart as ChartJS,
  Legend,
  LinearScale,
  TimeScale,
  LineController,
  PointElement,
  LineElement
} from 'chart.js'
import 'date-fns'
import 'chartjs-adapter-date-fns'
import { global, config } from '@/modules/pinia'
import { logDebug, logError, logInfo } from '@/modules/logger'
import { volumeCLtoUKOZ, volumeCLtoUSOZ } from '@/modules/utils'

const levels = ref([])

const historyKeg1 = ref([])
const historyKeg2 = ref([])

const historyPour1 = ref([])
const historyPour2 = ref([])

var chart = null

const chartData = ref({
  datasets: [
    {
      label: 'Keg 1',
      data: historyKeg1.value,
      borderColor: 'green',
      backgroundColor: 'green',
      yAxisID: 'y',
      pointRadius: 0
    },
    {
      label: 'Keg 2',
      data: historyKeg2.value,
      borderColor: 'blue',
      backgroundColor: 'blue',
      yAxisID: 'y',
      pointRadius: 0
    }
  ]
})

const scaleOptions = ref({
  x: {
    type: 'time',
    time: {
      unit: 'day'
    }
  }
})

const chartOptions = ref({
  type: 'line',
  data: chartData.value,
  options: {
    scales: scaleOptions.value,
    animation: false
  }
})

class Level {
  constructor(date, keg1, keg2, pour1, pour2) {
    var v = date.split(' ')
    var d = v[0].split('-')
    var t = v[1].split(':')

    this.date = new Date(d[0], d[1], d[2], t[0], t[1], t[2])
    this.keg1 = keg1
    this.keg2 = keg2
    this.pour1 = pour1
    this.pour2 = pour2

    // logDebug(this.date, this.keg1, this.keg2, this.pour1, this.pour2)
  }

  static fromString(l) {
    var d = l.split(';')
    return new Level(
      d[0],
      Number.parseFloat(d[1]),
      Number.parseFloat(d[2]),
      Number.parseFloat(d[3]),
      Number.parseFloat(d[4])
    )
  }

  get date() {
    return this._date
  }
  get keg1() {
    return this._keg1
  }
  get keg2() {
    return this._keg2
  }
  get pour1() {
    return this._pour1
  }
  get pour2() {
    return this._pour2
  }
  set date(date) {
    this._date = date
  }
  set keg1(keg1) {
    this._keg1 = keg1
  }
  set keg2(keg2) {
    this._keg2 = keg2
  }
  set pour1(pour1) {
    this._pour1 = pour1
  }
  set pour2(pour2) {
    this._pour2 = pour2
  }
}

onMounted(() => {
  logInfo('TapsHistoryView.onMounted()')

  try {
    ChartJS.register(Legend, LinearScale, TimeScale, PointElement, LineController, LineElement)
  } catch (err) {
    logError('TapsHistoryView.onMounted()', err)
  }

  fetch(global.baseURL + 'levels', {
    method: 'GET',
    signal: AbortSignal.timeout(global.fetchTimout)
  })
    .then((res) => res.text())
    .then((text) => {
      logDebug(text)
      var lines = text.split('\n')

      lines.forEach((l) => {
        if (l.length > 5) levels.value.push(Level.fromString(l))
      })

      levels.value.forEach((l) => {
        logDebug(l.keg1, l.keg2)

        var d = new Date(l.date).toISOString()
        d = d.substring(0, 10) + ' ' + d.substring(11, 19)

        if (!isNaN(l.keg1)) historyKeg1.value.push({ x: d, y: l.keg1 })

        if (!isNaN(l.keg2)) historyKeg2.value.push({ x: d, y: l.keg2 })

        // TODO: Convert to chosen unit

        if (!isNaN(l.pour1) && l.pour1 > 0) {
          //logDebug("check", config.volume_unit == 'cl' ? l.pour1/100 : (config.volume_unit == "us-oz" ? volumeCLtoUSOZ(l.pour1/100) : volumeCLtoUKOZ(l.pour1/100) ) )
          var vol =
            new Number(
              config.volume_unit == 'cl'
                ? l.pour1 * 100
                : config.volume_unit == 'us-oz'
                  ? volumeCLtoUSOZ(l.pour1 * 100)
                  : volumeCLtoUKOZ(l.pour1 * 100)
            ).toFixed(0) +
            ' ' +
            config.getVolumeUnit
          historyPour1.value.push({ date: d, volume: vol })
        }

        if (!isNaN(l.pour2) && l.pour2 > 0) {
          //logDebug("check", config.volume_unit == 'cl' ? l.pour2/100 : (config.volume_unit == "us-oz" ? volumeCLtoUSOZ(l.pour2/100) : volumeCLtoUKOZ(l.pour2/100) ) )
          vol =
            new Number(
              config.volume_unit == 'cl'
                ? l.pour2 * 100
                : config.volume_unit == 'us-oz'
                  ? volumeCLtoUSOZ(l.pour2 * 100)
                  : volumeCLtoUKOZ(l.pour2 * 100)
            ).toFixed(0) +
            ' ' +
            config.getVolumeUnit
          historyPour2.value.push({ date: d, volume: vol })
        }
      })

      // logDebug(historyKeg1.value)
      // logDebug(historyKeg2.value)
      // logDebug(historyPour1.value)
      // logDebug(historyPour2.value)

      try {
        logDebug('TapsHistoryView.onMounted()', chartOptions.value)

        if (document.getElementById('historyChart') == null) {
          logError('TapsHistoryView.onMounted()', 'Unable to find the chart canvas')
        } else {
          chart = new ChartJS(document.getElementById('historyChart'), chartOptions.value)
          // logDebug(chart)
        }
      } catch (err) {
        logDebug('TapsHistoryView.onMounted()', err)
      }
    })
    .catch((err) => {
      logError('TapsHistoryView.onMounted()', err)
    })
})
</script>
