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
#include <boost/log/trivial.hpp>

Application::Application(MCU& mcu, FTDI::Interface& ftdi) : mcu(mcu), ftdi(ftdi)
{
}

Application::~Application()
{
}

void Application::deviceInfo(){
  BOOST_LOG_TRIVIAL(info) <<  "Get Device Information";
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
  std::cout << "Chip Version 0x" << std::hex << dev_info.version << std::endl;
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
  BOOST_LOG_TRIVIAL(info) <<  "Set all CBUS Pins to Output:0";
  ftdi.setCBUSPins(pins);
  pins.outputCBUS2 = 1;
  usleep(1000);
  BOOST_LOG_TRIVIAL(info) <<  "Set CBUS Pin 2 to Output:1";
  ftdi.setCBUSPins(pins);
  usleep(10000);
  BOOST_LOG_TRIVIAL(info) <<  "Disable CBUS Mode";
  ftdi.disableCBUSMode();
  usleep(10000);

  const std::vector<uint8_t> unlock_key={0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88};
  BOOST_LOG_TRIVIAL(info) <<  "Send \"Enable ISP Mode\" request to device";
  auto ret = mcu.enableISPMode(unlock_key);
  if(ret != 0){
    throw std::runtime_error("Could not enable ISP Mode");
  }
}

void Application::eraseMemory(MCU::MemoryID id){
  BOOST_LOG_TRIVIAL(info) <<  "Get Handle for memory";
  auto handle = mcu.getMemoryHandle(id);
  if(handle < 0){
    throw std::runtime_error("Could not get Handle for Memory");
  }
  BOOST_LOG_TRIVIAL(info) <<  "Got handle " << handle;

  BOOST_LOG_TRIVIAL(info) <<  "Erase memory with handle " << handle;
  auto ret = mcu.eraseMemory(handle);
  if(ret < 0){
    throw std::runtime_error("Could not erase Memory");
  }

  BOOST_LOG_TRIVIAL(info) <<  "Check if Memory has been erased ...";
  ret = mcu.memoryIsErased(handle);
  if(ret < 0){
    throw std::runtime_error("Memory not successfully erased");
  }
  BOOST_LOG_TRIVIAL(info) <<  "Success";

  BOOST_LOG_TRIVIAL(info) <<  "Close memory Handle " << handle;
  ret = mcu.closeMemory(handle);
  if(ret < 0){
    throw std::runtime_error("Closing Memory handle failed");
  }
}

void Application::flashFirmware(const std::vector<uint8_t>& fw){
  BOOST_LOG_TRIVIAL(info) <<  "Get Handle to Flash memory";
  auto handle = mcu.getMemoryHandle(MCU::MemoryID::flash);
  if(handle < 0){
    throw std::runtime_error("Could not get Handle for Memory");
  }

  BOOST_LOG_TRIVIAL(info) <<  "Start flashing Firmware";
  auto ret = mcu.flashMemory(handle, fw);
  if(ret != 0){
    throw std::runtime_error(std::string("Only ") + std::to_string(ret) + std::string(" bytes written of ") + std::to_string(fw.size()) + std::string(" bytes"));
  }

  BOOST_LOG_TRIVIAL(info) <<  "Close Memory Handle " << handle;
  ret = mcu.closeMemory(handle);
  if(ret < 0){
    throw std::runtime_error("Closing Memory handle failed");
  }
}