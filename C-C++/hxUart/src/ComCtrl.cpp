/*
 * ComCtrl.cpp
 *
 *  Created on: 2018-5-3
 *      Author: YC2
 */
#include "../include/ComCtrl.h"
#include "ComFunc.h"

#define TAG	"ComCtrl"

#define INVALID_HANDLE_VALUE	-1


void printfBuffHex(const unsigned char* buff, int len){
	if (len > 1200){
		len = 1200;
	}
	char * printfBuff = (char*)calloc(len * 3+10, sizeof(char));
	char Temp[4] = { 0 };
	for (int i = 0; i < len; i++){
		memset(Temp, 0 , 4);
		sprintf(Temp, "%02x ", buff[i]);
		strcat(printfBuff, Temp);
	}
	strcat(printfBuff, "\r\n");
	printf("len = %d:%s", len, printfBuff);
	free(printfBuff);
	printfBuff = NULL;
}


void* threadUartRx(void* param){
	CComCtrl* pUart = (CComCtrl*)param;
	return (pUart->threadReadUart());
}

CComCtrl::CComCtrl(){
	m_pDataCallback = NULL;
	mComFd = INVALID_HANDLE_VALUE;
	m_bOpen = false;
	m_bThreadRun = false;
	m_bReqExitThread = false;
}
CComCtrl::~CComCtrl(){

}

bool CComCtrl::openUart(const char* fileName,int baudRate){
	mComFd = open(fileName, O_RDWR | O_NOCTTY);
	if (mComFd == INVALID_HANDLE_VALUE){
		LOGT(TAG, "Open Uart %s FAIL,%d", fileName, mComFd);
		return false;
	}
	else{
		if (setUartOpt(mComFd, baudRate, 8, 'N', 1)){
			if (pthread_create(&pRxThread, NULL, threadUartRx, this) != 0) {
				LOGT(TAG, "Create Thread Proc Com Rx error\r\n");
				return false;
			}
			m_bOpen = true;
			return true;
		}
		return false;
	}
}
void CComCtrl::closeUart(){
	if (mComFd != INVALID_HANDLE_VALUE){
		if(m_bThreadRun){
			m_bReqExitThread = true;
			for(int i =0;i < 300;i++){
				SLEEP(10);
				if(!m_bThreadRun){
					break;
				}
			}
		}
		pRxThread = NULL;
		m_bOpen = false;
		//CloseHandle(pRxThread);
		pRxThread = NULL;
		close((int)mComFd);
		mComFd = INVALID_HANDLE_VALUE;
		LOGT(TAG, "CloseUart [OK]");
	}
}
bool CComCtrl::isUartOpen(){
	return m_bOpen;
}
bool CComCtrl::writeUart(void const *pRam,int num){
	if (!isUartOpen()){
		return false;
	}
	LOGT(TAG, "Uart write len = %d", num);
	int len = write(mComFd, pRam, num);
	return (len > 0);

}
void CComCtrl::setUartDataCallback(UartDataCallback* cb){
	m_pDataCallback = cb;
}

void* CComCtrl::threadReadUart(){
	int readNum = 0;
	static unsigned char readData[1024] = {0};
	LOGT(TAG, "[Uart Thread read ] enter while m_fd:.............%d", mComFd);
	if(mComFd != INVALID_HANDLE_VALUE){
//		fcntl((int)m_HandleUart, F_SETFL, O_SYNC); // set block mode
		//fcntl((int)mComFd, F_SETFL, O_NONBLOCK);//非阻塞
	}
	m_bThreadRun = true;
	int count = 0;
	while(mComFd != INVALID_HANDLE_VALUE && m_bOpen){
		if(m_bReqExitThread){
			LOGT(TAG, "Thread reqExit");
			break;
		}
		readNum = read((int)mComFd, readData, sizeof(readData));
		//ULOG("[Uart Thread read ] Read  NUm:%d",readNum);
		if(readNum > 0){
			if (readNum > 100){
				LOGT(TAG, "[Uart Thread read ] Read  NUm:%d",readNum);
			}
			if (m_pDataCallback != NULL){
				m_pDataCallback->onDataReceive(readData, readNum);
			}
		}
		/*
		else{
			if (count >= 100){
				count = 0;
				unsigned char buffSendData[11] = {0xFF,0xAA,0x89,0x55, 0xA2, 0x05, 0x07,0x00,0x01,0x55,0x0A};
				if (m_pDataCallback != NULL){
					m_pDataCallback->onDataReceive(buffSendData, 11);
				}
			}
			count++;
		}
		*/
		SLEEP(10);
	}
	m_bReqExitThread = false;
	m_bThreadRun = false;
	return NULL;
}



