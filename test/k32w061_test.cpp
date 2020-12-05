/*******************************************************************************
 *
 * Copyright (c) 2020 Albert Krenz
 * 
 * This code is licensed under BSD + Patent (see LICENSE.txt for full license text)
 *******************************************************************************/

 /* SPDX-License-Identifier: BSD-2-Clause-Patent */
#include "ftdi_mock.h"
#include <k32w061.h>

#include <gtest/gtest.h>
#include <algorithm>

using ::testing::_;
using ::testing::Return;
using ::testing::ContainerEq;
using ::testing::AllOf;

int ftdi_usb_close(struct ftdi_context *ftdi){
  return 0;
}

MATCHER_P(MemoryIdIs, id, "") { return arg.at(4) == id; }
MATCHER_P(FrameTypeIs, type, "") { return arg.at(3) == type; }
MATCHER_P(FrameHeaderEq, header, "") { return std::equal(header.begin(), header.end(), arg.begin());}
MATCHER_P(FramePayloadEq, payload, "") {return std::equal(payload.begin(), payload.end(), std::begin(arg)+4);}
MATCHER_P(FrameCrcEq, crc, "") {
  for(auto val: crc){
    std::cout << std::hex << (unsigned)val << " ";
  }
  std::cout << std::endl;
  for(auto val: arg){
    std::cout << std::hex << (unsigned)val << " ";
  }
  std::cout << std::endl;
  return std::equal(crc.begin(), crc.end(), std::end(arg)-4);
  }



class K32W061_EnableISPMode : public testing::Test{
public:
  K32W061_EnableISPMode() : dev(ftdi){};
  virtual ~K32W061_EnableISPMode(){};

  virtual void SetUp(){};
  virtual void TearDown(){};
  FTDIMock ftdi;
  K32W061 dev;
};

class K32W061_OpenMemory : public K32W061_EnableISPMode {};
class K32W061_GetDeviceInfo : public K32W061_EnableISPMode {};
class K32W061_EraseMemory : public K32W061_EnableISPMode {};
class K32W061_MemoryIsErased : public K32W061_EnableISPMode {};
class K32W061_FlashMemory : public K32W061_EnableISPMode {};
class K32W061_CloseMemory : public K32W061_EnableISPMode {};

TEST_F(K32W061_EnableISPMode, callsReadAfterWrite){
  testing::Sequence s1;
  EXPECT_CALL(ftdi, writeData(_)).Times(1).WillOnce(Return(9));
  EXPECT_CALL(ftdi, readData()).Times(1).WillRepeatedly(Return(std::vector<uint8_t>()));
  dev.enableISPMode();
}

TEST_F(K32W061_EnableISPMode, writeEnableISPFrame){
  std::vector<uint8_t> enable_isp_frame(9);
  enable_isp_frame[0] = 0x0;
  enable_isp_frame[1] = 0x0;
  enable_isp_frame[2] = 0x9;
  enable_isp_frame[3] = 0x4E;
  enable_isp_frame[4] = 0x0;
  enable_isp_frame[5] = 0xA7;
  enable_isp_frame[6] = 0x09;
  enable_isp_frame[7] = 0xAE;
  enable_isp_frame[8] = 0x19;


  EXPECT_CALL(ftdi, writeData(ContainerEq(enable_isp_frame))).Times(1).WillOnce(Return(9));
  EXPECT_CALL(ftdi, readData()).Times(1).WillRepeatedly(Return(std::vector<uint8_t>()));
  dev.enableISPMode();
}

TEST_F(K32W061_EnableISPMode, sendFrameTypeEnableISP){
  EXPECT_CALL(ftdi, writeData(FrameTypeIs(0x4E))).Times(1).WillOnce(Return(9));
  EXPECT_CALL(ftdi, readData());
  dev.enableISPMode();
}

