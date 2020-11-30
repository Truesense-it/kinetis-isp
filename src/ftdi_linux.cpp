/*******************************************************************************
 *
 * Copyright (c) 2020 Albert Krenz
 * 
 * This code is licensed under BSD + Patent (see LICENSE.txt for full license text)
 *******************************************************************************/

 /* SPDX-License-Identifier: BSD-2-Clause-Patent */
#include "ftdi_linux.h"

#include <ftdi.h>
#include <stdexcept>
#include <memory.h>
#include <array>
#include <iostream>

FTDILinux::FTDILinux(){

}

FTDILinux::FTDILinux(const int vid, const int pid){
  int ret = open(vid, pid);
  if(ret < 0) throw std::runtime_error("Could not open FTDILinux device");
}

FTDILinux::~FTDILinux(){
  if(ftdi != NULL){
    ftdi_usb_close(ftdi);
    ftdi_free(ftdi);
  }
}

int FTDILinux::open(const int vid, const int pid){
  ftdi = ftdi_new();
  if(ftdi == NULL){
    return -1;
  }

  /* Open FTDILinux device based on FT232R vendor & product IDs */
  if(ftdi_usb_open(ftdi, vid, pid) < 0) {
    ftdi_free(ftdi);
    return -1;
  }

  if(ftdi_set_baudrate(ftdi, 115200) < 0){
    ftdi_usb_close(ftdi);
    ftdi_free(ftdi);
    return -1;
  }

  if(ftdi_set_line_property(ftdi, BITS_8, STOP_BIT_1, NONE) <0){
    ftdi_usb_close(ftdi);
    ftdi_free(ftdi);
    return -1;
  }

  return 0;
}

bool FTDILinux::is_open(){
  if(ftdi != NULL) return true;
  return false;
}

int FTDILinux::setCBUSPins(const FTDI::CBUSPins& pins){
  uint8_t bitmask = pins.outputCBUS0 | (pins.outputCBUS1 << 1) | (pins.outputCBUS2 << 2) | (pins.outputCBUS3 << 3) | (pins.modeCBUS0 << 4) | (pins.modeCBUS1 << 5) | (pins.modeCBUS2 << 6) | (pins.modeCBUS3 << 7);
  int ret = ftdi_set_bitmode(ftdi, bitmask, BITMODE_CBUS);
  return ret;
}

int FTDILinux::diableCBUSMode(){
  return ftdi_disable_bitbang(ftdi); 
}

int FTDILinux::writeData(std::vector<uint8_t> data){
  int ret = ftdi_write_data(ftdi, data.data(), data.size());
  if(ret < 0){
    return -1;
  }

  return data.size();
}

std::vector<uint8_t> FTDILinux::readData(){
  std::array<uint8_t, 100> buf{0};
  std::vector<uint8_t> data;
  int ret = 0;
  do{
    ret = ftdi_read_data(ftdi, buf.data(), buf.size());

  }while(ret == 0);
  if(ret > 0){
    data.resize(ret);
    std::copy(std::begin(buf), std::begin(buf)+ret, std::begin(data));
  }
      
  return data;
}
