/*
 * Parser.cpp
 *
 *  Created on: 2018-5-4
 *      Author: YC2
 */
#include "../include/Parser.h"
#include "string.h"
#include "../include/ComCtrl.h"
#include "ComFunc.h"

#define TAG "Parser"

//获取byte的bits数
unsigned char getByteByBits(const unsigned char data, unsigned char offset, unsigned char len){
	
	if (offset > 7 || len > 8 || len == 0 || (offset+ len) > 8) {
		return 0;
	}
	unsigned short bb = (unsigned short)(data & 0xff);
	return ((bb >> offset) & (2 << (len -1)) -1) ;
}
unsigned char McuParser::getCheckSum(const unsigned char* p_src, int len){
	unsigned char sum = 0;
	for (int i = 0; i < len; i++){
		sum += *(p_src + i);
	} 
	return sum;
}


#define MCU_SEN_CMD_MAX_LEN		(100)	//命令是大长度
Send_st McuParser::getSendData(unsigned char cmd, unsigned char* pData, int len){
	static Send_st send;
	if (len > MCU_SEN_CMD_MAX_LEN-9){
		LOGT(TAG, "too large to send");
		len = MCU_SEN_CMD_MAX_LEN-9;
	}
	send.len = len + 9;
	static unsigned char sendbuff[MCU_SEN_CMD_MAX_LEN] = {0};
	memset(sendbuff,0, MCU_SEN_CMD_MAX_LEN);

	sendbuff[0] = 0xFF;
	sendbuff[1] = 0xAA;
	sendbuff[2] = cmd;
	sendbuff[5] = (~( ((send.len>>8)&0x0F) + ((send.len>>4)&0x0F) + (send.len&0x0F)) + 1)<<4 | ((send.len>>8)&0x0F);
	sendbuff[6] = send.len&0xFF;
	memcpy(sendbuff + 7, pData, len);
	sendbuff[7+len] = getCheckSum(&sendbuff[2], send.len - 4);
	sendbuff[send.len - 1] = 0x0A;
	send.pData = sendbuff;
	return send;
}


#define MCU_PARSER_BUFF_SIZE		(1024*10)
#define MCU_PARSER_CMD_MAX_LEN		(100)	//命令是大长度
void McuParser::inputData(const unsigned char* pData, int len){
	static unsigned char rxBuf[MCU_PARSER_BUFF_SIZE + 1] = { 0 }; //预存数据
	static int rxLen = 0; //未处理数据长度

	if (len < 0){
		return;
	}

//	RETAILMSG(UART_DEBUG, (TEXT("DVR Receive Data:len=[%d]\r\n"), len));
	//拼接数据
	if (rxLen + len > MCU_PARSER_BUFF_SIZE){
		memcpy(rxBuf + rxLen, pData, MCU_PARSER_BUFF_SIZE - rxLen);
		rxLen = MCU_PARSER_BUFF_SIZE;
	}
	else{
		memcpy(rxBuf + rxLen, pData, len);
		rxLen = rxLen + len;
	}
//		RETAILMSG(UART_DEBUG, (TEXT("DVR Receive rxBuf:len=[%d]\r\n"), rxLen));
//		PrintfBuffHex(rxBuf, rxLen);
	if (rxLen < 9) //至少6个才处理（DVR至少6个字节）
	{
		return;
	}
	while (1)
	{
		int Index = findInvalidIndex(rxBuf, rxLen); //查找无效的索引
		if (Index > 0){
			LOGT(TAG, "invalid index = %d, len = %d", Index, rxLen);
			rxLen -= (Index + 1);
			memcpy(rxBuf, &rxBuf[Index], rxLen); //将前面已经无效的内容覆盖掉
		}
		//LOGT(TAG, "inputData index = %d, len = %d", Index, rxLen);
		if (!isValidMsg(rxBuf, rxLen)){ //不是有效的消息长度
			break;
		}
		else{
			static unsigned char sCmdData[MCU_PARSER_CMD_MAX_LEN] = { 0 }; //只存DVR接收数据的data
			int cmdLen = getLenId(rxBuf, rxLen);
			memset(sCmdData,0, MCU_PARSER_CMD_MAX_LEN);
			memcpy(&sCmdData, rxBuf, cmdLen); //取出一条信息

			rxLen -= cmdLen;
			memcpy(rxBuf, &rxBuf[cmdLen], rxLen); //将一条信息删除

			LOGT(TAG, "notifyCmd len = %d", cmdLen);
			printfBuffHex(sCmdData, cmdLen);
			
			notifyCmd(sCmdData, cmdLen);
		}
	}
	
}

