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
#include <zlib.h>
#include <arpa/inet.h>

#define CRC_SIZE 4

enum FrameType : uint8_t{
  ResetReq = 0x14,
  ExecuteReq = 0x21,
  OpenMemoryForAccessReq = 0x40,
  OpenMemoryForAccessResp = 0x41,
  EraseMemoryReq = 0x42,
  EraseMemoryResp = 0x43,
  CheckBlankMemoryReq = 0x44,
  CheckBlankMemoryResp = 0x45,
  EnableISPModeReq = 0x4E,
  EnableISPModeResp = 0x4F
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

K32W061::K32W061(FTDI::Interface &dev) : dev(dev){

}

K32W061::~K32W061(){

}

int K32W061::enableISPMode(){
  std::vector<uint8_t>req(9);
  req[0] = 0;
  req[1] = 0;
  req[2] = 9;
  req[3] = 0x4e;
  req[4] = 0;

  auto crc = calculateCrc(req);
  insertCrc(req, crc);
  int count = dev.writeData(req);
  if(count != 9){
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
  std::vector<uint8_t>buf(4);
  buf[0] = 0;
  buf[1] = 0;
  buf[2] = 8;
  buf[3] = 0x32;

  int count = dev.writeData(buf);
  if(count != 4){
    return K32W061::DeviceInfo();
  }

  K32W061::DeviceInfo dev_info;
  auto data = dev.readData();
  FrameHeader * header = reinterpret_cast<FrameHeader*>(data.data());
  if(header->type != 0x33){
    return dev_info;
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

unsigned long K32W061::calculateCrc(const std::vector<uint8_t> data) const{
  return crc32(0, data.data(), data.size() - CRC_SIZE);
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