TEST_F(K32W061_EnableISPMode, verifyWriteFrameHeader){
  std::vector<uint8_t> req(4);
  req[0] = 0x0;
  req[1] = 0x0;
  req[2] = 0x9;
  req[3] = 0x4E;
  EXPECT_CALL(ftdi, writeData(FrameHeaderEq(req))).Times(1).WillOnce(Return(9));
  EXPECT_CALL(ftdi, readData());
  dev.enableISPMode();
}

TEST_F(K32W061_EnableISPMode, verifyWriteFrameCrc){
  std::vector<uint8_t> crc(4);
  crc[0] = 0xA7;
  crc[1] = 0x09;
  crc[2] = 0xAE;
  crc[3] = 0x19;
  EXPECT_CALL(ftdi, writeData(FrameCrcEq(crc))).Times(1).WillOnce(Return(9));
  EXPECT_CALL(ftdi, readData());
  dev.enableISPMode();
}

TEST_F(K32W061_EnableISPMode, failsIfResponseFrameTypeIsNot0x4F){
  std::vector<uint8_t> response(9);
  response[0] = 0x00;
  response[1] = 0x00;
  response[2] = 0x09;
  response[3] = 0x4A; // should be 0x4f, all other values are incorrect
  response[4] = 0x00;
  response[5] = 0xC3;
  response[6] = 0x65;
  response[7] = 0x6B;
  response[8] = 0x1D;

  EXPECT_CALL(ftdi, writeData(_)).Times(1).WillOnce(Return(9));
  EXPECT_CALL(ftdi, readData()).Times(1).WillRepeatedly(Return(response));
  auto ret = dev.enableISPMode();
  EXPECT_NE(ret, 0);
}

TEST_F(K32W061_EnableISPMode, failsIfresponseCrcIsWrong){
  std::vector<uint8_t> response(9);
  response[0] = 0x00;
  response[1] = 0x00;
  response[2] = 0x09;
  response[3] = 0x4f; 
  response[4] = 0x00;
  response[5] = 0xC3;
  response[6] = 0x65;
  response[7] = 0x6B;
  response[8] = 0x1F; // should be 1D

  EXPECT_CALL(ftdi, writeData(_));
  EXPECT_CALL(ftdi, readData()).WillRepeatedly(Return(response));
  auto ret = dev.enableISPMode();
  EXPECT_NE(ret, 0);
}

TEST_F(K32W061_OpenMemory, callsWriteAfterRead){
  testing::Sequence s1;
  EXPECT_CALL(ftdi, writeData(_)).Times(1);
  EXPECT_CALL(ftdi, readData()).Times(1);
  dev.getMemoryHandle(K32W061::MemoryID::flash);
}

TEST_F(K32W061_OpenMemory, sendsOpenMemoryRequest){
  std::vector<uint8_t> req(10);
  req[0] = 0x00;
  req[1] = 0x00;
  req[2] = 0x0A;
  req[3] = 0x40;
  req[4] = 0x00;
  req[5] = 0x0F;
  req[6] = 0x3E;
  req[7] = 0x5A;
  req[8] = 0xD1;
  req[9] = 0x96;


  EXPECT_CALL(ftdi, writeData(ContainerEq(req))).Times(1);
  EXPECT_CALL(ftdi, readData());
  dev.getMemoryHandle(K32W061::MemoryID::flash);
}

TEST_F(K32W061_OpenMemory, returnsSuccessOnValidResponse){
  std::vector<uint8_t> resp(10);
  resp[0] = 0x00;
  resp[1] = 0x00;
  resp[2] = 0x0A;
  resp[3] = 0x41;
  resp[4] = 0x00;
  resp[5] = 0x00;
  resp[6] = 0xAF;
  resp[7] = 0x27;
  resp[8] = 0xA6;
  resp[9] = 0x30;


  EXPECT_CALL(ftdi, writeData(_)).WillOnce(Return(10));
  EXPECT_CALL(ftdi, readData()).WillOnce(Return(resp));
  auto ret = dev.getMemoryHandle(K32W061::MemoryID::flash);
  EXPECT_EQ(ret, 0);
}

