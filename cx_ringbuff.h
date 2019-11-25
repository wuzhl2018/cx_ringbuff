#ifndef __CX_RINGBUFF_H
#define __CX_RINGBUFF_H
 
 
#include "stdlib.h"
#include "string.h"
 
 
#define         MaxBuffSize        256        //缓冲区大小可任意设置，但最好为2^X
 
 
typedef enum
{
	Success = 1,
	Fail = !Success,
}ReadRetState_Enum;
 
 
typedef struct
{
	ReadRetState_Enum Readstate;                //读状态
	unsigned char* ptr;                         //读取字节地址
	unsigned char  pos;                         //期望帧在注册表中位置
}ReadRetState_Typedef;
 
 
 
typedef struct
{
	unsigned char* Writepos;                    //写入地址
	unsigned char* Readpos;                     //读取地址
	unsigned char RingbufCount;                 //有效未读数据大小
	ReadRetState_Typedef ReadRetStateStruct;    //读缓冲相关
	unsigned char RxBuff[MaxBuffSize];          //数据缓存区
}RingBuff_Typedef;
 
 
void RingBuff_New(RingBuff_Typedef* rb_ptr);
unsigned char* NextDataAddrHandle(RingBuff_Typedef* rb_ptr,unsigned char* addr);
ReadRetState_Typedef* ReadDataFromRingbuff(RingBuff_Typedef* rb_ptr);
void WriteDataToRingbuff(RingBuff_Typedef* rb_ptr,unsigned char data);
ReadRetState_Typedef* ReadEfectiveFrameFixLength(RingBuff_Typedef* rb_ptr, unsigned char head, unsigned char length);
ReadRetState_Enum ReadEfectiveFrame(RingBuff_Typedef* rb_ptr, const char* str);
ReadRetState_Typedef* MatchExpectFrame(RingBuff_Typedef* rb_ptr, const char** str, unsigned char ExpectFrameCount);
 
 
#endif