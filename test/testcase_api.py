import unittest, requests, json, time

# python3 -m unittest -v testcase_api.py -v
# python3 -m unittest -v testcase_api.API.test_config_1
# python3 -m unittest -v testcase_api.API.test_status

host = "192.168.1.200"
id   = "7376ef"
ver  = "0.5.0"

scale1 = True       # Enable test for scale 1
scale2 = False      # Enable test for scale 2, my dev board only has one scale.

def call_api_post( path, json ):
    url = "http://" + host + path
    return requests.post( url, data=json )

def call_api_get( path ):
    url = "http://" + host + path
    return requests.get( url )

class API(unittest.TestCase):

    # Check that all parameters exist, dont validate the values.
    def test_status(self):
        r = call_api_get( "/api/status" )
        j = json.loads(r.text)
        self.assertEqual(r.status_code, 200)
        self.assertEqual(j["id"], id)
        self.assertEqual(j["wifi-ssid"], "@home")
        self.assertNotEqual(j["mdns"], "")
        self.assertEqual(j["app-ver"], ver)
        self.assertNotEqual(j["app-build"], "")
        self.assertNotEqual(j["weight-unit"], "") 
        self.assertNotEqual(j["volume-unit"], "") 
        self.assertNotEqual(j["temp-format"], "") 
        self.assertNotEqual(j["glass1"], -1) 
        self.assertNotEqual(j["keg-volume1"], -1) 
        self.assertNotEqual(j["temperature"], 0) 
        self.assertNotEqual(j["humidity"], 0) 

        if scale1:
            self.assertNotEqual(j["scale-factor1"], -1)
            self.assertNotEqual(j["scale-weight1"], -1)
            self.assertNotEqual(j["scale-raw1"], -1)
            self.assertNotEqual(j["scale-offset1"], -1) 
            self.assertNotEqual(j["beer-weight1"], -1) 
            self.assertNotEqual(j["beer-volume1"], -1) 
            self.assertNotEqual(j["scale-stable-weight1"], -1) 
            self.assertNotEqual(j["last-pour-weight1"], -1) 
            self.assertNotEqual(j["last-pour-volume1"], -1) 

        if scale2:
            self.assertNotEqual(j["scale-factor2"], -1)
            self.assertNotEqual(j["scale-weight2"], -1)
            self.assertNotEqual(j["scale-raw2"], -1)
            self.assertNotEqual(j["scale-offset2"], -1) 
            self.assertNotEqual(j["beer-weight2"], -1) 
            self.assertNotEqual(j["beer-volume2"], -1) 
            self.assertNotEqual(j["scale-stable-weight2"], -1) 
            self.assertNotEqual(j["last-pour-weight2"], -1) 
            self.assertNotEqual(j["last-pour-volume2"], -1) 


    # Check that all parameters exist
    def test_config_1(self):
        j = { "mdns": "kegscale", 
              "id": id, 
              "wifi-ssid": "", 
              "wifi-pass":" ",
              "wifi-ssid2": "", 
              "wifi-pass2": "",
              "temp-format": "C", 
              "wifi-portal-timeout": 120,
              "wifi-connect-timeout": 30,
              "ota-url": "",
              "http-post-target": "",
              "http-post-header1": "",
              "http-post-header2": "",
              "http-get-target": "",
              "http-get-header1": "",
              "http-get-header2": "",
              "influxdb2-target": "",
              "influxdb2-org": "",
              "influxdb2-bucket": "",
              "influxdb2-token": "",
              "mqtt-target": "",
              "mqtt-port": 1883,
              "mqtt-user": "",
              "mqtt-pass": "",
              "push-timeout": 10,
              "weight-unit": "kg",
              "volume-unit": "cl",
              "display-layout": 9,
              "brewfather-apikey": "",
              "brewfather-userkey": "", 
              "brewspy-token1": "",
              "brewspy-token2": "",
              "scale-temp-formula1": "",
              "scale-temp-formula2": "",
              "scale-factor1": 1,
              "scale-factor2": 1,
              "scale-offset1": 0, 
              "scale-offset2": 0, 
              "keg-weight1":0, 
              "keg-weight2": 0, 
              "keg-volume1":0, 
              "keg-volume2": 0, 
              "glass-volume1": 0.4,
              "glass-volume2": 0.4,
              "beer-name1": "", 
              "beer-name2": "", 
              "beer-abv1": 0, 
              "beer-abv2": 0, 
              "beer-ebc1": 0, 
              "beer-ebc2": 0, 
              "beer-fg1": 0, 
              "beer-fg2": 0, 
              "beer-ibu1": 0, 
              "beer-ibu2": 0 ,
              "scale-deviation-increase": 0.4,
              "scale-deviation-decrease": 0.1,
              "scale-deviation-kalman": 0.05,
              "scale-read-count": 3,
              "scale-read-count-calibration": 30,
              "scale-stable-count": 8 }

        r = call_api_post( "/api/config", j )
        self.assertEqual(r.status_code, 200)
        r = call_api_get( "/api/config" )
        j = json.loads(r.text)
        self.assertEqual(r.status_code, 200)
        self.assertEqual(j["mdns"], "kegscale")
        self.assertEqual(j["temp-format"], "C")
        self.assertEqual(j["wifi-portal-timeout"], 120)
        self.assertEqual(j["wifi-connect-timeout"], 30)
        self.assertEqual(j["ota-url"], "")
        self.assertEqual(j["http-post-target"], "")
        self.assertEqual(j["http-post-header1"], "")
        self.assertEqual(j["http-post-header2"], "")
        self.assertEqual(j["http-get-target"], "")
        self.assertEqual(j["http-get-header1"], "")
        self.assertEqual(j["http-get-header2"], "")
        self.assertEqual(j["influxdb2-target"], "")
        self.assertEqual(j["influxdb2-org"], "")
        self.assertEqual(j["influxdb2-bucket"], "")
        self.assertEqual(j["influxdb2-token"], "")
        self.assertEqual(j["mqtt-target"], "")
        self.assertEqual(j["mqtt-port"], 1883)
        self.assertEqual(j["mqtt-user"], "")
        self.assertEqual(j["mqtt-pass"], "")
        self.assertEqual(j["push-timeout"], 10)
        self.assertEqual(j["weight-unit"], "kg")
        self.assertEqual(j["volume-unit"], "cl")
        self.assertEqual(j["display-layout"], 9)
        self.assertEqual(j["brewfather-apikey"], "")
        self.assertEqual(j["brewfather-userkey"], "")
        self.assertEqual(j["brewspy-token1"], "")
        self.assertEqual(j["brewspy-token2"], "")
        self.assertEqual(j["scale-temp-formula1"], "")
        self.assertEqual(j["scale-temp-formula2"], "")
        self.assertEqual(j["scale-offset1"], 0)
        self.assertEqual(j["scale-factor1"], 1)
        self.assertEqual(j["scale-offset2"], 0)
        self.assertEqual(j["scale-factor2"], 1)
        self.assertEqual(j["keg-weight1"], 0)
        self.assertEqual(j["keg-weight2"], 0)
        self.assertEqual(j["keg-volume1"], 0)
        self.assertEqual(j["keg-volume2"], 0)
        self.assertEqual(j["glass-volume1"], 0.4)
        self.assertEqual(j["glass-volume2"], 0.4)
        self.assertEqual(j["beer-name1"], "")
        self.assertEqual(j["beer-name2"], "")
        self.assertEqual(j["beer-abv1"], 0)
        self.assertEqual(j["beer-abv2"], 0)
        self.assertEqual(j["beer-ebc1"], 0)
        self.assertEqual(j["beer-ebc2"], 0)
        self.assertEqual(j["beer-fg1"], 0)
        self.assertEqual(j["beer-fg2"], 0)
        self.assertEqual(j["beer-ibu1"], 0)
        self.assertEqual(j["beer-ibu2"], 0)
        self.assertEqual(j["scale-deviation-increase"], 0.4)
        self.assertEqual(j["scale-deviation-decrease"], 0.1)
        self.assertEqual(j["scale-deviation-kalman"], 0.05)
        self.assertEqual(j["scale-read-count"], 3)
        self.assertEqual(j["scale-read-count-calibration"], 30)
        self.assertEqual(j["scale-stable-count"], 8)

    def test_config_2(self):
        j = { "id": id, "temp-format": "C" }
        r = call_api_post( "/api/config", j )
        self.assertEqual(r.status_code, 200)
        r = call_api_get( "/api/config" )
        j = json.loads(r.text)
        self.assertEqual(r.status_code, 200)
        self.assertEqual(j["temp-format"], "C")

        j = { "id": id, "temp-format": "F" }
        r = call_api_post( "/api/config", j )
        self.assertEqual(r.status_code, 200)
        r = call_api_get( "/api/config" )
        j = json.loads(r.text)
        self.assertEqual(r.status_code, 200)
        self.assertEqual(j["temp-format"], "F")

        j = { "id": id, "temp-format": "G" }
        r = call_api_post( "/api/config", j )
        self.assertEqual(r.status_code, 200)
        r = call_api_get( "/api/config" )
        j = json.loads(r.text)
        self.assertEqual(r.status_code, 200)
        self.assertEqual(j["temp-format"], "F")

    def test_config_3(self):
        j = { "id": id, "weight-unit": "kg" }
        r = call_api_post( "/api/config", j )
        self.assertEqual(r.status_code, 200)
        r = call_api_get( "/api/config" )
        j = json.loads(r.text)
        self.assertEqual(r.status_code, 200)
        self.assertEqual(j["weight-unit"], "kg")

        j = { "id": id, "weight-unit": "lbs" }
        r = call_api_post( "/api/config", j )
        self.assertEqual(r.status_code, 200)
        r = call_api_get( "/api/config" )
        j = json.loads(r.text)
        self.assertEqual(r.status_code, 200)
        self.assertEqual(j["weight-unit"], "lbs")

        j = { "id": id, "weight-unit": "jeep" }
        r = call_api_post( "/api/config", j )
        self.assertEqual(r.status_code, 200)
        r = call_api_get( "/api/config" )
        j = json.loads(r.text)
        self.assertEqual(r.status_code, 200)
        self.assertEqual(j["weight-unit"], "lbs")

    def test_config_4(self):
        j = { "id": id, "volume-unit": "cl" }
        r = call_api_post( "/api/config", j )
        self.assertEqual(r.status_code, 200)
        r = call_api_get( "/api/config" )
        j = json.loads(r.text)
        self.assertEqual(r.status_code, 200)
        self.assertEqual(j["volume-unit"], "cl")

        j = { "id": id, "volume-unit": "uk-oz" }
        r = call_api_post( "/api/config", j )
        self.assertEqual(r.status_code, 200)
        r = call_api_get( "/api/config" )
        j = json.loads(r.text)
        self.assertEqual(r.status_code, 200)
        self.assertEqual(j["volume-unit"], "uk-oz")

        j = { "id": id, "volume-unit": "us-oz" }
        r = call_api_post( "/api/config", j )
        self.assertEqual(r.status_code, 200)
        r = call_api_get( "/api/config" )
        j = json.loads(r.text)
        self.assertEqual(r.status_code, 200)
        self.assertEqual(j["volume-unit"], "us-oz")

        j = { "id": id, "volume-unit": "jeep" }
        r = call_api_post( "/api/config", j )
        self.assertEqual(r.status_code, 200)
        r = call_api_get( "/api/config" )
        j = json.loads(r.text)
        self.assertEqual(r.status_code, 200)
        self.assertEqual(j["volume-unit"], "us-oz")


if __name__ == '__main__':
    unittest.main()

