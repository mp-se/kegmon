import requests
import configparser
import json

debug = False
useFiles = False

githubUrl = "https://api.github.com/repos/"

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

def get_github_latest_tag(repo):
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

def get_github_parent(repo):
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
      
def get_github_repo_version():
  deps = []

  for repo in config["repos"]:    
    tag = get_github_latest_tag(repo)
    parent = get_github_parent(repo)
    ptag = get_github_latest_tag(parent)
    dep = { "repo": repo, "tag": tag, "parent": parent, "parentTag": ptag }
    deps.append(dep)

  return deps

def resolve_platformio_key(config, key):
  key = key.removeprefix("${")
  key = key.removesuffix("}")
  libs = config.get(key.split(".")[0], key.split(".")[1])
  return libs.split()

def find_platformio_libs():
  config = configparser.ConfigParser()
  config.read('platformio.ini')

  depList = []

  for section in config.sections():
    if section.startswith("env:"):
      libs = config.get(section, "lib_deps" )
      for lib in libs.split():
        if lib.startswith("${") and lib.endswith("}"):
          depList = depList + resolve_platformio_key(config, lib)
        else:
          depList.append(lib)

  # Remove duplicated entries
  depList = list(set(depList))

  dependecies = []

  for dep in depList:
    s = dep.removeprefix("https://github.com/").split("#")
    if len(s) == 2:
      e = { "repo": s[0], "tag": s[1] }
      dependecies.append(e)
    else:
      e = { "repo": s[0], "tag": "" }
      dependecies.append(e)

  # Entires have format; repository # tag 
  return dependecies

def write_json(file, data):
  f = open(file, "w")
  f.write( json.dumps(data, indent=2) )
  f.close()

def read_json(file):
  f = open(file, "r")
  data = json.loads( "".join(f.readlines()) )
  f.close()
  return data


if __name__ == '__main__':
  print("Dependency checker")

  print("Reading platformio configuration")
  if useFiles: 
    depLibs = read_json("project_deps.json")
  else:
    depLibs = find_platformio_libs()
    write_json("project_deps.json", depLibs)

  print("Fetching data from github on latest tags")
  if useFiles: 
    repos = read_json("repos.json")
  else:
    repos = get_github_repo_version()
    write_json("repos.json", repos)

  for i in depLibs:
    for j in repos:
      if i["repo"] == j["repo"]:
        if i["tag"] != j["tag"]:
          print("Check dependency for: ", i["repo"], i["tag"], " => ", j["tag"] )

  print("Done...")
