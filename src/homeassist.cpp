/*
MIT License

Copyright (c) 2021-22 Magnus

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
 */
#include <homeassist.hpp>
#include <kegconfig.hpp>
#include <log.hpp>
#include <scale.hpp>
#include <templating.hpp>
#include <utils.hpp>

const char *volumeTemplate =
    "kegmon/${mdns}_volume${tap}/state:${volume}|"
    "kegmon/${mdns}_volume${tap}/"
    "attr:{\"glasses\":${glasses}}|"
    "homeassistant/sensor/${mdns}_volume${tap}/"
    "config:{\"device_class\":\"volume\",\"name\":\"${mdns}_volume${tap}\","
    "\"unit_of_measurement\":\"L\",\"state_topic\":\"kegmon/"
    "${mdns}_volume${tap}/state\",\"json_attributes_topic\":\"kegmon/"
    "${mdns}_volume${tap}/"
    "attr\",\"unique_id\":\"${mdns}_volume${tap}\","
    "\"device\": { \"identifiers\": \"${mdns}_${id}\", \"name\": \"${mdns}\", "
    "\"model\": \"kegmon\", \"manufacturer\": \"mp-se\", \"sw_version\": "
    "\"${sw-ver}\" } }|";

const char *beerTemplate =
    "kegmon/${mdns}_beer${tap}/state:${beer-name}|"
    "kegmon/${mdns}_beer${tap}/"
    "attr:{\"abv\":${beer-abv},\"abv\":${beer-abv},\"ibu\":${beer-ibu},\"ebc\":"
    "${beer-"
    "ebc}}|"
    "homeassistant/sensor/${mdns}_beer${tap}/config:"
    "{\"name\":\"${mdns}_beer${tap}\",\"state_topic\":\"kegmon/"
    "${mdns}_beer${tap}/state\",\"json_attributes_topic\":\"kegmon/"
    "${mdns}_beer${tap}/"
    "attr\",\"unique_id\":\"${mdns}_beer${tap}\","
    "\"device\": { \"identifiers\": \"${mdns}_${id}\", \"name\": \"${mdns}\", "
    "\"model\": \"kegmon\", \"manufacturer\": \"mp-se\", \"sw_version\": "
    "\"${sw-ver}\" } }|";

const char *pourTemplate =
    "kegmon/${mdns}_pour${tap}/state:${pour}|"
    "homeassistant/sensor/${mdns}_pour${tap}/config:"
    "{\"device_class\":\"volume\",\"name\":\"${mdns}_pour${tap}\",\"unit_of_"
    "measurement\":\"L\",\"state_topic\":\"kegmon/"
    "${mdns}_pour${tap}/state\",\"unique_id\":\"${mdns}_pour${tap}\", "
    "\"device\": { \"identifiers\": \"${mdns}_${id}\", \"name\": \"${mdns}\", "
    "\"model\": \"kegmon\", \"manufacturer\": \"mp-se\", \"sw_version\": "
    "\"${sw-ver}\" } }|";

const char *tempTemplate =
    "kegmon/${mdns}_temp/state:${temp}|"
    "homeassistant/sensor/${mdns}_temp/config:"
    "{\"device_class\":\"temperature\",\"name\":\"${mdns}_temp\",\"unit_of_"
    "measurement\":\"${temp-format}\",\"state_topic\":\"kegmon/"
    "${mdns}_temp/state\",\"unique_id\":\"${mdns}_temp\", "
    "\"device\": { \"identifiers\": \"${mdns}_${id}\", \"name\": \"${mdns}\", "
    "\"model\": \"kegmon\", \"manufacturer\": \"mp-se\", \"sw_version\": "
    "\"${sw-ver}\" } }|";

void HomeAssist::sendTempInformation(float tempC) {
  if (!myConfig.hasTargetMqtt()) return;
  if (isnan(tempC)) return;

  TemplatingEngine tpl;

  tpl.setVal("${mdns}", myConfig.getMDNS());
  tpl.setVal("${sw-ver}", CFG_APPVER);
  tpl.setVal("${id}", myConfig.getID());

  // if (myConfig.isTempFormatC()) {
  tpl.setVal("${temp}", tempC);
  tpl.setVal("${temp-format}", "°C");
  // } else {
  //  tpl.setVal("${temp}", convertCtoF(tempC));
  //  tpl.setVal("${temp-format}", "°F");
  // }

  const char *out = tpl.create(tempTemplate);
  EspSerial.print(out);
  EspSerial.print(CR);
  String outStr(out);
  _push->sendMqtt(outStr);

  Log.notice(F("HA  : Sending temp information to HA, last %FC" CR), tempC);

  tpl.freeMemory();
}

void HomeAssist::sendTapInformation(UnitIndex idx, float stableVol,
                                    float glasses) {
  if (!myConfig.hasTargetMqtt()) return;

  TemplatingEngine tpl;

  tpl.setVal("${mdns}", myConfig.getMDNS());
  tpl.setVal("${sw-ver}", CFG_APPVER);
  tpl.setVal("${id}", myConfig.getID());
  tpl.setVal("${volume}", stableVol, 3);
  tpl.setVal("${glasses}", glasses, 1);
  tpl.setVal("${tap}", static_cast<int>(idx) + 1);
  tpl.setVal("${beer-name}", myConfig.getBeerName(idx));
  tpl.setVal("${beer-abv}", myConfig.getBeerABV(idx));
  tpl.setVal("${beer-ibu}", myConfig.getBeerIBU(idx));
  tpl.setVal("${beer-ebc}", myConfig.getBeerEBC(idx));

  const char *out = tpl.create(volumeTemplate);
  EspSerial.print(out);
  EspSerial.print(CR);
  String outStr(out);
  _push->sendMqtt(outStr);

  Log.notice(F("HA  : Sending TAP information to HA, last %Fl [%d]" CR),
             stableVol);

  out = tpl.create(beerTemplate);
  EspSerial.print(out);
  EspSerial.print(CR);
  outStr = out;
  _push->sendMqtt(outStr);

  tpl.freeMemory();
}

void HomeAssist::sendPourInformation(UnitIndex idx, float pourVol) {
  if (!myConfig.hasTargetMqtt()) return;

  TemplatingEngine tpl;

  tpl.setVal("${mdns}", myConfig.getMDNS());
  tpl.setVal("${sw-ver}", CFG_APPVER);
  tpl.setVal("${id}", myConfig.getID());
  tpl.setVal("${pour}", pourVol, 3);
  tpl.setVal("${tap}", static_cast<int>(idx) + 1);

  Log.notice(F("HA  : Sending POUR information to HA, pour %Fl [%d]." CR),
             pourVol, idx);

  const char *out = tpl.create(pourTemplate);
  EspSerial.print(out);
  EspSerial.print(CR);
  String outStr(out);
  _push->sendMqtt(outStr);
  tpl.freeMemory();
}

// EOF
