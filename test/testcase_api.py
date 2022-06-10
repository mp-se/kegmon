import unittest, requests, json, time

# python3 -m unittest -v testcase_api.py -v
# python3 -m unittest -v testcase_api.API.test_config_1

host = "192.168.1.200"
id   = "7376ef"
ver  = "0.0.5"

def call_api_post( path, json ):
    url = "http://" + host + path
    return requests.post( url, data=json )

def call_api_get( path ):
    url = "http://" + host + path
    return requests.get( url )

class API(unittest.TestCase):

    # Check that all parameters exist, will require a full build with all sensors
    def test_status(self):
        r = call_api_get( "/api/status" )
        j = json.loads(r.text)
        self.assertEqual(r.status_code, 200)
        self.assertEqual(j["id"], id)
        self.assertEqual(j["wifi-ssid"], "@home")
        self.assertNotEqual(j["scale-factor1"], 0)
        self.assertNotEqual(j["scale-factor2"], 0)
        self.assertNotEqual(j["scale-weight1"], 2)
        self.assertNotEqual(j["scale-weight2"], 2)
        self.assertNotEqual(j["scale-raw1"], 0)
        self.assertNotEqual(j["scale-raw2"], 0)
        self.assertNotEqual(j["scale-offset1"], 0)
        self.assertNotEqual(j["scale-offset1"], 0)
        self.assertNotEqual(j["scale-raw1"], 0)
        self.assertNotEqual(j["scale-raw2"], 0)
        self.assertNotEqual(j["mdns"], "")
        self.assertNotEqual(j["app-ver"], "0.0.0")
        self.assertNotEqual(j["pints1"], -1)
        self.assertNotEqual(j["pints2"], -1)
        self.assertNotEqual(j["temperature"], 0)
        self.assertNotEqual(j["humidity"], 0)

    # Check that all parameters exist
    def test_config_1(self):
        j = { "mdns": "kegscale", "id": id, "wifi-ssid": "", "wifi-pass":" ","wifi-ssid2": "", "wifi-pass2": "",
              "temp-format": "C", "weight-precision":2, "brewfather-apikey": "","brewfather-userkey": "", "scale-factor1": 1,
              "scale-factor2": 1,"scale-offset1": 0, "scale-offset2": 0, "keg-weight1":0, "keg-weight2": 0, "pint-weight1": 0, 
              "pint-weight2": 0,"beer-name1": "", "beer-name2": "", "beer-abv1": 0, "beer-abv2": 0, "beer-ebc1": 0, "beer-ebc2": 0, 
              "beer-ibu1": 0, "beer-ibu2": 0 }
        r = call_api_post( "/api/config", j )
        self.assertEqual(r.status_code, 200)

        r = call_api_get( "/api/config" )
        j = json.loads(r.text)
        self.assertEqual(r.status_code, 200)
        self.assertEqual(j["mdns"], "kegscale")
        self.assertEqual(j["temp-format"], "C")
        self.assertEqual(j["brewfather-apikey"], "")
        self.assertEqual(j["brewfather-userkey"], "")
        self.assertEqual(j["weight-precision"], 2)

        self.assertNotEqual(j["scale-offset1"], 0) # Should not be able to set these
        self.assertNotEqual(j["scale-factor1"], 1)
        self.assertNotEqual(j["scale-offset2"], 0)
        self.assertNotEqual(j["scale-factor2"], 1)

        self.assertEqual(j["keg-weight1"], 0)
        self.assertEqual(j["pint-weight1"], 0)
        self.assertEqual(j["beer-name1"], "")
        self.assertEqual(j["beer-abv1"], 0)
        self.assertEqual(j["beer-ebc1"], 0)
        self.assertEqual(j["beer-ibu1"], 0)

        self.assertEqual(j["keg-weight2"], 0)
        self.assertEqual(j["pint-weight2"], 0)
        self.assertEqual(j["beer-name2"], "")
        self.assertEqual(j["beer-abv2"], 0)
        self.assertEqual(j["beer-ebc2"], 0)
        self.assertEqual(j["beer-ibu2"], 0)

if __name__ == '__main__':
    unittest.main()

