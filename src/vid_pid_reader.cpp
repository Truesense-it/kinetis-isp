/*******************************************************************************
 *
 * Copyright (c) 2020 Albert Krenz
 * 
 * This code is licensed under BSD + Patent (see LICENSE.txt for full license text)
 *******************************************************************************/

 /* SPDX-License-Identifier: BSD-2-Clause-Patent */
#include "vid_pid_reader.h"

#include <vector>
#include <fstream>
#include <boost/algorithm/string.hpp>
#include <boost/log/trivial.hpp>

std::tuple<int, int> VIDPIDReader::getVidPidForDev(std::string dev){
  std::vector<std::string> results;
  boost::split(results, dev, [](char c){return c == '/';});
  
  std::string pid_path = std::string("/sys/class/tty/") + results.back() + std::string("/../../../../idProduct");
  BOOST_LOG_TRIVIAL(info) << "Read PID from " << pid_path;
  std::ifstream pid_ifs(pid_path.c_str());
  if(!pid_ifs.is_open()){
    throw std::runtime_error("Could not read PID");
  }

  std::string vid_path = std::string("/sys/class/tty/") + results.back() + std::string("/../../../../idVendor");
  BOOST_LOG_TRIVIAL(info) << "Read VID from " << vid_path;
  std::ifstream vid_ifs(vid_path.c_str());
  if(!vid_ifs.is_open()){
    throw std::runtime_error(std::string("Could not read VID for ") + dev);
  }
  int vid = 0;
  int pid = 0;
  pid_ifs >> std::hex >> pid;
  vid_ifs >> std::hex >> vid;

  BOOST_LOG_TRIVIAL(info) << "VID/PID for " << dev << " are 0x" << std::hex << vid << "/0x" << std::hex << pid;
  return std::make_tuple(vid, pid);
}