TEST_F(K32W061_OpenMemory, sendOpenMemoryFrameType){
  EXPECT_CALL(ftdi, writeData(FrameTypeIs(0x40))).Times(1);
  EXPECT_CALL(ftdi, readData());
  dev.getMemoryHandle(K32W061::MemoryID::psect);
}

TEST_F(K32W061_OpenMemory, openPsectMemory){
  EXPECT_CALL(ftdi, writeData(MemoryIdIs(0x01))).Times(1);
  EXPECT_CALL(ftdi, readData());
  dev.getMemoryHandle(K32W061::MemoryID::psect);
}

TEST_F(K32W061_OpenMemory, failsIfresponseCrcIsWrong){
  std::vector<uint8_t> response(10);
  response[0] = 0x00;
  response[1] = 0x00;
  response[2] = 0x0A;
  response[3] = 0x41;
  response[4] = 0x00;
  response[5] = 0x00;
  response[6] = 0xAF;
  response[7] = 0x27;
  response[8] = 0xA6;
  response[9] = 0x31; // should be 0x30

  EXPECT_CALL(ftdi, writeData(_));
  EXPECT_CALL(ftdi, readData()).WillRepeatedly(Return(response));
  auto ret = dev.getMemoryHandle(K32W061::MemoryID::flash);
  EXPECT_NE(ret, 0);
}

TEST_F(K32W061_OpenMemory, failsIfResponseFrameTypeIsNot0x41){
  std::vector<uint8_t> response(10);
  response[0] = 0x00;
  response[1] = 0x00;
  response[2] = 0x0A;
  response[3] = 0x42; // should be 0x41
  response[4] = 0x00;
  response[5] = 0x00;
  response[6] = 0xAD;
  response[7] = 0x61;
  response[8] = 0x18;
  response[9] = 0x69;

  EXPECT_CALL(ftdi, writeData(_));
  EXPECT_CALL(ftdi, readData()).WillRepeatedly(Return(response));
  auto ret = dev.getMemoryHandle(K32W061::MemoryID::flash);
  EXPECT_NE(ret, 0);
}

TEST_F(K32W061_OpenMemory, failsIfResponseNotSuccess){
  std::vector<uint8_t> response(10);
  response[0] = 0x00;
  response[1] = 0x00;
  response[2] = 0x0A;
  response[3] = 0x41; 
  response[4] = 0x01;
  response[5] = 0x00;
  response[6] = 0xB6;
  response[7] = 0x3C;
  response[8] = 0x97;
  response[9] = 0x71;;

  EXPECT_CALL(ftdi, writeData(_));
  EXPECT_CALL(ftdi, readData()).WillRepeatedly(Return(response));
  auto ret = dev.getMemoryHandle(K32W061::MemoryID::flash);
  EXPECT_NE(ret, 0);
}

TEST_F(K32W061_OpenMemory, returnsHandleFromResponse){
  std::vector<uint8_t> response(10);
  response[0] = 0x00;
  response[1] = 0x00;
  response[2] = 0x0A;
  response[3] = 0x41; 
  response[4] = 0x00;
  response[5] = 0xFF; // handle
  response[6] = 0x82;
  response[7] = 0x25;
  response[8] = 0x49;
  response[9] = 0xBD;

  EXPECT_CALL(ftdi, writeData(_));
  EXPECT_CALL(ftdi, readData()).WillRepeatedly(Return(response));
  auto ret = dev.getMemoryHandle(K32W061::MemoryID::flash);
  EXPECT_EQ(ret, 0xFF);
}

TEST_F(K32W061_EraseMemory, callsReadAfterWrite){
  testing::Sequence s;
  EXPECT_CALL(ftdi, writeData(_)).Times(1).WillOnce(Return(18));
  EXPECT_CALL(ftdi, readData()).Times(1);
  dev.eraseMemory(0);
}

