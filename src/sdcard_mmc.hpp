
/*
MIT License

Copyright (c) 2025-2026 Magnus

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
#ifndef SRC_SDCARD_MMC_HPP_
#define SRC_SDCARD_MMC_HPP_

#if defined(ENABLE_MMC)

#include <SD_MMC.h>

#include <log.hpp>
#include <sdcard.hpp>

class SdCardMMC : public SdCard {
 public:
  uint64_t _cardSize = 0;
  bool _hasCard = false;

 public:
  SdCardMMC() {}
  ~SdCardMMC() { end(); }

  bool hasCard() const { return _hasCard; }

  bool begin(uint8_t clk, uint8_t cmd, uint8_t d0) {
    SD_MMC.setPins(clk, cmd, d0);
    if (SD_MMC.begin("/sdcard", true, false, 40000, 5)) {
      _hasCard = true;
      _cardSize = SD_MMC.cardSize();

      const char *type = "Unknown";
      switch (SD_MMC.cardType()) {
        case CARD_NONE:
          type = "No Card";
          break;
        case CARD_MMC:
          type = "MMC";
          break;
        case CARD_SD:
          type = "SDSC";
          break;
        case CARD_SDHC:
          type = "SDHC/SDXC";
          break;
      }
      Log.notice(F("SD  : Card initialized. Size: %d Mb, Type: %s." CR),
                 _cardSize / 1024 / 1024, type);
    } else {
      Log.error(F("SD  :Failed to initialize SD card." CR));
      _hasCard = false;
    }

    listFiles();

    return _hasCard;
  }

  void end() { SD_MMC.end(); }

  File open(const String &path, const char *mode = FILE_READ,
            bool create = false) {
    if (!_hasCard) {
      Log.error(F("SD  : Card not initialized." CR));
      return File();
    }

    if (create && !SD_MMC.exists(path)) {  // Create file if it does not exist
      File file = SD_MMC.open(path, FILE_WRITE);
      if (!file) {
        Log.error(F("SD  : Failed to create file." CR));
        return File();
      }
    }

    return SD_MMC.open(path, mode);
  }

  bool exists(const String &path) const {
    Serial.println("3");

    if (!_hasCard) {
      Log.error(F("SD  : Card not initialized." CR));
      return false;
    }
    return SD_MMC.exists(path);
  }

  bool remove(const String &path) {
    if (!_hasCard) {
      Log.error(F("SD  : Card not initialized." CR));
      return false;
    }
    return SD_MMC.remove(path);
  }

  bool rename(const String &from, const String &to) {
    if (!_hasCard) {
      Log.error(F("SD  : Card not initialized." CR));
      return false;
    }
    return SD_MMC.rename(from, to);
  }

  uint64_t totalBytes() const {
    if (!_hasCard) {
      Log.error(F("SD  : Card not initialized." CR));
      return 0;
    }
    return SD_MMC.totalBytes();
  }

  uint64_t usedBytes() const {
    if (!_hasCard) {
      Log.error(F("SD  : Card not initialized." CR));
      return 0;
    }
    return SD_MMC.usedBytes();
  }

  FS &getFS() const { return SD_MMC; }

  void listFiles(const char *dir = "/", uint8_t levels = 0) {
    if (!_hasCard) {
      Log.error(F("SD  : Card not initialized." CR));
      return;
    }
    File root = SD_MMC.open(dir);
    if (!root) {
      Log.error(F("SD  : Failed to open directory %s." CR), dir);
      return;
    }
    if (!root.isDirectory()) {
      Log.error(F("SD  : Not a directory: %s." CR), dir);
      root.close();
      return;
    }
    File file = root.openNextFile();
    while (file) {
      if (file.isDirectory()) {
        Log.notice(F("SD  : Dir : %s" CR), file.name());
        if (levels) {
          listFiles(file.name(), levels - 1);
        }
      } else {
        Log.notice(F("SD  : File : %s  Size : %d" CR), file.name(),
                   file.size());
      }
      file = root.openNextFile();
    }
    root.close();
  }
};

#endif  // ENABLE_MMC

#endif  // SRC_SDCARD_MMC_HPP_

// EOF
