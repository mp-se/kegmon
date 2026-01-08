/*
MIT License

Copyright (c) 2021-2026 Magnus

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
#include <kegpush.hpp>
#include <log.hpp>
#include <scale.hpp>
#include <utils.hpp>

void KegPushHandler::pushTempInformation(float tempC, bool isLoop) {
  if (isnan(tempC)) return;

  _ha->sendTempInformation(tempC);
}

void KegPushHandler::pushPourInformation(UnitIndex idx, float stableVol,
                                         float pourVol, bool isLoop) {
  if (!isLoop)  // Limit calls to brewspy
    _brewspy->sendPourInformation(idx, pourVol);

  _ha->sendPourInformation(idx, pourVol);
  _brewLogger->sendPourInformation(idx, pourVol, stableVol);
}

void KegPushHandler::pushKegInformation(UnitIndex idx, float stableVol,
                                        float pourVol, float glasses,
                                        bool isLoop) {
  if (!isLoop)  // Limit calls to brewspy
    _brewspy->sendTapInformation(idx, stableVol, pourVol);

  _ha->sendTapInformation(idx, stableVol, glasses);
  _barhelper->sendKegInformation(idx, stableVol);
  _brewLogger->sendKegInformation(idx, stableVol);
}

// EOF