TEST_F(K32W061_EraseMemory, verifyWriteFrameHeader){
  std::vector<uint8_t> req(4);
  req[0] = 0x00;
  req[1] = 0x00;
  req[2] = 0x12;
  req[3] = 0x42;
  EXPECT_CALL(ftdi, writeData(FrameHeaderEq(req))).Times(1);
  dev.eraseMemory(0);
}

TEST_F(K32W061_EraseMemory, verifyWriteFramePayload){
  std::vector<uint8_t> req(10);
  req[0] = 0x00;
  req[1] = 0x00;
  req[2] = 0x00;
  req[3] = 0x00;
  req[4] = 0x00;
  req[5] = 0x00;
  req[6] = 0x00;
  req[7] = 0xDE;
  req[8] = 0x09;
  req[9] = 0x00;
  EXPECT_CALL(ftdi, writeData(FramePayloadEq(req))).Times(1);
  dev.eraseMemory(0);
}

TEST_F(K32W061_EraseMemory, verifyWriteFrameCrc){
  std::vector<uint8_t> crc(4);
  crc[0] = 0xE9;
  crc[1] = 0x69;
  crc[2] = 0x09;
  crc[3] = 0xF9;
  EXPECT_CALL(ftdi, writeData(FrameCrcEq(crc))).Times(1);
  dev.eraseMemory(0);
}

TEST_F(K32W061_EraseMemory, failsIfNotAllBytesWritten){
  EXPECT_CALL(ftdi, writeData(_)).WillOnce(Return(0));
  auto ret = dev.eraseMemory(0);
  EXPECT_NE(ret, 0);
}

TEST_F(K32W061_EraseMemory, failsIfWriteFails){
  EXPECT_CALL(ftdi, writeData(_)).WillOnce(Return(-1));
  auto ret = dev.eraseMemory(0);
  EXPECT_NE(ret, 0);
}

TEST_F(K32W061_EraseMemory, returnsSuccessOnValidResponse){
  std::vector<uint8_t> resp(9);
  resp[0] = 0x00;
  resp[1] = 0x00;
  resp[2] = 0x09;
  resp[3] = 0x43;
  resp[4] = 0x00;
  resp[5] = 0x12;
  resp[6] = 0xA7;
  resp[7] = 0xD0;
  resp[8] = 0x54;
  EXPECT_CALL(ftdi, writeData(_)).WillOnce(Return(18));
  EXPECT_CALL(ftdi, readData()).WillOnce(Return(resp));
  auto ret = dev.eraseMemory(0);
  EXPECT_EQ(ret, 0);
}

TEST_F(K32W061_EraseMemory, failsIfResponseFrameTypeIsNot0x43){
  std::vector<uint8_t> resp(9);
  resp[0] = 0x00;
  resp[1] = 0x00;
  resp[2] = 0x09;
  resp[3] = 0x44; // shuld be 0x43
  resp[4] = 0x00;
  resp[5] = 0x5D;
  resp[6] = 0xE6;
  resp[7] = 0x46;
  resp[8] = 0x93;
  EXPECT_CALL(ftdi, writeData(_)).WillOnce(Return(18));
  EXPECT_CALL(ftdi, readData()).WillOnce(Return(resp));
  auto ret = dev.eraseMemory(0);
  EXPECT_NE(ret, 0);
}

TEST_F(K32W061_EraseMemory, failsIfResponseCrcIsWrong){
  std::vector<uint8_t> resp(9);
  resp[0] = 0x00;
  resp[1] = 0x00;
  resp[2] = 0x09;
  resp[3] = 0x43;
  resp[4] = 0x00;
  resp[5] = 0x12;
  resp[6] = 0xA7;
  resp[7] = 0xD0;
  resp[8] = 0x55; //should be 0x54

  EXPECT_CALL(ftdi, writeData(_)).WillOnce(Return(18));
  EXPECT_CALL(ftdi, readData()).WillOnce(Return(resp));
  auto ret = dev.eraseMemory(0);
  EXPECT_NE(ret, 0);
}

