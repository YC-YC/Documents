/*
 * Parser.h
 *
 *  Created on: 2018-5-4
 *      Author: YC2
 */

#ifndef PARSER_H_
#define PARSER_H_
#include <stdio.h>

typedef struct
{
	unsigned char* pData;
	int len;
}Send_st;


class CmdCallback{
public:
	virtual void onNotify(const unsigned char* pCmd, int len) = 0;
};

class Parser{
public:
	Parser(){
		mCmdCallback = NULL;
	}
	virtual void inputData(const unsigned char* pData, int len) = 0;
	void setCmdCallback(CmdCallback* cb){
		mCmdCallback = cb;
	}
	void notifyCmd(const unsigned char* pCmd, int len){
		if (mCmdCallback != NULL){
			mCmdCallback->onNotify(pCmd, len);
		}
	}
private:
	CmdCallback* mCmdCallback;
};

/**********************************************************
* 新协议mcu协议解析
* 格式：0xFF 0xAA CID(3) LENGTH(2) INFO(n) CHKSUM(1) 0x0A
* CID: cmd version flag
* LENGTH: LCHKSUM(4) LENID(12)
* LCHKSUM:
* LENID: 整个命令的长度，从0xFF到0x0A
* CHKSUM: CID+LENGTH+INFO的长度
* 
***********************************************************/
class McuParser:public Parser{
public:
	McuParser(){};
	~McuParser(){};
	void inputData(const unsigned char* pData, int len);

public:
	static Send_st getSendData(unsigned char cmd, unsigned char* pData, int len);
private:
	static unsigned char getCheckSum(const unsigned char* p_src, int len);

private:
	int findInvalidIndex(const unsigned char* pBuff, int len); //查找无效内容的索引
	bool isValidMsg(const unsigned char* pBuff, int len); //是有效的信息
	
	unsigned char getLenChkSun(unsigned char x, unsigned char y);
	int getLenId(const unsigned char* pBuff, int len);
private:
	//const INT MCU_PARSER_BUFF_SIZE;

};

/**********************************************************
* mcu升级协议
* 格式：0x55 cmd cmd反码 index DATA checksum(共6个字节)
* checksum:前面5个字节和
* 
***********************************************************/

class UpgradeParser:public Parser{
public:
	UpgradeParser(){};
	~UpgradeParser(){};
	void inputData(const unsigned char*  pData, int len);
private:
	int findInvalidIndex(const unsigned char* pBuff, int len); //查找无效内容的索引
	bool isValidMsg(const unsigned char* pBuff, int len); //是有效的信息
	unsigned char getCheckSum(const unsigned char* pBuff, int len);
private:

};


class TestParser:public Parser{
public:
	TestParser(){};
	~TestParser(){};
	void inputData(const unsigned char* pData, int len);
private:
	int findInvalidIndex(const unsigned char* pBuff, int len); //查找无效内容的索引
	bool isValidMsg(const unsigned char* pBuff, int len); //是有效的信息
private:

};


#endif /* PARSER_H_ */
