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

#include <iostream>
#include <boost/program_options.hpp>

#define CHIP_ID_K32W061 0x88888888

namespace po = boost::program_options;

int main(int argc, const char* argv[]){

  po::options_description desc("Options");
  std::string interface{};
  desc.add_options()
    ("help,h", "Print this help Message")
    ("device-info,d", "Show Device Information from Chip")
    ("erase,e", "Erase Memory")
    ("interface,i", po::value<std::string>(&interface), "Path to Interface /dev/ttyUSBX")
  ;

  po::variables_map vm;
  po::store(po::parse_command_line(argc, argv, desc), vm);
  po::notify(vm); 

  if (vm.count("help") || vm.count("h")) {
    std::cout << desc << std::endl;
    return 0;
  }

  FTDILinux ftdi = {};
  auto ret = ftdi.open(0x0403, 0x6015);
  if(ret < 0){
    std::cerr << "Could not open Device 0403:6015"<< std::endl;
    exit(EXIT_FAILURE);
  }
  K32W061 mcu(ftdi);
  Application app(mcu, ftdi);

  try{
    app.enableISPMode();
  if(vm.count("device-info") || vm.count("d")){
    app.deviceInfo();
  }
  if(vm.count("erase") || vm.count("e")){
    app.eraseMemory();
  }
  }catch(const std::exception& e){
    std::cerr << e.what() << std::endl;
    exit(EXIT_FAILURE);
  }
  
  return EXIT_SUCCESS;
}