int McuParser::findInvalidIndex(const unsigned char* pBuff, int len){

	int index = 0;
	for (index = 0; index < len; index++) //查找0xFF
	{
		if (pBuff[index] == 0xFF)
		{
			if (index == len - 1){ //0xFF是最后一个
				break;
			}
			else{
				if (pBuff[index + 1] == 0xAA){ //0xFF后一个是0xAA
					int lenId = 0;
					if (index + 6 < len && (lenId = getLenId((const unsigned char*)(pBuff+index), (len - index))) > 500){ //长度大于500当作无效指令
						LOGT(TAG, "to long len, invalid,len = %d", lenId);
						continue;
					}
					else{
						return index;
					}
				}
				else{
					continue;
				}
			}
		}
	}
	if (index > 0){
		index--;
	}
	return index;
}

bool McuParser::isValidMsg(const unsigned char* pBuff, int len){
	if (len < 9){
		return false;
	}
	
	unsigned char lenChkSum = getByteByBits(pBuff[5], 4, 4);
	unsigned char chkSum = getLenChkSun(pBuff[5], pBuff[6]);
	//LOGT(TAG, "lenChkSum = %d, chkSum = %d", lenChkSum, chkSum);
	if (lenChkSum == getLenChkSun(pBuff[5], pBuff[6])){
		int lenId = getLenId(pBuff, len);
		if (lenId <= len){
			return true;
		}
	}
	return false;
}



unsigned char McuParser::getLenChkSun(unsigned char x, unsigned char y){
		return (((~(((y>>0)&0x0F)+((y>>4)&0x0F)+((x>>8)&0x0F)))+1)&0x0F);
	}

int McuParser::getLenId(const unsigned char* pBuff, int len){
	if (len >= 7){
		return ((getByteByBits(pBuff[5], 0, 4)<<8) | getByteByBits(pBuff[6], 0, 8));
	}
	return 0;
}


#define UPGRADE_PARSER_BUFF_SIZE		(256)
#define UPGRADE_PARSER_CMD_MAX_LEN		(6)	//命令是大长度
void UpgradeParser::inputData(const unsigned char* pData, int len){
	LOGT(TAG, "inputData len = %d", len);
	printfBuffHex(pData, len);

	static unsigned char rxBuf[UPGRADE_PARSER_BUFF_SIZE + 1] = { 0 }; //预存数据
	static int rxLen = 0; //未处理数据长度

	if (len < 0){
		return;
	}

	//printfBuffHex(rxBuf, rxLen);
	//拼接数据
	if (rxLen + len > UPGRADE_PARSER_BUFF_SIZE){
		memcpy(rxBuf + rxLen, pData, UPGRADE_PARSER_BUFF_SIZE - rxLen);
		rxLen = UPGRADE_PARSER_BUFF_SIZE;
	}
	else{
		memcpy(rxBuf + rxLen, pData, len);
		rxLen = rxLen + len;
	}
	if (rxLen < 6) //至少3个才处理
	{
		return;
	}
	while (1)
	{
		int Index = findInvalidIndex(rxBuf, rxLen); //查找无效的索引
		if (Index > 0){
			rxLen -= Index;
			memcpy(rxBuf, &rxBuf[Index], rxLen); //将前面已经无效的内容覆盖掉
		}
		//LOGT(TAG, "inputData index = %d, len = %d", Index, rxLen);
		if (!isValidMsg(rxBuf, rxLen)){ //不是有效的消息长度
			break;
		}
		else{
			static unsigned char sCmdData[UPGRADE_PARSER_CMD_MAX_LEN] = { 0 };
			int cmdLen = 6;
			memset(sCmdData,0, UPGRADE_PARSER_CMD_MAX_LEN);
			memcpy(&sCmdData, rxBuf, cmdLen); //取出一条信息

			rxLen -= cmdLen;
			memcpy(rxBuf, &rxBuf[cmdLen], rxLen); //将一条信息删除

			//LOGT(TAG, "notifyCmd len = %d", cmdLen);
			//printfBuffHex(sCmdData, cmdLen);
			notifyCmd(sCmdData, cmdLen);
		}
	}

}

