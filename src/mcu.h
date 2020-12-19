#ifndef _MCU_H_
#define _MCU_H_

#include <cstdint>
#include <vector>

class MCU
{
  public:
  struct DeviceInfo{
    std::uint32_t chipId;
    std::uint32_t version;
  };
  
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

  virtual int enableISPMode(const std::vector<uint8_t> key) = 0;
  virtual DeviceInfo getDeviceInfo() = 0;
  virtual int eraseMemory(uint8_t handle) = 0;
  virtual int getMemoryHandle(const MemoryID) = 0;
  virtual bool memoryIsErased(uint8_t handle) = 0;
  virtual int flashMemory(uint8_t handle, const std::vector<uint8_t>& data) = 0;
  virtual int closeMemory(uint8_t handle) = 0;
  virtual int reset() = 0;
};

#endif /* _MCU_H_ */