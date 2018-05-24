/*
 * UartHelper.cpp
 *
 *  Created on: 2018-5-4
 *      Author: YC2
 */

#include "../include/UartHelper.h"
#include "ComFunc.h"

#define TAG "UartHelper"

UartHelper::UartHelper(Parser* pParser){
	//mParser = new McuParser();
	LOGT(TAG,"uart version:2018.05.24");
	mParser = pParser;
	mComCtrl = new CComCtrl();
	mComCtrl->setUartDataCallback(this);
}
UartHelper::~UartHelper(){
	delete mComCtrl;
	mComCtrl = NULL;
	delete mParser;
	mParser = NULL;
}
void UartHelper::onDataReceive(const unsigned char data[], int len){
	//mComCtrl->printfBuffHex(data, len);
	mParser->inputData(data, len);
}
void UartHelper::setCmdCallback(CmdCallback* cb){
	mParser->setCmdCallback(cb);
}

bool UartHelper::openUart(const char* fileName,int baudRate){
	//return mComCtrl->openUart("/dev/ttymxc1", 115200);
	return mComCtrl->openUart(fileName, baudRate);
}

bool UartHelper::writeUart(void const *pRam,int num){
	if (mComCtrl->isUartOpen()){
		return mComCtrl->writeUart(pRam, num);
	}
	return false;
}

void UartHelper::closeUart(){
	mComCtrl->closeUart();
}

