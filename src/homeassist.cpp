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
    "homeassistant/sensor/${mdns}_volume${tap}/"
    "config:{\"device_class\":\"volume\",\"name\":\"${mdns}_volume${tap}\","
    "\"unit_of_measurement\":\"L\",\"state_topic\":\"homeassistant/sensor/"
    "${mdns}_volume${tap}/state\",\"json_attributes_topic\":\"homeassistant/"
    "sensor/${mdns}_volume${tap}/"
    "attr\",\"unique_id\":\"${mdns}_volume${tap}\"}|"
    "homeassistant/sensor/${mdns}_volume${tap}/state:${volume}|"
    "homeassistant/sensor/${mdns}_volume${tap}/"
    "attr:{\"glasses\":\"${glasses}\"}|";

const char *beerTemplate =
    "homeassistant/sensor/${mdns}_beer${tap}/config:"
    "{\"name\":\"${mdns}_beer${tap}\",\"state_topic\":\"homeassistant/sensor/"
    "${mdns}_beer${tap}/state\",\"json_attributes_topic\":\"homeassistant/"
    "sensor/${mdns}_beer${tap}/"
    "attr\",\"unique_id\":\"${mdns}_beer${beer-tap}\"}|"
    "homeassistant/sensor/${mdns}_beer${tap}/state:${beer-name}|"
    "homeassistant/sensor/${mdns}_beer${tap}/"
    "attr:{\"abv\":\"${beer-abv}\",\"ibu\":\"${beer-ibu}\",\"ebc\":\"${beer-"
    "ebc}\"}|";

const char *pourTemplate =
    "homeassistant/sensor/${mdns}_pour${tap}/config:"
    "{\"device_class\":\"volume\",\"name\":\"${mdns}_pour${tap}\",\"unit_of_"
    "measurement\":\"L\",\"state_topic\":\"homeassistant/sensor/"
    "${mdns}_pour${tap}/state\",\"unique_id\":\"${mdns}_pour${tap}\"}|"
    "homeassistant/sensor/${mdns}_pour${tap}/state:${pour}|";

void HomeAssist::sendTapInformation(UnitIndex idx, float stableVol,
                                    float glasses) {
  if (!myConfig.hasTargetMqtt()) return;

  TemplatingEngine tpl;

  tpl.setVal("${mdns}", myConfig.getMDNS());
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