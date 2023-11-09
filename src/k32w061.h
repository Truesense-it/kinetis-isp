/*******************************************************************************
 *
 * Copyright (c) 2020 Albert Krenz
 * 
 * This code is licensed under BSD + Patent (see LICENSE.txt for full license text)
 *******************************************************************************/

 /* SPDX-License-Identifier: BSD-2-Clause-Patent */
#ifndef _K32W061_H_
#define _K32W061_H_

#include "ftdi.hpp"
#include "mcu.h"

#include <cstdint>
#include <array>

class K32W061 : public MCU{
public:
  K32W061(FTDI::Interface &dev);
  ~K32W061();

  static const unsigned int CHIP_ID_K32W061=0x88888888;

  int enableISPMode(const std::vector<uint8_t> key={}) override;
  DeviceInfo getDeviceInfo() override;
  int eraseMemory(uint8_t handle) override;
  int getMemoryHandle(const MemoryID) override;
  bool memoryIsErased(uint8_t handle) override;
  int flashMemory(uint8_t handle, const std::vector<uint8_t>& data) override;
  int closeMemory(uint8_t handle) override;
  int reset() override;

  int setBaudrate(uint32_t speed) override;

protected:
  void insertCrc(std::vector<uint8_t>& data, unsigned long crc) const;
  unsigned long calculateCrc(const std::vector<uint8_t>& data) const;
  unsigned long extractCrc(std::vector<uint8_t> data) const;
private:
  FTDI::Interface &dev;
};

#endif /* _K32W061_H_ */