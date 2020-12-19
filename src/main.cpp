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
#include <boost/log/core.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/log/sinks/text_file_backend.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/sources/record_ostream.hpp>
// #include <boost/log/utility/setup/file.hpp>

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
    ("verbose,v", "Enable Verbose Output")
  ;

  try{
    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm); 

    if (vm.count("help") || vm.count("h")) {
      std::cout << "nxp-isp Version " << VERSION << std::endl;
      std::cout << desc << std::endl;
      return 0;
    }

    boost::log::add_console_log(std::clog,boost::log::keywords::format = "%TimeStamp% [%Severity%]: %Message%");
    boost::log::core::get()->set_filter (boost::log::trivial::severity >= boost::log::trivial::warning);
    boost::log::add_common_attributes();
    if(vm.count("verbose")){
      boost::log::core::get()->set_filter (boost::log::trivial::severity >= boost::log::trivial::info);
    }

    FTDILinux ftdi = {};
    BOOST_LOG_TRIVIAL(info) <<  "Open FTDI Device 0403:6015";
    ftdi.open(0x0403, 0x6015);
    K32W061 mcu(ftdi);
    Application app(mcu, ftdi);
    BOOST_LOG_TRIVIAL(info) <<  "Enable ISP Mode";
    app.enableISPMode();
    BOOST_LOG_TRIVIAL(info) <<  "ISP Mode Enabled";

    if(vm.count("device-info") || vm.count("d")){
      BOOST_LOG_TRIVIAL(info) << "Read Device Info";
      app.deviceInfo();
    }

    if(vm.count("erase")){
      BOOST_LOG_TRIVIAL(info) << "Erase Memory " << vm["erase"].as<std::string>();
      app.eraseMemory(stringToMemID(vm["erase"].as<std::string>()));
      BOOST_LOG_TRIVIAL(info) << "Memory " << vm["erase"].as<std::string>() << " erased";
    }

    if(vm.count("firmware")){
      BOOST_LOG_TRIVIAL(info) <<  "Open file " << vm["firmware"].as<std::string>();
      std::ifstream ifs(vm["firmware"].as<std::string>(), std::ios::binary);
      FirmwareReader fw(ifs);
      BOOST_LOG_TRIVIAL(info) << "Flash Firmware";
      app.flashFirmware(fw.data());
      BOOST_LOG_TRIVIAL(info) << "Success";
    }
  }catch(const std::exception& e){
    BOOST_LOG_TRIVIAL(error) << e.what();
    exit(EXIT_FAILURE);
  }
  
  return EXIT_SUCCESS;
}