/*
MIT License

Copyright (c) 2026 Magnus

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
#include <brewgraph.hpp>

#include <kegconfig.hpp>
#include <log.hpp>

constexpr auto BREWGRAPH_API = "/ingest/kegmon";
constexpr auto BREWGRAPH_BEER_API = "/ingest/kegmon/beer";

String BrewGraph::getApiUrl(const char* path) const {
  String url = myConfig.getBrewGraphUrl();
  while (url.endsWith("/")) url.remove(url.length() - 1);
  return url + path;
}

void BrewGraph::sendPourInformation(UnitIndex idx, float pourVol,
                                    float kegVol, uint32_t eventTimeMs) {
  const uint8_t kegIdx = static_cast<uint8_t>(idx);
  if (strlen(myConfig.getBrewGraphUrl()) == 0 ||
      strlen(myConfig.getBrewGraphToken(kegIdx)) == 0)
    return;

  JsonDocument doc;
  doc["token"] = myConfig.getBrewGraphToken(kegIdx);
  doc["pour"] = pourVol;
  doc["volume"] = kegVol;
  doc["maxVolume"] = myConfig.getKegVolume(idx);
  char eventId[32];
  snprintf(eventId, sizeof(eventId), "keg-%u-%lu", kegIdx + 1,
           static_cast<unsigned long>(eventTimeMs));
  doc["eventId"] = eventId;

  String out;
  out.reserve(120);
  serializeJson(doc, out);

  Log.notice(F("BGRF: Sending pour information for keg %d." CR), kegIdx + 1);
  const String response = _push->sendHttpPost(
      out, getApiUrl(BREWGRAPH_API).c_str(), "Content-Type: application/json",
      "");
  updateStatus(response);
  Log.info(F("BGRF: Response code %d." CR), _lastHttpError);
}

void BrewGraph::sendKegInformation(UnitIndex idx, float kegVol) {
  const uint8_t kegIdx = static_cast<uint8_t>(idx);
  if (strlen(myConfig.getBrewGraphUrl()) == 0 ||
      strlen(myConfig.getBrewGraphToken(kegIdx)) == 0)
    return;

  JsonDocument doc;
  doc["token"] = myConfig.getBrewGraphToken(kegIdx);
  doc["volume"] = kegVol;
  doc["maxVolume"] = myConfig.getKegVolume(idx);

  String out;
  out.reserve(100);
  serializeJson(doc, out);

  Log.notice(F("BGRF: Sending level information for keg %d." CR), kegIdx + 1);
  const String response = _push->sendHttpPost(
      out, getApiUrl(BREWGRAPH_API).c_str(), "Content-Type: application/json",
      "");
  updateStatus(response);
  Log.info(F("BGRF: Response code %d." CR), _lastHttpError);
}

bool BrewGraph::getBeerInformation(JsonObject& obj, UnitIndex idx) {
  const uint8_t kegIdx = static_cast<uint8_t>(idx);
  if (strlen(myConfig.getBrewGraphUrl()) == 0 ||
      strlen(myConfig.getBrewGraphToken(kegIdx)) == 0)
    return false;

  JsonDocument request;
  request["token"] = myConfig.getBrewGraphToken(kegIdx);

  String payload;
  payload.reserve(80);
  serializeJson(request, payload);

  Log.notice(F("BGRF: Requesting beer information for keg %d." CR),
             kegIdx + 1);
  const String response = _push->sendHttpPost(
      payload, getApiUrl(BREWGRAPH_BEER_API).c_str(),
      "Content-Type: application/json", "");
  if (!_push->wasLastSuccessful()) return false;

  JsonDocument beer;
  if (deserializeJson(beer, response) || beer["name"].isNull()) return false;

  obj["name"] = beer["name"];
  obj["abv"] = beer["abv"];
  obj["ibu"] = beer["ibu"];
  obj["ebc"] = beer["ebc"];
  return true;
}

void BrewGraph::updateStatus(const String& response) {
  _lastTimestamp = millis();
  _lastStatus = _push->wasLastSuccessful();
  _lastHttpError = _push->getLastResponseCode();
  _lastResponse = response.substring(0, 160);
  _hasRun = true;
}

// EOF
