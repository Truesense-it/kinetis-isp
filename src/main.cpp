/*******************************************************************************
 *
 * Copyright (c) 2020 Albert Krenz
 * 
 * This code is licensed under BSD + Patent (see LICENSE.txt for full license text)
 *******************************************************************************/

 /* SPDX-License-Identifier: BSD-2-Clause-Patent */
#include "ftdi_linux.h"
#include "k32w061.h"
#include "application.h"
#include "firmware_reader.h"

#include <iostream>
#include <boost/program_options.hpp>

#define CHIP_ID_K32W061 0x88888888

namespace po = boost::program_options;

MCU::MemoryID stringToMemID(const std::string str){
  MCU::MemoryID id;
  if(str == "FLASH"){
    id = MCU::MemoryID::flash;
  }else if(str == "PSECT"){
    id = MCU::MemoryID::psect;
  }else if(str == "PFLASH"){
    id = MCU::MemoryID::pflash;
  }else if(str == "CONFIG"){
    id = MCU::MemoryID::config;
  }else if(str == "EFUSE"){
    id = MCU::MemoryID::efuse;
  }else if(str == "ROM"){
    id = MCU::MemoryID::rom;
  }else{
    throw std::runtime_error(std::string("Unknown Memory Type \"") + str + std::string("\""));
  }

  return id;
}

int main(int argc, const char* argv[]){

  po::options_description desc("Options");
  desc.add_options()
    ("help,h", "Print this help Message")
    ("device-info,d", "Show Device Information from Chip")
    ("erase,e", po::value<std::string>(), "Erase Memory. Available Types are: FLASH, PSECT, PFLASH, CONFIG, EFUSE, ROM")
    ("firmware,f", po::value<std::string>(), "Path Firmware Binary")
    ("interface,i", po::value<std::string>(), "Path to Interface /dev/ttyUSBX")
  ;

  try{
    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm); 

    if (vm.count("help") || vm.count("h")) {
      std::cout << desc << std::endl;
      return 0;
    }

    FTDILinux ftdi = {};
    ftdi.open(0x0403, 0x6015);
    K32W061 mcu(ftdi);
    Application app(mcu, ftdi);
    app.enableISPMode();

    if(vm.count("device-info") || vm.count("d")){
      app.deviceInfo();
    }

    if(vm.count("erase")){
      app.eraseMemory(stringToMemID(vm["erase"].as<std::string>()));
    }

    if(vm.count("firmware")){
      std::ifstream ifs(vm["firmware"].as<std::string>(), std::ios::binary);
      FirmwareReader fw(ifs);
      app.flashFirmware(fw.data());
    }
  }catch(const std::exception& e){
    std::cerr << e.what() << std::endl;
    exit(EXIT_FAILURE);
  }
  
  return EXIT_SUCCESS;
}