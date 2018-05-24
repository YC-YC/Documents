/*
 * UartHelper.h
 *
 *  Created on: 2018-5-4
 *      Author: YC2
 */

#ifndef UARTHELPER_H_
#define UARTHELPER_H_
#include "Parser.h"
#include "ComCtrl.h"

class UartHelper:public UartDataCallback{
public:
	UartHelper(Parser* pParser);
	~UartHelper();
	void onDataReceive(const unsigned char data[], int len);
	void setCmdCallback(CmdCallback* cb);
	bool openUart(const char* fileName,int baudRate);
	bool writeUart(void const *pRam,int num);
	void closeUart();
	CComCtrl* getComCtrl(){
		return mComCtrl;
	}
private:
	CComCtrl* mComCtrl;
	Parser* mParser;

};


#endif /* UARTHELPER_H_ */
