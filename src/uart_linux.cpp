/*******************************************************************************
 *
 * Copyright (c) 2020 Albert Krenz
 * 
 * This code is licensed under BSD + Patent (see LICENSE.txt for full license text)
 *******************************************************************************/

 /* SPDX-License-Identifier: BSD-2-Clause-Patent */
//#include "ftdi_linux.h"

#include <ftdi.h>
#include <stdexcept>
#include <memory.h>
#include <array>
#include <iostream>
#include <errno.h>
#include <fcntl.h> 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <boost/log/trivial.hpp>
#include "uart_linux.h"
#define UNUSED(x) (void)(x)

static int set_interface_attribs(int fd, int speed)
{
    struct termios tty;

    if (tcgetattr(fd, &tty) < 0) {
        printf("Error from tcgetattr: %s\n", strerror(errno));
        return -1;
    }

    cfsetospeed(&tty, (speed_t)speed);
    cfsetispeed(&tty, (speed_t)speed);

    tty.c_cflag |= (CLOCAL | CREAD);    /* ignore modem controls */
    tty.c_cflag &= ~CSIZE;
    tty.c_cflag |= CS8;         /* 8-bit characters */
    tty.c_cflag &= ~PARENB;     /* no parity bit */
    tty.c_cflag &= ~CSTOPB;     /* only need 1 stop bit */
    tty.c_cflag &= ~CRTSCTS;    /* no hardware flowcontrol */

    /* setup for non-canonical mode */
    tty.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL | IXON);
    tty.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
    tty.c_oflag &= ~OPOST;

    /* fetch bytes as they become available */
    tty.c_cc[VMIN] = 0;
    tty.c_cc[VTIME] = 0;

    if (tcsetattr(fd, TCSANOW, &tty) != 0) {
        printf("Error from tcsetattr: %s\n", strerror(errno));
        return -1;
    }
    return 0;
}

int get_baud(int baud)
{
    switch (baud) {
    case 9600:
        return B9600;
    case 19200:
        return B19200;
    case 38400:
        return B38400;
    case 57600:
        return B57600;
    case 115200:
        return B115200;
    case 230400:
        return B230400;
    case 460800:
        return B460800;
    case 500000:
        return B500000;
    case 576000:
        return B576000;
    case 921600:
        return B921600;
    case 1000000:
        return B1000000;
    case 1152000:
        return B1152000;
    case 1500000:
        return B1500000;
    case 2000000:
        return B2000000;
    case 2500000:
        return B2500000;
    case 3000000:
        return B3000000;
    case 3500000:
        return B3500000;
    case 4000000:
        return B4000000;
    default: 
        return -1;
    }
}
int set_baudrate(int fd, uint32_t speed)
{
    struct termios tty;
    int rc1, rc2;
    printf("Setting baudrate to %d", speed);
    if (tcgetattr(fd, &tty) < 0) {
        printf("Error from tcgetattr: %s\n", strerror(errno));
        return -1;
    }
    rc1 = cfsetospeed(&tty, get_baud(speed));
    rc2 = cfsetispeed(&tty, get_baud(speed));
    if ((rc1 | rc2) != 0 ) {
        printf("Error from cfsetxspeed: %s\n", strerror(errno));
        return -1;
    }
    if (tcsetattr(fd, TCSANOW, &tty) != 0) {
        printf("Error from tcsetattr: %s\n", strerror(errno));
        return -1;
    }
    tcflush(fd, TCIOFLUSH);  /* discard buffers */

    return 0;
}

UARTLinux::UARTLinux(){

}

UARTLinux::UARTLinux(const int vid, const int pid){
  UNUSED(vid);
  UNUSED(pid);
  //open(vid, pid);
}

UARTLinux::~UARTLinux(){
  //if(ftdi != nullptr){
  //  ftdi_usb_close(ftdi);
  //  ftdi_free(ftdi);
  //}
}

void UARTLinux::open(std::string dev)
{
 this->fd = ::open(dev.c_str(), O_RDWR | O_NOCTTY | O_SYNC);
    if (fd < 0) {
        printf("Error opening %s: %s\n", dev.c_str(), strerror(errno));
        //return -1;
    }
    /*baudrate 115200, 8 bits, no parity, 1 stop bit */
    set_interface_attribs(this->fd, B115200); 
}
void UARTLinux::open(const int vid, const int pid){
  UNUSED(vid);
  UNUSED(pid);
  // ftdi = ftdi_new();
  // if(ftdi == nullptr){
  //   throw std::runtime_error("Could not create new FTDI instance");
  // }

  // /* Open FTDILinux device based on FT232R vendor & product IDs */
  // if(ftdi_usb_open(ftdi, vid, pid) < 0) {
  //   ftdi_free(ftdi);
  //   ftdi=nullptr;
  //   throw std::runtime_error("Could not open USB Device");
  // }
  
  // if(ftdi_set_baudrate(ftdi, 115200) < 0){
  //   ftdi_usb_close(ftdi);
  //   ftdi_free(ftdi);
  //   ftdi=nullptr;
  //   throw std::runtime_error("Could not set baudrate to 115200 Baud/s");
  // }

  // if(ftdi_set_line_property(ftdi, BITS_8, STOP_BIT_1, NONE) <0){
  //   ftdi_usb_close(ftdi);
  //   ftdi_free(ftdi);
  //   ftdi=nullptr;
  //   throw std::runtime_error("Could not set FTDI Line Properties to 8 Bit, 1 Stop bit and no parity");
  // }
}

bool UARTLinux::is_open(){
  if(this->fd != 0) return true;
  return false;
}

int UARTLinux::setCBUSPins(const FTDI::CBUSPins& pins){
  UNUSED(pins);
  // uint8_t bitmask = pins.outputCBUS0 | (pins.outputCBUS1 << 1) | (pins.outputCBUS2 << 2) | (pins.outputCBUS3 << 3) | (pins.modeCBUS0 << 4) | (pins.modeCBUS1 << 5) | (pins.modeCBUS2 << 6) | (pins.modeCBUS3 << 7);
  // int ret = ftdi_set_bitmode(ftdi, bitmask, BITMODE_CBUS);
  // return ret;
  return 0;
}

int UARTLinux::disableCBUSMode(){
  //return ftdi_disable_bitbang(ftdi); 
  return 0;
}

int UARTLinux::writeData(std::vector<uint8_t> data){
  int ret = ::write(this->fd, data.data(), data.size()); //ftdi_write_data(ftdi, data.data(), data.size());
  //usleep(10000);
  //BOOST_LOG_TRIVIAL(info) << "wrote " << ret << " bytes out of " << data.size();
  if(ret != (int)data.size()){
    return -1;
  }
  tcdrain(this->fd);
  return data.size();
}

std::vector<uint8_t> UARTLinux::readData(){
  std::array<uint8_t, 100> buf{0};
  std::vector<uint8_t> data;
  int ret = 0;
  do{
    ret = ::read(this->fd, buf.data(), buf.size());

  }while(ret == 0);
  // BOOST_LOG_TRIVIAL(info) << "read " << ret << " bytes";
  if(ret > 0){
    data.resize(ret);
    std::copy(std::begin(buf), std::begin(buf)+ret, std::begin(data));
  }
      
  return data;
}
int UARTLinux::setBaudrate(uint32_t speed)
{
  if(this->fd!=0)
  {
    return set_baudrate(this->fd,speed);
  }
  return -1;
}