TEST_F(K32W061_EraseMemory, failsIfResponseStatusIsNotSuccess){
  std::vector<uint8_t> resp(9);
  resp[0] = 0x00;
  resp[1] = 0x00;
  resp[2] = 0x09;
  resp[3] = 0x43;
  resp[4] = 0x01;
  resp[5] = 0x65;
  resp[6] = 0xA0;
  resp[7] = 0xE0;
  resp[8] = 0xC2;

  EXPECT_CALL(ftdi, writeData(_)).WillOnce(Return(18));
  EXPECT_CALL(ftdi, readData()).WillOnce(Return(resp));
  auto ret = dev.eraseMemory(0);
  EXPECT_NE(ret, 0);
}

TEST_F(K32W061_EraseMemory, usesSpecifiedHandleInWriterequest){
  std::vector<uint8_t> req(10);
  req[0] = 0xFF;
  req[1] = 0x00;
  req[2] = 0x00;
  req[3] = 0x00;
  req[4] = 0x00;
  req[5] = 0x00;
  req[6] = 0x00;
  req[7] = 0xDE;
  req[8] = 0x09;
  req[9] = 0x00;

  EXPECT_CALL(ftdi, writeData(FramePayloadEq(req))).WillOnce(Return(18));
  EXPECT_CALL(ftdi, readData());
  auto ret = dev.eraseMemory(0xFF);
  EXPECT_NE(ret, 0);
}

TEST_F(K32W061_MemoryIsErased, callsReadAfterWrite){
  testing::Sequence s;
  EXPECT_CALL(ftdi, writeData(_)).Times(1).WillOnce(Return(18));
  EXPECT_CALL(ftdi, readData()).Times(1);
  dev.memoryIsErased(0);
}

TEST_F(K32W061_MemoryIsErased, verifyWriteFrameHeader){
  std::vector<uint8_t> req{0x00, 0x00, 0x12, 0x44};
  EXPECT_CALL(ftdi, writeData(FrameHeaderEq(req))).Times(1);
  dev.memoryIsErased(0);
}

TEST_F(K32W061_MemoryIsErased, verifyWritePayload){
  std::vector<uint8_t> payload{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xDE, 0x09, 0x00};
  EXPECT_CALL(ftdi, writeData(FramePayloadEq(payload))).Times(1);
  dev.memoryIsErased(0);
}

TEST_F(K32W061_MemoryIsErased, verifyWriteChecksum){
  std::vector<uint8_t> crc{0x01, 0xDC, 0xC3, 0xBA};
  EXPECT_CALL(ftdi, writeData(FrameCrcEq(crc))).Times(1);
  dev.memoryIsErased(0);
}

TEST_F(K32W061_MemoryIsErased, failsIfNotAllBytesWritten){
  EXPECT_CALL(ftdi, writeData(_)).WillOnce(Return(0));
  auto ret = dev.memoryIsErased(0);
  EXPECT_NE(ret, true);
}

TEST_F(K32W061_MemoryIsErased, failsIfWriteFails){
  EXPECT_CALL(ftdi, writeData(_)).WillOnce(Return(-1));
  auto ret = dev.memoryIsErased(0);
  EXPECT_NE(ret, true);
}

TEST_F(K32W061_MemoryIsErased, returnsSuccessOnValidResponse){
  std::vector<uint8_t> resp{0x00, 0x00, 0x09, 0x45, 0x00, 0x44, 0xFD, 0x77, 0xD2};
  EXPECT_CALL(ftdi, writeData(_)).WillOnce(Return(18));
  EXPECT_CALL(ftdi, readData()).WillOnce(Return(resp));
  auto ret = dev.memoryIsErased(0);
  EXPECT_EQ(ret, true);
}

