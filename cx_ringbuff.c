/**
  ******************************************************************************
  * @file    cx_ringbuff.c
  * @author  CX
  * @version V1.0.0.2
  * @date    2016-07-13
  * @brief   1.0.0.2 期望帧逻辑优化
		     修改匹配期望帧任务的条件
		     增加匹配期望帧函数
             1.0.0.1 优化耦合性
		     优化读取逻辑
		     增加多缓冲区支持
	     1.0.0.0 主体架构搭建，完成读写环形结构化
  ******************************************************************************
  * @attention
  *
  * 项目   ：None
  * 官网   : None
  * 实验室 ：None
  *
  ******************************************************************************
  */

#include "cx_ringbuff.h"


/* 除了必须的缓冲区外没有向外部模块公开一个变量，实现高内聚，低耦合 */
RingBuff_Typedef  RingBuffStruct;


/**
* @brief   缓冲区实例化
* @param   rb_ptr 缓冲区结构体地址
* @retval  None
* @notice  None
*/
void RingBuff_New(RingBuff_Typedef* rb_ptr)
{
	rb_ptr->Readpos = rb_ptr->RxBuff;
	rb_ptr->Writepos = rb_ptr->RxBuff;
	rb_ptr->ReadRetStateStruct.Readstate = Fail;
	rb_ptr->ReadRetStateStruct.ptr = NULL;
	rb_ptr->RingbufCount = 0;
	memset(rb_ptr->RxBuff, 0, MaxBuffSize);
}


/**
  * @brief   定位函数
  * @param   rb_ptr 缓冲区结构体地址,addr 当前读写地址
  * @retval  addr+1 or 0 
  * @notice  None
  */
unsigned char* NextDataAddrHandle(RingBuff_Typedef* rb_ptr, unsigned char* addr)
{
	return (addr + 1) == (rb_ptr->RxBuff + MaxBuffSize) ? rb_ptr->RxBuff : (addr + 1);
}


/**
  * @brief   读字节函数
  * @param   rb_ptr 缓冲区结构体地址
  * @retval  data
  * @notice  None
  */
ReadRetState_Typedef* ReadDataFromRingbuff(RingBuff_Typedef* rb_ptr)
{
	if (rb_ptr->RingbufCount > 0)
	{
		rb_ptr->ReadRetStateStruct.ptr = rb_ptr->Readpos;
		rb_ptr->ReadRetStateStruct.Readstate = Success;
		rb_ptr->Readpos = NextDataAddrHandle(rb_ptr, rb_ptr->Readpos);
		rb_ptr->RingbufCount--;
		return &rb_ptr->ReadRetStateStruct;
	}
	rb_ptr->ReadRetStateStruct.Readstate = Fail;
	rb_ptr->ReadRetStateStruct.ptr = NULL;
	return &rb_ptr->ReadRetStateStruct;
}


/**
  * @brief   写字节函数
  * @param   rb_ptr 缓冲区结构体地址,data 写入数据
  * @retval  None
  * @notice  None
  */
void WriteDataToRingbuff(RingBuff_Typedef* rb_ptr, unsigned char data)
{
	*(rb_ptr->Writepos) = data;
	rb_ptr->Writepos = NextDataAddrHandle(rb_ptr,rb_ptr->Writepos);
	rb_ptr->RingbufCount++;
}


/**
  * @brief   读指定包长函数
  * @param   rb_ptr 缓冲区结构体地址,head 包头,length 包长
  * @retval  读取状态及数据包地址
  * @notice  length为整个数据包长即头置尾
*/
ReadRetState_Typedef* ReadEfectiveFrameFixLength(RingBuff_Typedef* rb_ptr, unsigned char head, unsigned char length)
{
	unsigned char count = rb_ptr->RingbufCount;              //标记触发前有效数据大小
	if (count < length)
	{
		rb_ptr->ReadRetStateStruct.Readstate = Fail;
		rb_ptr->ReadRetStateStruct.ptr = NULL;
		return &rb_ptr->ReadRetStateStruct;
	}
	while (count >= length)
	{
		rb_ptr->ReadRetStateStruct = *ReadDataFromRingbuff(rb_ptr);
		count--;
		if (rb_ptr->ReadRetStateStruct.Readstate == Success && *rb_ptr->ReadRetStateStruct.ptr == head)
		{
			return &rb_ptr->ReadRetStateStruct;
		}
	}
	rb_ptr->ReadRetStateStruct.Readstate = Fail;
	rb_ptr->ReadRetStateStruct.ptr = NULL;
	return &rb_ptr->ReadRetStateStruct;
}


/**
  * @brief   读期望帧
  * @param   rb_ptr 缓冲区结构体地址,str 期望帧
  * @retval  读取状态
  * @notice  None
*/
ReadRetState_Enum ReadEfectiveFrame(RingBuff_Typedef* rb_ptr, const char* str)
{
	unsigned char count = rb_ptr->RingbufCount;                 //标记触发前有效数据大小                
	unsigned char length = strlen(str);
	unsigned char i = 0;
	if (count < length)
	{
		return Fail;
	}
	while (count >= length)
	{
		rb_ptr->ReadRetStateStruct = *ReadDataFromRingbuff(rb_ptr);
		count--;
		if (rb_ptr->ReadRetStateStruct.Readstate == Success && *rb_ptr->ReadRetStateStruct.ptr == *(str + i))
		{
			count++;
			i++;
		}
		else
		{
			i = 0;
		}
		if (i == length)
		{
			return Success;
		}
	}
	return Fail;
}


/**
  * @brief   匹配期望帧
  * @param   rb_ptr 缓冲区结构体地址,str 期望帧所在注册表地址, ExpectFrameCount 期望帧注册表的个数
  * @retval  读取状态
  * @notice  None
*/
ReadRetState_Typedef* MatchExpectFrame(RingBuff_Typedef* rb_ptr, const char** str, unsigned char ExpectFrameCount)
{
	unsigned char Retcount = rb_ptr->RingbufCount;              
	unsigned char* RetPos = rb_ptr->Readpos;

	unsigned char i = 0;
	for (i = 0; i < ExpectFrameCount; i++)
	{
		if (ReadEfectiveFrame(rb_ptr, *(str + i)) == Success)
		{
			rb_ptr->ReadRetStateStruct.pos = i;
			rb_ptr->ReadRetStateStruct.Readstate = Success;
			return &rb_ptr->ReadRetStateStruct;
		}
		else
		{
			rb_ptr->RingbufCount = Retcount;
			rb_ptr->Readpos = RetPos;
		}
	}
	rb_ptr->ReadRetStateStruct.Readstate = Fail;
	return &rb_ptr->ReadRetStateStruct;
}

