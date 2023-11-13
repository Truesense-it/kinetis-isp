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
#ifdef __APPLE__
#include <sys/ioctl.h>
#include <IOKit/serial/ioss.h>
#endif


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

//termios.h implementation for MacOSX does not contain declarations of baud rates above 460800
//so you have to set correct values for the corresponding undefined speeds!
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
        #if !defined(_POSIX_C_SOURCE) || defined(_DARWIN_C_SOURCE)
            return 460800;
        #else
            return B460800;
        #endif
    case 500000:
        #if !defined(_POSIX_C_SOURCE) || defined(_DARWIN_C_SOURCE)
            return 500000;
        #else
            return B500000;
        #endif
    case 576000:
        #if !defined(_POSIX_C_SOURCE) || defined(_DARWIN_C_SOURCE)
            return 576000;
        #else
            return B576000;
        #endif
    case 921600:
        #if !defined(_POSIX_C_SOURCE) || defined(_DARWIN_C_SOURCE)
            return 921600;
        #else
            return B921600;
        #endif
    case 1000000:
        #if !defined(_POSIX_C_SOURCE) || defined(_DARWIN_C_SOURCE)
            return 1000000;
        #else
            return B1000000;
        #endif
    case 1152000:
        #if !defined(_POSIX_C_SOURCE) || defined(_DARWIN_C_SOURCE)
            return 1152000;
        #else
            return B1152000;
        #endif
    case 1500000:
        #if !defined(_POSIX_C_SOURCE) || defined(_DARWIN_C_SOURCE)
            return 1500000;
        #else
            return B1500000;
        #endif
    case 2000000:
        #if !defined(_POSIX_C_SOURCE) || defined(_DARWIN_C_SOURCE)
            return 2000000;
        #else
            return B2000000;
        #endif
    case 2500000:
        #if !defined(_POSIX_C_SOURCE) || defined(_DARWIN_C_SOURCE)
            return 2500000;
        #else
            return B2500000;
        #endif
    case 3000000:
        #if !defined(_POSIX_C_SOURCE) || defined(_DARWIN_C_SOURCE)
            return 3000000;
        #else
            return B3000000;
        #endif
    case 3500000:
        #if !defined(_POSIX_C_SOURCE) || defined(_DARWIN_C_SOURCE)
            return 3500000;
        #else
            return B3500000;
        #endif
    case 4000000:
        #if !defined(_POSIX_C_SOURCE) || defined(_DARWIN_C_SOURCE)
            return 4000000;
        #else
            return B4000000;
        #endif
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
#ifndef __APPLE__
    int rc = tcsetattr(fd, TCSANOW, &tty);
#else
    int rc = ioctl(fd, IOSSIOSPEED, &speed);
#endif
    
    if (rc != 0) {

        printf("Error from tcsetattr: %s\n", strerror(errno));
        return errno;


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
 this->fd = ::open(dev.c_str(), O_RDWR | O_NOCTTY | O_NDELAY);
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
  UNUSED(ftdi);
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