int CComCtrl::setUartOpt(int fd,int speed, int bits, char event, int stop){

	struct termios newtio,oldtio;
	if(tcgetattr( fd,&oldtio) != 0){
		perror("setUartOpt");
		LOGT(TAG, "tcgetattr( fd,&oldtio) -> %d",tcgetattr( fd,&oldtio));
		return 0;
	}

	//bzero( &newtio, sizeof( newtio ) );
	memset(&newtio, 0, sizeof( newtio ) );
	/*step1: set charactor size*/
	newtio.c_cflag  |=  CLOCAL | CREAD;
	newtio.c_cflag &= ~CSIZE;

	/*set stop bit*/
	switch(bits){
		case 7:
			newtio.c_cflag |= CS7;
			break;
		case 8:
			newtio.c_cflag |= CS8;
			break;
	 }
	/*set parity bit*/
	switch(event){
		case 'o':
		case 'O': //odd
			newtio.c_cflag |= PARENB;
			newtio.c_cflag |= PARODD;
			newtio.c_iflag |= (INPCK | ISTRIP);
			break;
		case 'e':
		case 'E': //even
			newtio.c_iflag |= (INPCK | ISTRIP);
			newtio.c_cflag |= PARENB;
			newtio.c_cflag &= ~PARODD;
			break;
		case 'n':
		case 'N':  //No parity
			newtio.c_cflag &= ~PARENB;
			break;
		default:
			break;
	 }
	 /*set baud rate*/
	switch(speed){
	  case 57600:
		  cfsetispeed(&newtio, B57600);
		  cfsetospeed(&newtio, B57600);
		  break;
	 case 2400:
		 cfsetispeed(&newtio, B2400);
		 cfsetospeed(&newtio, B2400);
		 break;
	 case 38400:
		 cfsetispeed(&newtio, B38400);
		 cfsetospeed(&newtio, B38400);
		 break;
	 case 4800:
		 cfsetispeed(&newtio, B4800);
		 cfsetospeed(&newtio, B4800);
		 break;
	  case 19200:
		  cfsetispeed(&newtio, B19200);
		  cfsetospeed(&newtio, B19200);
		  break;
	 case 9600:
		 cfsetispeed(&newtio, B9600);
		 cfsetospeed(&newtio, B9600);
		 break;
	 case 115200:
		 cfsetispeed(&newtio, B115200);
		 cfsetospeed(&newtio, B115200);
		 break;
	 case 460800:
		 cfsetispeed(&newtio, B460800);
		 cfsetospeed(&newtio, B460800);
		 break;
	 default:
		 cfsetispeed(&newtio, B9600);
		 cfsetospeed(&newtio, B9600);
		 break;
	 }

	/*set stop bit*/
	if(stop == 1){
		newtio.c_cflag &=  ~CSTOPB;
	}
	else if (stop == 2){
		newtio.c_cflag |=  CSTOPB;
	}

	/*set waiting time*/
	newtio.c_cc[VTIME]  = 0;
	newtio.c_cc[VMIN] = 0;
	tcflush(fd,TCIFLUSH);

	/*enable the config*/
	if((tcsetattr(fd,TCSANOW,&newtio))!=0) {
		perror("com set error");
		return 0;
	}

	LOGT(TAG, "set done!");
	return 1;
}
