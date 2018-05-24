/*
 * ComFunc.h
 *
 *  Created on: 2018-5-23
 *      Author: YC2
 */

#ifndef COM_FUNC_H_
#define COM_FUNC_H_

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define LOGT(TAG, fmt, msg...) printf("[%s -- %s]:" fmt "\n", __TIME__, TAG, ##msg);
#define LOG( fmt, msg...) LOGT("Shm", fmt, ##msg)

#endif /* COM_FUNC_H_ */
