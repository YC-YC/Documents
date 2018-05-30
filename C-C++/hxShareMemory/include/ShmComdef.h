/*
 * ShmComdef.h
 *
 *  Created on: 2018-5-23
 *      Author: YC2
 */

#ifndef SHM_COMDEF_H_
#define SHM_COMDEF_H_

#include <stdint.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#define MEMORY_KEY_ID	0x8187
#define PARAS_MAX_LEN	255

//Data_st 的paras索引值
enum PARAS_INX{
	CMD_UPGRADE_REQ_SUSPEND = 0,	//升级应用请求其它应用挂起UI
	CMD_UPGRADE_REQ_UART,		//升级应用请求使用串口
	CMD_PLAYER_RES_UART,		//主应用（kzbplayer）回应停止使用串口
	CMD_MAX
};

typedef struct{
	int params[PARAS_MAX_LEN];
	//int cmd;
	//char data[DATA_LEN+1];
}Data_st;

typedef struct
{
	int semId;
	Data_st shmData;

}SHAREDATA;


#endif /* SHM_COMDEF_H_ */
