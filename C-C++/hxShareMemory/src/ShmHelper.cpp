/*
 * ShmHelper.cpp
 *
 *  Created on: 2018-5-23
 *      Author: YC2
 */
#include "../include/ShmHelper.h"
#include "ShmFunc.h"
#include <sys/types.h>
#include <sys/ipc.h>
#include <errno.h>
#include <pthread.h>
#include <string.h>

//信号量ID
const int SEM_ID = 0x1000;

void* threadHandler(void* param){
	ShmHelper* pHelper = (ShmHelper*)param;
	return (pHelper->observerThreadHandler());
}

ShmHelper::ShmHelper(){
	LOG("version is:2018.05.30.01");
	m_pShm = NULL;
	mShmid = -1;
	m_bCreateShm = false;
	m_bCreateReadThread = false;
	m_bThreadRunReq = false;
	m_bThreadRunReq = false;
	memset(&mData, 0, sizeof(Data_st));
	if ((mShmid = getShareMemory(MEMORY_KEY_ID, sizeof(SHAREDATA))) == -1){
		mShmid = createShareMemory(MEMORY_KEY_ID, sizeof(SHAREDATA));
		if (mShmid != -1){
			LOG("create share memory service ok\n");
			m_bCreateShm = true;
		}
		else{
			LOG("create share memory service error, no=%d\n", errno);
		}
	}
	else{
		LOG("getShareMemory ok\n");
	}

	if (mShmid != -1){
		m_pShm = (SHAREDATA *)attachShareMemory(mShmid);
	}
	else{
		LOG("getShareMemory error,no =%d\n", errno);
	}

	if(m_pShm != NULL){
		if (m_bCreateShm){
			LOG("init share memory data\n");
			(*m_pShm).semId = -1;
			memset(&(*m_pShm).shmData, 0, sizeof(Data_st));
			(*m_pShm).semId = createSemaphore(SEM_ID);
			LOG("create Semaphore %d\n", (*m_pShm).semId);
			if ((*m_pShm).semId != -1){
				LOG("create Semaphore ok\n");
			}
			else{
				LOG("create Semaphore error, no=%d\n", errno);
			}
		}
	}
	else{
		LOG("attachShareMemory error,no =%d\n", errno);
	}

}
ShmHelper::~ShmHelper(){
	m_bThreadRunReq = false;
	if (m_pShm != NULL){
		if ((*m_pShm).semId >= 0){
			deleteSemaphore((*m_pShm).semId);
		}

		detachShareMemory(m_pShm);
		if (m_bCreateShm){
			deleteShareMemory((*m_pShm).semId);
		}
	}
}
void ShmHelper::addShmDataCallback(ShmDataCallback* cb){
	mCBList.push_back(cb);
	if (!m_bCreateReadThread){
		m_bCreateReadThread = true;
		m_bThreadRunReq = true;
		pthread_t threadId = 0;
		pthread_create(&threadId, NULL, threadHandler, this);
	}
}
void ShmHelper::removeShmDataCallback(ShmDataCallback* cb){
	CB_LIST::iterator it = mCBList.begin();
	while(it != mCBList.end()){
		if ((*it) == cb){
			mCBList.erase(it);
			break;
		}
		it++;
	}
	if (mCBList.empty()){
		printf("req exit thread\n");
		m_bThreadRunReq = false;
		m_bCreateReadThread = false;
	}
}
int ShmHelper::getShmData(int index){
	if (m_pShm != NULL && index >= CMD_UPGRADE_REQ_SUSPEND && index < CMD_MAX){
		if (getSemaphore((*m_pShm).semId)){
			int val = (*m_pShm).shmData.params[index];
			releaseSemaphore((*m_pShm).semId);
			return val;
		}
	}
	return -1;

}
bool ShmHelper::getAllShmData(Data_st& data){
	if (m_pShm != NULL){
		if (getSemaphore((*m_pShm).semId)){
			memcpy(&data, &(*m_pShm).shmData, sizeof(Data_st));
			releaseSemaphore((*m_pShm).semId);
			return true;
		}
	}
	return false;

}
bool ShmHelper::setShmData(int index, int param){
	if (m_pShm != NULL){
		if (getSemaphore((*m_pShm).semId)){
			//memcpy(&(*m_pShm).shmData, &data, sizeof(Data_st));
			(*m_pShm).shmData.params[index] = param;
			//LOG("setShmData cmd = %d, data=%s\n", (*m_pShm).shmData.cmd, (*m_pShm).shmData.data);
			releaseSemaphore((*m_pShm).semId);
			return true;
		}
	}
	else{
		LOG("pShm is null\n");
	}
	return false;
}

void ShmHelper::notifyDataChange(int index, int param){
	CB_LIST::iterator it = mCBList.begin();
	while(it != mCBList.end()){
		(*it)->onDataChange(index, param);
		it++;
	}
}

void* ShmHelper::observerThreadHandler(){
	LOG("start observerThreadHandler\n");

	while(m_bThreadRunReq){
		
		Data_st data;
		memset(&data, 0, sizeof(Data_st));
		getAllShmData(data);
		//LOG("observerThreadHandler run...cmd = %d, data=%s\n", data.cmd, data.data);
		for (int i = CMD_UPGRADE_REQ_SUSPEND; i < CMD_MAX; i++){
			if (data.params[i] != mData.params[i]){
				mData.params[i] = data.params[i];
				notifyDataChange(i, mData.params[i]);
			}	
		}
		usleep(50*1000);
	}
	LOG("observerThreadHandler exit\n");
	return NULL;
}



