#include "ftdi_linux.h"
#include "k32w061.h"

#include <iostream>
#include <unistd.h>

#define CHIP_ID_K32W061 0x88888888

int main(void){

  FTDILinux ftdi = {};
  auto ret = ftdi.open(0x0403, 0x6015);
  if(ret < 0){
    std::cerr << "Could not open Device 0403:6015"<< std::endl;
    exit(EXIT_FAILURE);
  }

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

  K32W061 device(ftdi);
  ret = device.enableISPMode();
  if(ret != 0){
    std::cerr << "Could not enable ISP Mode" << std::endl;
    exit(EXIT_FAILURE);
  }
  K32W061::DeviceInfo dev_info = device.getDeviceInfo();
  switch(dev_info.chipId){
    case CHIP_ID_K32W061:{
      std::cout << "Found Chip K32W061" << std::endl;
      break;
    }
    default:{
      std::cerr << "Found unknown chip ID " << dev_info.chipId << std::endl;
      exit(EXIT_FAILURE);
    }
  }
  std::cout << "Chip Version " << dev_info.version << std::endl;
  


  return 0;
}