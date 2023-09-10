import requests

githubUrl = "https://api.github.com/repos/"
debug = False

config = { 
   "repos": [
      "mp-se/ArduinoJson",
      "mp-se/SimpleKalmanFilter",
      "mp-se/Statistic",
      "mp-se/esp8266-oled-ssd1306",
      "mp-se/DHT-sensor-library",
      "mp-se/Adafruit_Sensor",
      "mp-se/incbin",
      "mp-se/Arduino-Log",
      "mp-se/ArduinoJson",
      "mp-se/arduino-mqtt",
      "mp-se/tinyexpr",
      "mp-se/NAU7802_Arduino_Library",
      "mp-se/LiquidCrystal_I2C",
      "mp-se/Adafruit_BME280_Library",
	    "mp-se/tinyexpr",
	    "mp-se/arduinoCurveFitting",
	    "mp-se/NimBLE-Arduino",
    ],
}

def get_latest_tag(repo):
    if repo == "":
        return ""

    if debug: print("Debug:", githubUrl + repo + "/tags")
    response = requests.get(githubUrl + repo + "/tags")

    if response.status_code == 403:
       print( "Error:", "Probably rate limiting has kicked in.....") 

    if response.status_code == 200: 
      json = response.json()
      
      if len(json) > 0:
        if debug: print("Debug:", repo, json[0]["name"])
        return json[0]["name"]
    else:
      if debug: print("Debug:", "Return code:", response.status_code, repo)

    # print("Error:", "No releases found for:", repo)
    return ""

def get_parent(repo):
    if repo == "":
        return ""

    if debug: print("Debug:", githubUrl + repo)
    response = requests.get(githubUrl + repo)

    if response.status_code == 200:
      json = response.json()
      if "parent" in json:
        if debug: print("Debug:", repo, json["parent"]["full_name"])
        return json["parent"]["full_name"]
    else:
      if debug: print("Debug:", "Return code:", response.status_code, repo)

    return ""
      

if __name__ == '__main__':

  for repo in config["repos"]:    
    tag = get_latest_tag(repo)
    parent = get_parent(repo)
    ptag = get_latest_tag(parent)

    status = ""
    if tag != ptag: status = "CHECK!"
      
    print("Dependency:", repo, tag, parent, ptag, status)