TEST_F(K32W061_MemoryIsErased, failsIfResponseFrameTypeIsNot0x45){
  std::vector<uint8_t> resp{0x00, 0x00, 0x09, 0x46, 0x00, 0x44, 0xFD, 0x77, 0xD2};
  EXPECT_CALL(ftdi, writeData(_)).WillOnce(Return(18));
  EXPECT_CALL(ftdi, readData()).WillOnce(Return(resp));
  auto ret = dev.memoryIsErased(0);
  EXPECT_NE(ret, true);
}

TEST_F(K32W061_MemoryIsErased, failsIfResponseCrcIsWrong){
  std::vector<uint8_t> resp{0x00, 0x00, 0x09, 0x45, 0x00, 0x44, 0xFD, 0x77, 0xD3};

  EXPECT_CALL(ftdi, writeData(_)).WillOnce(Return(18));
  EXPECT_CALL(ftdi, readData()).WillOnce(Return(resp));
  auto ret = dev.memoryIsErased(0);
  EXPECT_NE(ret, true);
}

TEST_F(K32W061_MemoryIsErased, failsIfResponseStatusIsNotSuccess){
  std::vector<uint8_t> resp{0x00, 0x00, 0x09, 0x45, 0xF9, 0x80, 0x9C, 0x3D, 0x6A };

  EXPECT_CALL(ftdi, writeData(_)).WillOnce(Return(18));
  EXPECT_CALL(ftdi, readData()).WillOnce(Return(resp));
  auto ret = dev.memoryIsErased(0);
  EXPECT_NE(ret, true);
}

TEST_F(K32W061_MemoryIsErased, usesSpecifiedHandleInWriterequest){
  std::vector<uint8_t> payload{0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xDE, 0x09, 0x00};

  EXPECT_CALL(ftdi, writeData(FramePayloadEq(payload))).WillOnce(Return(18));
  EXPECT_CALL(ftdi, readData());
  dev.memoryIsErased(0xFF);
}

TEST_F(K32W061_FlashMemory, callsReadAfterWrite){
  testing::Sequence s;
  EXPECT_CALL(ftdi, writeData(_)).Times(1).WillOnce(Return(18));
  EXPECT_CALL(ftdi, readData()).Times(1);
  std::vector<uint8_t> data;
  dev.flashMemory(0, data);
}

TEST_F(K32W061_FlashMemory, verifyWriteFrameHeader){
  std::vector<uint8_t> req{0x00, 0x00, 0x30, 0x48};
  EXPECT_CALL(ftdi, writeData(FrameHeaderEq(req))).Times(1);
  
  std::vector<uint8_t> data(30);
  dev.flashMemory(0, data);
}

TEST_F(K32W061_FlashMemory, verifyWritePayload){
  std::vector<uint8_t> payload{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0A, 0x00, 0x00, 0x00, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03};

  EXPECT_CALL(ftdi, writeData(FramePayloadEq(payload))).Times(1);
  std::vector<uint8_t> data(10);
  std::for_each(data.begin(), data.end(), [](uint8_t &n){ n=3;});
  dev.flashMemory(0, data);
}

TEST_F(K32W061_FlashMemory, verifyWriteChecksum){
  std::vector<uint8_t> crc{0xE2, 0xCE, 0x2A, 0x75};

  EXPECT_CALL(ftdi, writeData(FrameCrcEq(crc))).Times(1);
  std::vector<uint8_t> data(10);
  std::for_each(data.begin(), data.end(), [](uint8_t &n){ n=3;});
  dev.flashMemory(0, data);
}

