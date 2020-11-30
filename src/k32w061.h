#ifndef _K32W061_H_
#define _K32W061_H_

#include "ftdi.hpp"

#include <cstdint>
#include <array>

class K32W061 {
public:
  K32W061(FTDI::Interface &dev);
  ~K32W061();

  struct DeviceInfo{
    uint32_t chipId;
    uint32_t version;
  };

  int enableISPMode();
  DeviceInfo getDeviceInfo();
  int eraseMemory(uint8_t handle);

  enum MemoryID{
    flash = 0x00,
    psect = 0x01,
    pflash = 0x02,
    config = 0x03,
    efuse = 0x04,
    rom = 0x05,
    ram0 = 0x06,
    ram1 = 0x07
  };
  int openMemory(const MemoryID);

protected:
  void insertCrc(std::vector<uint8_t>& data, unsigned long crc) const;
  unsigned long calculateCrc(const std::vector<uint8_t> data) const;
  unsigned long extractCrc(std::vector<uint8_t> data) const;
private:
  FTDI::Interface &dev;
};

#endif /* _K32W061_H_ */