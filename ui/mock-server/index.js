/*
 * Project specific mock server
 *
 * (c) 2023-2024 Magnus Persson
 */
import { createRequire } from "module";
import { registerEspFwk } from './espfwk.js'
// import { configData, statusData } from './data.js'

const require = createRequire(import.meta.url);
const express = require('express')
var cors = require('cors')

const app = express()
const port = 3000

app.use(cors())
app.use(express.json())

registerEspFwk(app)

var testRunning = false

app.post('/api/push', (req, res) => {
  console.log('GET: /api/push')
  /* 
   * Description:    Initiate the push test for a defined target
   * Authentication: Required
   * Limitation:     - 
   * Note:           Use /api/test/push/status to check for completion
   * Return:         200 OK, 401 Access Denied, 422 Content Invalid
   * Request body:
     {
       push_format: "http_format|http_format2|http_format3|influxdb2_format|mqtt_format"
     }
   */
  if(!req.body.hasOwnProperty("push_format")) {
    res.sendStatus(422)
    return
  }  
  testRunning = true
  setTimeout(() => { testRunning = false }, 5000)
  var data = {
    success: true,
    message: "Test scheduled."
  }
  res.type('application/json')
  res.send(data)
})

app.get('/api/push/status', (req, res) => {
  console.log('GET: /api/push/status')
  /* 
   * Description:    Return status of the current gyro calibration process. 
   * Authentication: Required
   * Limitation:     - 
   * Note:           -
   * Return:         200 OK, 401 Access Denied
   */
  var data = {}
  if (testRunning) {
    data = {
      status: testRunning,
      success: false,
      message: "Push test running..."
    }
  } else {
    data = {
      status: false,
      success: true,
      message: "Push test completed...",
      push_return_code: 200,
      push_enabled: true
    }
  }
  res.type('application/json')
  res.send(data)
})


app.get('/levels', (req, res) => {
  console.log('GET: /levels')

  var levels = "2024-07-11 13:12:07;nan;16.587868;nan;nan\n\
2024-07-11 13:30:14;nan;16.488798;nan;0.099069 \n\
2024-07-11 13:30:15;nan;16.488798;nan;nan\n\
2024-07-11 14:01:24;nan;16.291916;nan;0.196884\n\
2024-07-11 14:01:24;nan;16.291916;nan;nan\n\
2024-07-11 17:22:55;nan;16.192898;nan;0.099018\n\
2024-07-11 17:22:55;nan;16.192898;nan;nan\n\
2024-07-11 19:34:53;nan;15.863690;nan;0.329206\n\
2024-07-11 19:34:53;nan;15.863690;nan;nan\n\
2024-07-11 22:14:11;nan;15.768347;nan;0.190148\n\
2024-07-11 22:14:11;nan;15.768347;nan;nan\n\
2024-07-11 22:17:53;nan;15.668524;nan;0.099822\n\
2024-07-11 22:17:53;nan;15.668524;nan;nan\n\
2024-07-11 22:19:23;nan;15.539115;nan;0.129409\n\
2024-07-11 22:19:24;nan;15.539115;nan;nan\n\
2024-07-12 00:03:40;8.623783;nan;nan;nan\n\
2024-07-12 00:03:40;nan;15.826628;nan;nan\n\
2024-07-12 01:48:25;nan;15.726063;nan;0.100565\n\
2024-07-12 01:48:26;nan;15.726063;nan;nan\n\
2024-07-12 01:50:02;nan;15.582197;nan;0.143866\n\
2024-07-12 01:50:02;nan;15.582197;nan;nan\n\
2024-07-12 05:12:30;nan;15.482185;nan;0.100012\n\
2024-07-12 05:12:30;nan;15.482185;nan;nan\n\
2024-07-12 23:25:21;8.524719;nan;0.099064;nan\n\
2024-07-12 23:25:21;8.524719;nan;nan;nan\n\
2024-07-14 04:58:30;8.670466;nan;nan;nan\n\
2024-07-14 04:58:30;nan;16.128782;nan;nan\n\
2024-07-14 05:07:22;nan;15.972906;nan;0.155876\n\
2024-07-14 05:07:22;nan;15.972906;nan;nan\n\
2024-07-14 05:11:04;nan;15.788408;nan;0.184498\n\
2024-07-14 05:11:05;nan;15.788408;nan;nan\n\
2024-07-14 05:17:51;nan;15.688248;nan;0.100161\n\
2024-07-14 05:17:51;nan;15.688248;nan;nan\n\
2024-07-14 19:57:58;8.662101;nan;nan;nan\n\
2024-07-14 19:57:59;nan;16.060747;nan;nan\n\
2024-07-14 20:34:13;nan;15.960558;nan;0.100189\n\
2024-07-14 20:34:13;nan;15.960558;nan;nan\n\
2024-07-14 20:36:31;nan;15.815523;nan;0.145035\n\
2024-07-14 20:36:31;nan;15.815523;nan;nan\n\
2024-07-14 20:40:11;nan;15.715315;nan;0.100208\n\
2024-07-14 20:40:11;nan;15.715315;nan;nan\n\
2024-07-15 09:53:54;nan;15.615223;nan;0.100092\n\
2024-07-15 09:53:54;nan;15.615223;nan;nan\n\
2024-07-16 14:37:44;nan;15.156647;nan;0.458576\n\
2024-07-16 14:37:45;nan;15.156647;nan;nan"
  res.send(levels)
})

app.get('/api/stability', (req, res) => {
  console.log('GET: /api/stability')
  /*
   * Description:    Return stability data as json document.
   * Authentication: Required
   * Note:           -
   * Return:         200 OK, 401 Access Denied
   */
  const statsData = {
    stability_sum1: 0,
    stability_min1: 0,
    stability_max1: 0,
    stability_ave1: 0,
    stability_var1: 0,
    stability_popdev1: 0,
    stability_ubiasdev1: 0,
    stability_count1: 0,    
    stability_sum2: 0,
    stability_min2: 0,
    stability_max2: 0,
    stability_ave2: 0,
    stability_var2: 0,
    stability_popdev2: 0,
    stability_ubiasdev2: 0,
    stability_count2: 0,    
  }

  res.type('application/json')
  res.send(statsData)
})

app.listen(port, () => {
  console.log(`Kegmon API simulator port http://localhost:${port}/`)
})