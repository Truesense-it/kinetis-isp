/*******************************************************************************
 *
 * Copyright (c) 2020 Albert Krenz
 * 
 * This code is licensed under BSD + Patent (see LICENSE.txt for full license text)
 *******************************************************************************/

 /* SPDX-License-Identifier: BSD-2-Clause-Patent */
#ifndef _APPLICATION_H_
#define _APPLICATION_H_

#include "mcu.h"
#include "ftdi.hpp"

#include <string>

class Application
{
public:
  Application(MCU& mcu, FTDI::Interface& ftdi);
  ~Application();

  void enableISPMode();
  void deviceInfo();
  void eraseMemory(MCU::MemoryID id);
  void flashFirmware(const std::vector<uint8_t>& fw);
  void reset();
  void setBaudrate(uint32_t speed);

private:
  MCU& mcu;
  FTDI::Interface& ftdi;
};

#endif /* _APPLICATION_H_ */