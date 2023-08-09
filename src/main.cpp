/*******************************************************************************
 *
 * Copyright (c) 2020 Albert Krenz
 * 
 * This code is licensed under BSD + Patent (see LICENSE.txt for full license text)
 *******************************************************************************/

 /* SPDX-License-Identifier: BSD-2-Clause-Patent */
#include "ftdi_linux.h"
#include "uart_linux.h"
#include "k32w061.h"
#include "application.h"
#include "firmware_reader.h"
#include "vid_pid_reader.h"

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
    ("reset,r", "Reset device via ISP command")
    ("interface,i", po::value<std::string>()->default_value("/dev/ttyUSB0"), "Path to Interface /dev/ttyUSBX. If not specified defaults to /dev/ttyUSB0")
    ("verbose,v", "Enable Verbose Output")
    ("noftdi,n", "Don'tuse FTDI")
    ("speed,s",  po::value<std::uint32_t>(), "programming baudrate")
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
    
    Application *app_p;


    if (vm.count("noftdi") || vm.count("n")) {
      UARTLinux *ftdi = new UARTLinux();
      ftdi->open(vm["interface"].as<std::string>());
      K32W061 *mcu=new K32W061(*ftdi);
      app_p=new Application(*mcu, *ftdi);
      BOOST_LOG_TRIVIAL(info) <<  "Enable ISP Mode";
      app_p->enableISPMode();
      BOOST_LOG_TRIVIAL(info) <<  "ISP Mode Enabled";
    }
    else {
      FTDILinux *ftdi = new FTDILinux();
      BOOST_LOG_TRIVIAL(info) <<  "Open FTDI Device " << vm["interface"].as<std::string>();
      int vid = 0;
      int pid = 0;
      std::tie(vid, pid) = VIDPIDReader::getVidPidForDev(vm["interface"].as<std::string>());
      ftdi->open(vid, pid);
      K32W061 *mcu=new K32W061(*ftdi);
      app_p=new Application(*mcu, *ftdi);
      BOOST_LOG_TRIVIAL(info) <<  "Enable ISP Mode";
      app_p->enableISPMode();
      BOOST_LOG_TRIVIAL(info) <<  "ISP Mode Enabled";
      

    }
    

    if(vm.count("device-info") || vm.count("d")){
      BOOST_LOG_TRIVIAL(info) << "Read Device Info";
      app_p->deviceInfo();
    }
    if(vm.count("speed") || vm.count("s")){
      BOOST_LOG_TRIVIAL(info) << "Set baudrate to " << vm["speed"].as<std::uint32_t>();
      app_p->setBaudrate(vm["speed"].as<std::uint32_t>());
    }

    if(vm.count("erase")){
      BOOST_LOG_TRIVIAL(info) << "Erase Memory " << vm["erase"].as<std::string>();
      app_p->eraseMemory(stringToMemID(vm["erase"].as<std::string>()));
      BOOST_LOG_TRIVIAL(info) << "Memory " << vm["erase"].as<std::string>() << " erased";
    }

    if(vm.count("firmware")){
      BOOST_LOG_TRIVIAL(info) <<  "Open file " << vm["firmware"].as<std::string>();
      std::ifstream ifs(vm["firmware"].as<std::string>(), std::ios::binary);
      FirmwareReader fw(ifs);
      BOOST_LOG_TRIVIAL(info) << "Flash Firmware";
      app_p->flashFirmware(fw.data());
      BOOST_LOG_TRIVIAL(info) << "Success";
    }

    if(vm.count("reset")){
      BOOST_LOG_TRIVIAL(info) << "Reset device";
      app_p->reset();
      BOOST_LOG_TRIVIAL(info) << "Success";
    }
  }catch(const std::exception& e){
    BOOST_LOG_TRIVIAL(error) << e.what();
    exit(EXIT_FAILURE);
  }
  
  return EXIT_SUCCESS;
}