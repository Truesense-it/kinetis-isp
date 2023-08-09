/*******************************************************************************
 *
 * Copyright (c) 2020 Albert Krenz
 * 
 * This code is licensed under BSD + Patent (see LICENSE.txt for full license text)
 *******************************************************************************/

 /* SPDX-License-Identifier: BSD-2-Clause-Patent */
#include "k32w061.h"
#include <iostream>
#include <unistd.h>
#include <arpa/inet.h>
#include <boost/crc.hpp>
#include <boost/log/core.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/trivial.hpp>
#include "ftdi.hpp"

#define CRC_SIZE 4

enum FrameType : uint8_t{
  ResetReq = 0x14,
  ResetResp = 0x15,
  ExecuteReq = 0x21,
  GetDeviceInfoReq = 0x32,
  GetDeviceInfoResp = 0x33,
  OpenMemoryForAccessReq = 0x40,
  OpenMemoryForAccessResp = 0x41,
  EraseMemoryReq = 0x42,
  EraseMemoryResp = 0x43,
  CheckBlankMemoryReq = 0x44,
  CheckBlankMemoryResp = 0x45,
  WriteMemoryReq = 0x48,
  WriteMemoryResp = 0x49,
  CloseMemoryReq = 0x4A,
  CloseMemoryResp = 0x4B,
  EnableISPModeReq = 0x4E,
  EnableISPModeResp = 0x4F,
  SetBaudRateReq = 0x27,
  SetBaudRateResp = 0x28
};

struct __attribute__((__packed__)) FrameHeader{
  uint8_t reserved:1;
  uint8_t hasSHA256Sig:1;
  uint8_t hasNextHash:1;
  uint8_t reserved2:5;
  uint16_t size;
  uint8_t type;
};

enum ResponseCode : uint8_t{
  Success = 0x00,
  MemoryInvalidMode = 0xEF,
  MemoryBadState = 0xF0,
  MemoryTooLong = 0xF1,
  MemoryOutOfRange = 0xF2,
  MemoryAccessInvalid = 0xF3,
  MemoryNotSupported = 0xF4,
  MemoryInvalid = 0xF5
};
struct __attribute__((__packed__)) ResponseHeader{
  uint8_t status;
};

static bool frameHasType(std::vector<uint8_t> frame, FrameType type){
  FrameHeader * header = reinterpret_cast<FrameHeader*>(frame.data());
  if(header->type != type){
    return false;
  }
  return true;
}

static bool responseHasSuccessStatus(std::vector<uint8_t> frame){
  ResponseHeader * header = reinterpret_cast<ResponseHeader*>(frame.data() + sizeof(FrameHeader));
  if(header->status != ResponseCode::Success){
    return false;
  }
  return true;
}

static uint8_t responseType(std::vector<uint8_t> frame){
  FrameHeader * header = reinterpret_cast<FrameHeader*>(frame.data());
  return header->type;
}

K32W061::K32W061(FTDI::Interface &dev) : dev(dev){

}

K32W061::~K32W061(){

}

int K32W061::enableISPMode(const std::vector<uint8_t> key){
  std::vector<uint8_t>req(9+key.size());
  req[0] = 0;
  req[1] = 0;
  req[2] = 9 + key.size();
  req[3] = 0x4e;
  
  if(key.size() == 0){
    req[4] = 0x00;
  }else{
    req[4] = 0x01;
    std::copy(key.begin(), key.end(), req.begin()+5);
  }

  auto crc = calculateCrc(req);
  insertCrc(req, crc);
  int count = dev.writeData(req);
  if(count != 9+static_cast<int>(key.size())){
    return -1;
  }

  auto data = dev.readData();
  if(data.size() == 0){
    return -1;
  }
  if(calculateCrc(data) != extractCrc(data)){
    return -1;
  }
  if(!frameHasType(data, FrameType::EnableISPModeResp)){
    return -1;
  }
  return 0;
}