int UpgradeParser::findInvalidIndex(const unsigned char* pBuff, int len){
	int index = 0;
	for (index = 0; index < len; index++){ //查找0x55
	
		if (pBuff[index] == 0x55){
			unsigned char sum = 0;
			if (index + 5 < len && (sum = getCheckSum((const unsigned char*) (pBuff+index), 5)) != pBuff[index+5]){ //校验各不匹配
				LOGT(TAG, "invalid checksum %d != %d,", sum, pBuff[index+5]);
				continue;
			}
			else{
				return index;
			}
		}
	}
	if (index == len){
		index--;
	}
	return index;
}
bool UpgradeParser::isValidMsg(const unsigned char* pBuff, int len){
	return (len >= 6);
}

unsigned char UpgradeParser::getCheckSum(const unsigned char* pBuff, int len){
	unsigned char sum = 0;
	for(int i = 0; i < len; i++){
		sum += pBuff[i];
	}
	return sum;
}



#define TEST_PARSER_BUFF_SIZE		(1024)
#define TEST_PARSER_CMD_MAX_LEN		(10)	//命令是大长度
void TestParser::inputData(const unsigned char* pData, int len){
	static unsigned char rxBuf[TEST_PARSER_BUFF_SIZE + 1] = { 0 }; //预存数据
	static int rxLen = 0; //未处理数据长度

	if (len < 0){
		return;
	}

//	RETAILMSG(UART_DEBUG, (TEXT("DVR Receive Data:len=[%d]\r\n"), len));
	//拼接数据
	if (rxLen + len > TEST_PARSER_BUFF_SIZE){
		memcpy(rxBuf + rxLen, pData, TEST_PARSER_BUFF_SIZE - rxLen);
		rxLen = TEST_PARSER_BUFF_SIZE;
	}
	else{
		memcpy(rxBuf + rxLen, pData, len);
		rxLen = rxLen + len;
	}
	if (rxLen < 3) //至少3个才处理
	{
		return;
	}
	while (1)
	{
		int Index = findInvalidIndex(rxBuf, rxLen); //查找无效的索引
		if (Index > 0){
			rxLen -= Index;
			memcpy(rxBuf, &rxBuf[Index], rxLen); //将前面已经无效的内容覆盖掉
		}
		//LOGT(TAG, "inputData index = %d, len = %d", Index, rxLen);
		if (!isValidMsg(rxBuf, rxLen)){ //不是有效的消息长度
			break;
		}
		else{
			static unsigned char sCmdData[TEST_PARSER_CMD_MAX_LEN] = { 0 };
			int cmdLen = 3;
			memset(sCmdData,0, TEST_PARSER_CMD_MAX_LEN);
			memcpy(&sCmdData, rxBuf, cmdLen); //取出一条信息

			rxLen -= cmdLen;
			memcpy(rxBuf, &rxBuf[cmdLen], rxLen); //将一条信息删除

			//LOGT(TAG, "notifyCmd len = %d", cmdLen);
			notifyCmd(sCmdData, cmdLen);
		}
	}
}

int TestParser::findInvalidIndex(const unsigned char* buff, int len){

	int index = 0;
	for (index = 0; index < len; index++){ //查找0x55
	
		if (buff[index] == 0x55){
			return index;
		}
	}
	if (index == len){
		index--;
	}
	return index;
}


bool TestParser::isValidMsg(const unsigned char* pBuff, int len){
	return (len >= 3);
}




