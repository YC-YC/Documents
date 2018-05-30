/*
 * ShmHelper.h
 *
 *  Created on: 2018-5-23
 *      Author: YC2
 */

#ifndef SHMHELPER_H_
#define SHMHELPER_H_
#include "ShmComdef.h"
#include <list>



class ShmDataCallback{
public:
	virtual void onDataChange(int index, int param) = 0;
};



class ShmHelper{
public:
	ShmHelper();
	~ShmHelper();
	void addShmDataCallback(ShmDataCallback* cb);
	void removeShmDataCallback(ShmDataCallback* cb);
	int getShmData(int index);	//获取索引值对应的值
	bool setShmData(int index, int param);	//设置索引值对应的值
	friend void* threadHandler(void* param);
private:
	void* observerThreadHandler();
	void notifyDataChange(int index, int param);
	bool getAllShmData(Data_st& data);
private:
	typedef std::list<ShmDataCallback *> CB_LIST;
	CB_LIST mCBList;
	SHAREDATA *m_pShm;
	Data_st mData;
	int mShmid;
	bool m_bCreateShm;	//是否创建了Shm
	bool m_bCreateReadThread;
	bool m_bThreadRunReq;
};




#endif /* SHMHELPER_H_ */