K32W061::DeviceInfo K32W061::getDeviceInfo(){
  std::vector<uint8_t>req{0x00, 0x00, 0x08, 0x32, 0x00, 0x00, 0x00, 0x00};

  auto crc = calculateCrc(req);
  insertCrc(req, crc);
  int count = dev.writeData(req);
  if(count != 8){
    return K32W061::DeviceInfo();
  }

  K32W061::DeviceInfo dev_info;
  auto data = dev.readData();
  if( data.size() == 0 ||
      !frameHasType(data, FrameType::GetDeviceInfoResp) ||
      extractCrc(data) != calculateCrc(data) ||
      !responseHasSuccessStatus(data)){
    return K32W061::DeviceInfo();
  }

  struct __attribute__((__packed__)) DevInfoFrame{
    ResponseHeader header;
    uint32_t chipId;
    uint32_t chipVersion;
  };
  DevInfoFrame * dev_info_frame = reinterpret_cast<DevInfoFrame*>(data.data() + sizeof(FrameHeader));
  dev_info.version = dev_info_frame->chipVersion;
  dev_info.chipId = dev_info_frame->chipId;
  return dev_info;
}

int K32W061::eraseMemory(uint8_t handle){
  struct __attribute__((__packed__)) EraseMemoryHeader{
    uint8_t handle;
    uint8_t eraseMode;
    uint32_t address;
    uint32_t length;
  };
  std::vector<uint8_t> req(sizeof(FrameHeader) + sizeof(EraseMemoryHeader) + CRC_SIZE);
  FrameHeader * header = reinterpret_cast<FrameHeader*>(req.data());
  header->size = htons(sizeof(FrameHeader) + sizeof(EraseMemoryHeader) + CRC_SIZE);
  header->type = FrameType::EraseMemoryReq;
  auto erase_memory_header = reinterpret_cast<EraseMemoryHeader*>(req.data() + sizeof(FrameHeader));
  erase_memory_header->address = 0x00;
  erase_memory_header->handle = handle;
  erase_memory_header->length = 0x9DE00;
  erase_memory_header->eraseMode = 0x00;

  auto crc = calculateCrc(req);
  insertCrc(req, crc);

  auto ret = dev.writeData(req);
  if(ret != (sizeof(FrameHeader) + sizeof(EraseMemoryHeader) + CRC_SIZE)){
    return -1;
  }

  auto resp = dev.readData();
  if( resp.size() != (sizeof(FrameHeader) + CRC_SIZE + 1) ||
      !responseHasSuccessStatus(resp) || 
      !frameHasType(resp, FrameType::EraseMemoryResp) ||
      calculateCrc(resp) != extractCrc(resp)){
    return -1;
  }

  return 0;
}

#define convert(value) ((0x000000ff & value) << 24) | ((0x0000ff00 & value) << 8) | \
                       ((0x00ff0000 & value) >> 8) | ((0xff000000 & value) >> 24) 

int K32W061::setBaudrate(uint32_t speed){
  struct __attribute__((__packed__)) BaudrateHeader{
    uint8_t reserved;
    uint32_t speed;
    
  };
  std::vector<uint8_t> req(sizeof(FrameHeader) + sizeof(BaudrateHeader) + CRC_SIZE);
  FrameHeader * header = reinterpret_cast<FrameHeader*>(req.data());
  header->size = htons(sizeof(FrameHeader) + sizeof(BaudrateHeader) + CRC_SIZE);
  header->type = FrameType::SetBaudRateReq;
  auto baudrate_header = reinterpret_cast<BaudrateHeader*>(req.data() + sizeof(FrameHeader));
  baudrate_header->speed = convert(speed);
  
  auto crc = calculateCrc(req);
  insertCrc(req, crc);

  auto ret = dev.writeData(req);
  if(ret != (sizeof(FrameHeader) + sizeof(BaudrateHeader) + CRC_SIZE)){
    return -1;
  }
  dev.setBaudrate(speed);
  auto resp = dev.readData();
  if( resp.size() != (sizeof(FrameHeader) + CRC_SIZE + 1) ||
      !responseHasSuccessStatus(resp) || 
      !frameHasType(resp, FrameType::SetBaudRateResp) ||
      calculateCrc(resp) != extractCrc(resp)){
    return -1;
  }

  return 0;
}