MATCHER_P(FrameMemoryAddressEq, offset, "") {
  auto frame_offset = (uint32_t*)&arg[6];
  return offset == *frame_offset;
}
MATCHER_P(FrameMemoryPayloadLengthEq, length, "") {
  auto payload_length = (uint32_t*)&arg[10];
  return length == *payload_length;
}
TEST_F(K32W061_FlashMemory, largePayloadIsSplitInto512byteChunks){
  std::vector<uint8_t> resp{0x00, 0x00, 0x09, 0x49, 0x00, 0xE8, 0x48, 0x38, 0xDE};
  EXPECT_CALL(ftdi, writeData(AllOf(FrameMemoryAddressEq(1024), FrameMemoryPayloadLengthEq(10))) ).Times(1).WillOnce(Return(28));
  EXPECT_CALL(ftdi, writeData(AllOf(FrameMemoryAddressEq(512), FrameMemoryPayloadLengthEq(512))) ).Times(1).WillOnce(Return(530));
  EXPECT_CALL(ftdi, writeData(AllOf(FrameMemoryAddressEq(0), FrameMemoryPayloadLengthEq(512))) ).Times(1).WillOnce(Return(530));
  EXPECT_CALL(ftdi, readData()).WillRepeatedly(Return(resp));
  std::vector<uint8_t> data(1034);
  for(unsigned int i=0;i<data.size();i++){
    data[i] = i;
  }
  dev.flashMemory(0, data);
}

TEST_F(K32W061_FlashMemory, failsifWriteFails){
  
  EXPECT_CALL(ftdi, writeData(_)).Times(1).WillOnce(Return(-1));
  std::vector<uint8_t> data(1034);
  auto ret = dev.flashMemory(0, data);
  EXPECT_LT(ret, 0);
}

TEST_F(K32W061_FlashMemory, failsIfReturnedCrcIsWrong){
  std::vector<uint8_t> resp{0x00, 0x00, 0x09, 0x49, 0x00, 0xE8, 0x48, 0x38, 0xDF};
  EXPECT_CALL(ftdi, writeData(_)).Times(1).WillOnce(Return(28));
  EXPECT_CALL(ftdi, readData()).Times(1).WillOnce(Return(resp));
  std::vector<uint8_t> data(10);
  auto ret = dev.flashMemory(0, data);
  EXPECT_LT(ret, 0);
}

TEST_F(K32W061_FlashMemory, failsIfResponseStatusCodeIsNotSuccess){
  std::vector<uint8_t> resp{0x00, 0x00, 0x09, 0x49, 0xF0, 0x55, 0xF5, 0xCA, 0xC2};
  EXPECT_CALL(ftdi, writeData(_)).Times(1).WillOnce(Return(28));
  EXPECT_CALL(ftdi, readData()).Times(1).WillOnce(Return(resp));
  std::vector<uint8_t> data(10);
  auto ret = dev.flashMemory(0, data);
  EXPECT_LT(ret, 0);
}

TEST_F(K32W061_FlashMemory, failsIfResponseFrameTypeIsNot0x49){
  std::vector<uint8_t> resp{0x00, 0x00, 0x09, 0x50, 0x00, 0x73, 0x48, 0x91, 0xC6};
  EXPECT_CALL(ftdi, writeData(_)).Times(1).WillOnce(Return(28));
  EXPECT_CALL(ftdi, readData()).Times(1).WillOnce(Return(resp));
  std::vector<uint8_t> data(10);
  auto ret = dev.flashMemory(0, data);
  EXPECT_LT(ret, 0);
}

TEST_F(K32W061_GetDeviceInfo, callsReadAfterWrite){
  testing::Sequence s;
  EXPECT_CALL(ftdi, writeData(_)).Times(1).WillOnce(Return(8));
  EXPECT_CALL(ftdi, readData()).Times(1);
  dev.getDeviceInfo();
}

TEST_F(K32W061_GetDeviceInfo, verifyWriteFrameHeader){
  std::vector<uint8_t> header{0x00, 0x00, 0x08, 0x32};
  EXPECT_CALL(ftdi, writeData(FrameHeaderEq(header))).Times(1);
  dev.getDeviceInfo();
}

TEST_F(K32W061_GetDeviceInfo, verifyWriteCrc){
  std::vector<uint8_t> crc{0x21, 0x4A, 0x04, 0x94};
  EXPECT_CALL(ftdi, writeData(FrameCrcEq(crc))).Times(1);
  dev.getDeviceInfo();
}

