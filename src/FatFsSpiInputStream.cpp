#include "FatFsSpiInputStream.h"
#include <pico/printf.h>

#ifdef DEBUG_FAT_SPI
#define DBG_PRINTF(...) printf(__VA_ARGS__)
#else
#define DBG_PRINTF(...)
#endif

FatFsSpiInputStream::FatFsSpiInputStream(SdCardFatFsSpi* sdCard, const char* name) :
  _sdCard(sdCard),
  _eof(false),
  _open(false)
{
  if (!_sdCard->mounted()) {
    if (!_sdCard->mount()) {
      DBG_PRINTF("Failed to mount SD card\n");
      return;
    }
  }
  
  DBG_PRINTF("openning file %s for read\n", name);
  _fr = f_open(&_fil, name, FA_READ|FA_OPEN_EXISTING);
  if (FR_OK != _fr && FR_EXIST != _fr) {
    DBG_PRINTF("f_open(%s) error: %s (%d)\n", name, FRESULT_str(_fr), _fr);
  }
  else {
    DBG_PRINTF("openned file %s ok!\n", name);
    _open = true;
    _fr = FR_OK;
  }
}

int32_t FatFsSpiInputStream::readByte() {
  uint8_t b;
  int32_t r = read(&b, 1);
  return r < 0 ? r : b;
}

int32_t FatFsSpiInputStream::read(uint8_t* buffer, const uint32_t length) {
  if (_eof || !_open) return -1;
  if (FR_OK != _fr) return -2; 
  UINT br = 0;
  _fr = f_read(&_fil, buffer, length, &br);
  _eof = br == 0;
  if (_eof) {
    DBG_PRINTF("eof\n");
    return -1;
  }
  if (_fr != FR_OK) {
    DBG_PRINTF("f_read(%s) error: (%d)\n", FRESULT_str(_fr), _fr);
    return -2;
  }
  return br;
}

void FatFsSpiInputStream::close() {
  if (_open) {
    f_close(&_fil);
    _open = false;
  }
}

bool FatFsSpiInputStream::closed() {
  return !_open;
}

bool FatFsSpiInputStream::end() {
  return _eof;
}

int32_t FatFsSpiInputStream::seek(const uint32_t pos) {
  if (_eof || !_open) return -1;
  if (FR_OK != _fr) return -2;
  _fr = f_lseek(&_fil, pos);
  if (_fr != FR_OK) {
    DBG_PRINTF("f_seek(%s) error: (%d)\n", FRESULT_str(_fr), _fr);
    return -2;
  }
  return 0;
}

int32_t FatFsSpiInputStream::rseek(const int32_t rpos) {
  if (_eof || !_open) return -1;
  if (FR_OK != _fr) return -2;
  return seek(rpos - f_tell(&_fil));
}

uint32_t FatFsSpiInputStream::pos() {
  if (_eof || !_open) return 0;
  if (FR_OK != _fr) return 0;
  return f_tell(&_fil);
}