int K32W061::getMemoryHandle(const K32W061::MemoryID id){
  struct __attribute__((__packed__)) OpenMemoryHeader{
    uint8_t memID;
    uint8_t accessMode;
  }; 
  std::vector<uint8_t> req(sizeof(FrameHeader) + sizeof(OpenMemoryHeader) + CRC_SIZE);
  OpenMemoryHeader * open_memory_header = reinterpret_cast<OpenMemoryHeader*>(req.data() + sizeof(FrameHeader));
  
  FrameHeader* frame_header = reinterpret_cast<FrameHeader*>(req.data());
  frame_header->type = FrameType::OpenMemoryForAccessReq;
  frame_header->size = htons(sizeof(FrameHeader) + sizeof(OpenMemoryHeader) + CRC_SIZE);
  open_memory_header->accessMode = 0x0F;
  open_memory_header->memID = id;
  
  auto crc = calculateCrc(req);
  insertCrc(req, crc);
  dev.writeData(req);

  auto resp = dev.readData();
  if( resp.size() == 0 ||
      !responseHasSuccessStatus(resp) ||
      calculateCrc(resp) != extractCrc(resp) ||
      !frameHasType(resp, FrameType::OpenMemoryForAccessResp)){
    return -1;
  }

  struct __attribute__((__packed__)) OpenMemoryResponse{
    uint8_t handle;
  };
  OpenMemoryResponse * resp_data = reinterpret_cast<OpenMemoryResponse*>(resp.data() + sizeof(FrameHeader) + sizeof(ResponseHeader));

  return resp_data->handle;
}

void K32W061::insertCrc(std::vector<uint8_t>& data, unsigned long crc) const{
  crc = ntohl(crc);

  data[data.size()-CRC_SIZE + 0] = crc & 0xFF;
  data[data.size()-CRC_SIZE + 1] = (crc & 0xFF00) >> 8;
  data[data.size()-CRC_SIZE + 2] = (crc & 0xFF0000) >> 16;
  data[data.size()-CRC_SIZE + 3] = (crc & 0xFF000000) >> 24;
}

unsigned long K32W061::calculateCrc(const std::vector<uint8_t>& data) const{
  boost::crc_32_type result;
  result.process_bytes(data.data(), data.size() - CRC_SIZE);
  return result.checksum();
}

unsigned long K32W061::extractCrc(std::vector<uint8_t> data) const{
  unsigned crc = 0;
  crc = data[data.size() - CRC_SIZE + 3];
  crc |= data[data.size() - CRC_SIZE + 2] << 8;
  crc |= data[data.size() - CRC_SIZE + 1] << 16;
  crc |= data[data.size() - CRC_SIZE + 0] << 24;

  return crc;
}

bool K32W061::memoryIsErased(uint8_t handle){
  struct __attribute__((__packed__)) checkBlankMemoryHeader{
    uint8_t handle;
    uint8_t mode;
    uint32_t address;
    uint32_t length;
  }; 
  std::vector<uint8_t> req(sizeof(FrameHeader) + sizeof(checkBlankMemoryHeader) + CRC_SIZE);
  FrameHeader * frame_header = reinterpret_cast<FrameHeader*>(req.data());
  checkBlankMemoryHeader * blank_memory_header = reinterpret_cast<checkBlankMemoryHeader*>(req.data() + sizeof(FrameHeader));
  frame_header->type = FrameType::CheckBlankMemoryReq;
  frame_header->size = htons(req.size());
  blank_memory_header->address = 0x00;
  blank_memory_header->length = 0x9DE00;
  blank_memory_header->handle = handle;
  blank_memory_header->mode = 0x00;

  auto crc = calculateCrc(req);
  insertCrc(req, crc);
  
  auto ret = dev.writeData(req);
  if(ret != static_cast<int>(req.size())){
    return false;
  }

  auto resp = dev.readData();
  if( resp.size() == 0 ||
      !frameHasType(resp, FrameType::CheckBlankMemoryResp) ||
      extractCrc(resp) != calculateCrc(resp) ||
      !responseHasSuccessStatus(resp)){
    return false;
  }
  return true;
}

