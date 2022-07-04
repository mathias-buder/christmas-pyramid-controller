/*
 * MP3模块测试程序
 * 
 * 用途：MP3模块测试程序
 * 
 * 作者					日期				备注
 * Huafeng Lin			20010/09/10			新增
 * Huafeng Lin			20010/09/10			修改
 * 
 */

#include <reg52.h>
#include <intrins.h>

#define uchar unsigned char
#define uint unsigned int
#define ulong unsigned long
#define true 1
#define flase 0

void  InitPortVS1003(void);	
void TestVS1003B(void);

void main(void)
{
	while(1)
	{
		InitPortVS1003();
		TestVS1003B();
	}
}
