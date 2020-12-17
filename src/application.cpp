/*******************************************************************************
 *
 * Copyright (c) 2020 Albert Krenz
 * 
 * This code is licensed under BSD + Patent (see LICENSE.txt for full license text)
 *******************************************************************************/

 /* SPDX-License-Identifier: BSD-2-Clause-Patent */
#include "application.h"
#include "k32w061.h"

#include <iostream>
#include <unistd.h>
#include <stdexcept>

Application::Application(MCU& mcu, FTDI::Interface& ftdi) : mcu(mcu), ftdi(ftdi)
{
}

Application::~Application()
{
}

void Application::deviceInfo(){
  K32W061::DeviceInfo dev_info = mcu.getDeviceInfo();
  switch(dev_info.chipId){
    case K32W061::CHIP_ID_K32W061:{
      std::cout << "Found Chip K32W061" << std::endl;
      break;
    }
    default:{
      throw std::runtime_error("Found unknown chip ID");
    }
  }
  std::cout << "Chip Version " << dev_info.version << std::endl;
}

void Application::enableISPMode(){
  FTDI::CBUSPins pins = {};
  pins.outputCBUS0 = 0;
  pins.outputCBUS1 = 0;
  pins.outputCBUS2 = 0;
  pins.outputCBUS3 = 0;
  pins.modeCBUS0 = FTDI::CBUSMode::OUTPUT;
  pins.modeCBUS1 = FTDI::CBUSMode::OUTPUT;
  pins.modeCBUS2 = FTDI::CBUSMode::OUTPUT;
  pins.modeCBUS3 = FTDI::CBUSMode::OUTPUT;
  ftdi.setCBUSPins(pins);
  pins.outputCBUS2 = 1;
  usleep(1000);
  ftdi.setCBUSPins(pins);
  usleep(10000);
  ftdi.diableCBUSMode();

  const std::vector<uint8_t> unlock_key={0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88};
  auto ret = mcu.enableISPMode(unlock_key);
  if(ret != 0){
    throw std::runtime_error("Could not enable ISP Mode");
  }
}

void Application::eraseMemory(MCU::MemoryID id){
  auto handle = mcu.getMemoryHandle(id);
  if(handle < 0){
    throw std::runtime_error("Could not get Handle for Memory");
  }
  auto ret = mcu.eraseMemory(handle);
  if(ret < 0){
    throw std::runtime_error("Could not erase Memory");
  }

  ret = mcu.memoryIsErased(handle);
  if(ret < 0){
    throw std::runtime_error("Memory not successfully erased");
  }

  ret = mcu.closeMemory(handle);
  if(ret < 0){
    throw std::runtime_error("Closing Memory handle failed");
  }
}