int K32W061::flashMemory(uint8_t handle, const std::vector<uint8_t>& data){
  struct __attribute__((__packed__)) FlashMemoryHeader{
    uint8_t handle;
    uint8_t mode;
    uint32_t address;
    uint32_t length;
  }; 
  auto size = data.size();
  size_t chunk_size = 512;
  if(data.size() < chunk_size){
    chunk_size = data.size();
  }
  std::vector<uint8_t> req(sizeof(FrameHeader) + sizeof(FlashMemoryHeader) + chunk_size + CRC_SIZE);
  
  uint32_t offset = 0;
  do{
    FrameHeader * header = reinterpret_cast<FrameHeader*>(req.data());
    FlashMemoryHeader * flash_memory_header = reinterpret_cast<FlashMemoryHeader*>(req.data() + sizeof(FrameHeader));
    header->size = htons(sizeof(FrameHeader) + sizeof(FlashMemoryHeader) + chunk_size + CRC_SIZE);
    header->type = FrameType::WriteMemoryReq;
    flash_memory_header->handle = handle;
    flash_memory_header->address = offset;
    flash_memory_header->length = chunk_size;
    flash_memory_header->mode = 0x00;
    std::copy(std::begin(data)+offset, std::begin(data)+offset+chunk_size, req.begin() + sizeof(FrameHeader) + sizeof(FlashMemoryHeader));

    auto crc = calculateCrc(req);
    insertCrc(req, crc);

    BOOST_LOG_TRIVIAL(info) << "Write " << chunk_size << " Bytes at offset " << offset << std::endl;
    auto ret = dev.writeData(req);
    if(ret != (signed)req.size()){
      return -1;
    }

    /*auto*/ std::vector<uint8_t> resp = dev.readData();
    if(resp.size()<9)
    {
      std::vector<uint8_t> resp2 = dev.readData();
      resp.insert(std::end(resp), std::begin(resp2), std::end(resp2));
    }
      
      
   // if( resp.size() == 0 ||
    if( resp.size() < 9 ||
        extractCrc(resp) != calculateCrc(resp) ||
        !responseHasSuccessStatus(resp) ||
        responseType(resp) != FrameType::WriteMemoryResp){
      return -1;
    }

    size -= chunk_size;
    offset+= chunk_size;
    if(size < chunk_size){
      chunk_size = size;
      req.resize(sizeof(FrameHeader) + sizeof(FlashMemoryHeader) + chunk_size + CRC_SIZE);
    }
  }while(size != 0);
  
  
  return 0;
}

int K32W061::closeMemory(uint8_t handle){
  std::vector<uint8_t> req(9);
  FrameHeader * header = reinterpret_cast<FrameHeader*>(req.data());
  header->type = FrameType::CloseMemoryReq;
  header->size = htons(0x09);

  struct __attribute__((__packed__)) CloseMemoryHeader{
    uint8_t handle;
  };
  CloseMemoryHeader * close_memory = reinterpret_cast<CloseMemoryHeader*>(req.data() + sizeof(FrameHeader));
  close_memory->handle = handle;

  auto crc = calculateCrc(req);
  insertCrc(req, crc);

  if(dev.writeData(req) != static_cast<signed>(req.size())){
    return -1;
  };
  
  auto resp = dev.readData();
  if( resp.size() == 0 ||
      calculateCrc(resp) != extractCrc(resp) ||
      responseType(resp) != FrameType::CloseMemoryResp ||
      !responseHasSuccessStatus(resp)){
    return -1;
  }

  return 0;
}

int K32W061::reset()
{
  std::vector<uint8_t> req(8);
  FrameHeader * header = reinterpret_cast<FrameHeader*>(req.data());
  header->type = FrameType::ResetReq;
  header->size = htons(0x08);

  auto crc= calculateCrc(req);
  insertCrc(req, crc);

  if(dev.writeData(req) != 8){
    return -1;
  }

  auto resp = dev.readData();
  if( resp.size() == 0 ||
      extractCrc(resp) != calculateCrc(resp) ||
      responseType(resp) != FrameType::ResetResp ||
      !responseHasSuccessStatus(resp))
  {
    return -1;
  }

  return 0;
}