TEST_F(K32W061_GetDeviceInfo, failsIfNotAllBytesWritten){
  EXPECT_CALL(ftdi, writeData(_)).Times(1).WillOnce(Return(7));
  auto ret = dev.getDeviceInfo();
  EXPECT_EQ(ret.chipId, 0);
  EXPECT_EQ(ret.version, 0);
}

TEST_F(K32W061_GetDeviceInfo, failsIfWriteFails){
  EXPECT_CALL(ftdi, writeData(_)).Times(1).WillOnce(Return(-1));
  auto ret = dev.getDeviceInfo();
  EXPECT_EQ(ret.chipId, 0);
  EXPECT_EQ(ret.version, 0);
}

TEST_F(K32W061_GetDeviceInfo, returnsValidInfoOnValidResponse){
  std::vector<uint8_t> resp{0x00, 0x00, 0x11, 0x33, 0x00, 0x88, 0x88, 0x88, 0x88, 0x00, 0x00, 0x00, 0x00, 0xAB, 0x9A, 0x33, 0xAE};
  EXPECT_CALL(ftdi, writeData(_)).Times(1).WillOnce(Return(8));
  EXPECT_CALL(ftdi, readData()).Times(1).WillOnce(Return(resp));
  auto ret = dev.getDeviceInfo();
  EXPECT_EQ(ret.chipId, 0x88888888);
  EXPECT_EQ(ret.version, 0);
}

TEST_F(K32W061_GetDeviceInfo, failsIfResponseFrameTypeIsNot0x33){
  std::vector<uint8_t> resp{0x00, 0x00, 0x11, 0x32, 0x00, 0x88, 0x88, 0x88, 0x88, 0x00, 0x00, 0x00, 0x00, 0x44, 0x58, 0x58, 0x90};
  EXPECT_CALL(ftdi, writeData(_)).Times(1).WillOnce(Return(8));
  EXPECT_CALL(ftdi, readData()).Times(1).WillOnce(Return(resp));
  auto ret = dev.getDeviceInfo();
  EXPECT_EQ(ret.chipId, 0x00);
  EXPECT_EQ(ret.version, 0);
}

TEST_F(K32W061_GetDeviceInfo, failsIfResponseCrcIsWrong){
  std::vector<uint8_t> resp{0x00, 0x00, 0x11, 0x33, 0x00, 0x88, 0x88, 0x88, 0x88, 0x00, 0x00, 0x00, 0x00, 0xAB, 0x9A, 0x33, 0xAF};
  EXPECT_CALL(ftdi, writeData(_)).Times(1).WillOnce(Return(8));
  EXPECT_CALL(ftdi, readData()).Times(1).WillOnce(Return(resp));
  auto ret = dev.getDeviceInfo();
  EXPECT_EQ(ret.chipId, 0x00);
  EXPECT_EQ(ret.version, 0);
}

TEST_F(K32W061_GetDeviceInfo, failIsResponseTypeIsNotSuccess){
  std::vector<uint8_t> resp{0x00, 0x00, 0x11, 0x33, 0xF0, 0x88, 0x88, 0x88, 0x88, 0x00, 0x00, 0x00, 0x00, 0x3D, 0x6C, 0xF0, 0x34};
  EXPECT_CALL(ftdi, writeData(_)).Times(1).WillOnce(Return(8));
  EXPECT_CALL(ftdi, readData()).Times(1).WillOnce(Return(resp));
  auto ret = dev.getDeviceInfo();
  EXPECT_EQ(ret.chipId, 0x00);
  EXPECT_EQ(ret.version, 0);
}

TEST_F(K32W061_CloseMemory, callsReadAfterWrite){
  testing::Sequence s;
  EXPECT_CALL(ftdi, writeData(_)).Times(1).WillOnce(Return(18));
  EXPECT_CALL(ftdi, readData()).Times(1);
  dev.closeMemory(0);
}

