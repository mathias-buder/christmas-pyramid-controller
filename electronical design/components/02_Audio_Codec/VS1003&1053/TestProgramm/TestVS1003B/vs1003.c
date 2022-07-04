/*
 * MP3ģ����Գ���
 * 
 * ��;��MP3ģ����Գ���
 *       vs1003 ��Ӳ�����Գ�������оƬΪSTC12LE5A60S2
 *       ������΢������(��SPI�ӿڵ�)ֻ���Լ��޸ļ�������
 *       ���ڲ���ӲSPI�ӿڵ�΢������������IO����SPI��ʱ��ģ��
 * 
 * ����					����				��ע
 * Huafeng Lin			20010/09/10			����
 * Huafeng Lin			20010/09/10			�޸�
 * 
 */
 
#include "vs1003.h"
#include "MusicDataMP3.c"


#include <intrins.h>

#define uchar unsigned char
#define uint unsigned int
#define ulong unsigned long
#define bool bit
#define true 1
#define flase 0


//���SD����д���������
#define uint8 unsigned char

sbit  MP3_XRESET  = P3^2;
 
#define Mp3PutInReset()  { MP3_XRESET = 0; }
 
#define Mp3ReleaseFromReset()  { MP3_XRESET =1; }
 
sbit MP3_XCS = P3^3;
 
#define Mp3SelectControl()  { MP3_XCS = 0; }
 
#define Mp3DeselectControl()  { MP3_XCS = 1; }
 
sbit MP3_XDCS  = P3^4;

#define Mp3SelectData()		{ MP3_XDCS = 0; }

#define Mp3DeselectData()	{ MP3_XDCS = 1; }

sbit MP3_DREQ = P3^5;

sbit c_SPI_SI = P1^5;
sbit c_SPI_SO = P1^6;
sbit c_SPI_CLK = P1^7;

#define Macro_Set_SI_High()  	c_SPI_SI = 1
#define Macro_Set_SI_Low()  	c_SPI_SI = 0
#define Macro_Set_CLK_High()  	c_SPI_CLK = 1
#define Macro_Set_CLK_Low()  	c_SPI_CLK = 0

void LCD_write_english_string(unsigned char X,unsigned char Y,char *s);

//#define SPIWait()	{ while((S0SPSR & 0x80) == 0); }//�ȴ�SPI�����ݷ������

//#define SPI_RESULT_BYTE  S0SPDR

//extern long volatile timeval; //������ʱ��ȫ�ֱ���
//1ms Delayfunction
//void Delay(uchar ucDelayCount)
void wait(uchar ucDelayCount)
{
	uchar ucTempCount;
	uchar uci;

	for(ucTempCount=0; ucTempCount<ucDelayCount; ucTempCount++)
	{
//		uci = 200;	//Err
//		uci = 250;	//OK
		uci = 230;
		while(uci--)
		{
			_nop_();
	   	}
	}
}

//#define wait(x) Delay(x)
/**********************************************************/
/*  �������� :   MSPI_Init                                */
/*  �������� �� ��ʼ��SPI�ӿڣ�����Ϊ������               */
/*  ����     :  ��                                        */
/*  ����ֵ   :  ��                                        */
/*--------------------------------------------------------*/
void  MSPI_Init(void)
{  
/*
	PINSEL0 = (PINSEL0 & 0xFFFF00FF) | 0x00005500;	//ѡ�� SPI 
        S0SPCCR = 0x08;		                        // SPI ʱ������
 	S0SPCR  = (0 << 3) |				// CPHA = 0, 
 		  (0 << 4) |				// CPOL = 0, 
 		  (1 << 5) |				// MSTR = 1, 
 		  (0 << 6) |				// LSBF = 0, 
 	          (0 << 7);				// SPIE = 0, 
*/
	c_SPI_SO = 1;
	MP3_DREQ = 1;

}

/**********************************************************/
/*  �������� :  InitPortVS1003                            */
/*  �������� �� MCU��vs1003�ӿڵĳ�ʼ��                   */
/*  ����     :  ��                                        */
/*  ����ֵ   :  ��                                        */
/*--------------------------------------------------------*/
void  InitPortVS1003(void)
{
	MSPI_Init();//SPI�ڵĳ�ʼ��
//	IODIR &= 0xfffeffff;   //�����ӿ��ߵ����ã�����dreq Ϊ�����
//	IODIR |= MP3_XRESET | MP3_XCS | MP3_XDCS;//xRESET��xCS��xDS��Ϊ�����
//	IOSET |= MP3_XRESET | MP3_XCS | MP3_XDCS;//xRESET��xCS��xDSĬ������ߵ�ƽ	
	MP3_DREQ = 1;		//��Ϊ����

	MP3_XRESET = 1;
	MP3_XCS = 1;
	MP3_XDCS = 1;
}

//uint8 SD_SPI_ReadByte(void);
//void SD_SPI_WriteByte(uint8 ucSendData);

//#define SPI_RecByte()  SD_SPI_ReadByte()
//#define SPIPutChar(x) SD_SPI_WriteByte(x)

#if 1
/**********************************************************/
/*  �������� :  SPIPutChar                                */
/*  �������� �� ͨ��SPI����һ���ֽڵ�����                 */
/*  ����     :  �����͵��ֽ�����                          */
/*  ����ֵ   :  ��                                        */
/*--------------------------------------------------------*/
void  SPIPutChar(unsigned char ucSendData)
{      
//	S0SPDR = c;
//	while((S0SPSR & 0x80) == 0);	 //�ȴ�SPI�����ݷ������
	uchar ucCount;
	uchar ucMaskCode;

	ucMaskCode = 0x80;
	for(ucCount=0; ucCount<8; ucCount++)
	{
		Macro_Set_CLK_Low();

		if(ucMaskCode & ucSendData)
		{
			Macro_Set_SI_High();
		}
		else
		{
			Macro_Set_SI_Low();
		}

		Macro_Set_CLK_High();
		ucMaskCode >>= 1;

	}
}

/*******************************************************************************************************************
** ��������: INT8U SPI_RecByte()				Name:	  INT8U SPI_RecByte()
** ��������: ��SPI�ӿڽ���һ���ֽ�				Function: receive a byte from SPI interface
** �䡡  ��: ��									Input:	  NULL
** �� �� ��: �յ����ֽ�							Output:	  the byte that be received
********************************************************************************************************************/
static uchar SPI_RecByte(void)
{
	uchar ucReadData;
	uchar ucCount;

	ucReadData = 0;
	Macro_Set_SI_High();

	for(ucCount=0; ucCount<8; ucCount++)
	{
		ucReadData <<= 1;
			//����ʱ��Ƶ��
		Macro_Set_CLK_Low();

	
		if(c_SPI_SO)
		{
			ucReadData |= 0x01;
		}
		Macro_Set_CLK_High();

	}

	return(ucReadData);
}

#endif

/*************************************************************/
/*  �������� :  Mp3WriteRegister                             */
/*  �������� �� дvs1003�Ĵ���                               */
/*  ����     :  �Ĵ�����ַ����д���ݵĸ�8λ����д���ݵĵ�8λ */
/*  ����ֵ   :  ��                                           */
/*-----------------------------------------------------------*/
void Mp3WriteRegister(unsigned char addressbyte, unsigned char highbyte, unsigned char lowbyte)
{
	Mp3DeselectData();
	Mp3SelectControl();//XCS = 0
	SPIPutChar(VS_WRITE_COMMAND); //����д�Ĵ�������
	SPIPutChar(addressbyte);      //���ͼĴ����ĵ�ַ
	SPIPutChar(highbyte);         //���ʹ�д���ݵĸ�8λ
	SPIPutChar(lowbyte);          //���ʹ�д���ݵĵ�8λ
	Mp3DeselectControl();
}

/*************************************************************/
/*  �������� :  Mp3ReadRegister                              */
/*  �������� �� дvs1003�Ĵ���                               */
/*  ����     :  �Ĵ�����ַ				     */
/*  ����ֵ   :  vs1003��16λ�Ĵ�����ֵ                       */
/*-----------------------------------------------------------*/
unsigned int Mp3ReadRegister(unsigned char addressbyte)
{
	unsigned int resultvalue = 0;
	uchar ucReadValue;

	Mp3DeselectData();
	Mp3SelectControl();//XCS = 0
	SPIPutChar(VS_READ_COMMAND); //���Ͷ��Ĵ�������
	SPIPutChar((addressbyte));	 //���ͼĴ����ĵ�ַ

//	SPIPutChar(0xff); 		//���Ͷ�ʱ��
//	resultvalue = (SPI_RESULT_BYTE) << 8;//��ȡ��8λ����
	ucReadValue = SPI_RecByte();
	resultvalue = ucReadValue<<8;
//	SPIPutChar(0xff);		   //���Ͷ�ʱ��
//	resultvalue |= (SPI_RESULT_BYTE);  //��ȡ��8λ����
	ucReadValue = SPI_RecByte();
	resultvalue |= ucReadValue;

	Mp3DeselectControl();              
	return resultvalue;                 //����16λ�Ĵ�����ֵ
}

/**********************************************************/
/*  �������� :  Mp3SoftReset                              */
/*  �������� �� vs1003�����λ                            */
/*  ����     :  ��                                        */
/*  ����ֵ   :  ��                                        */
/*--------------------------------------------------------*/
void Mp3SoftReset(void)
{
	Mp3WriteRegister (SPI_MODE, 0x08, 0x04); //�����λ

	wait(1); //��ʱ1ms
	while (MP3_DREQ == 0); //�ȴ������λ����
	Mp3WriteRegister(SPI_CLOCKF, 0x98, 0x00);//����vs1003��ʱ��,3��Ƶ
	Mp3WriteRegister (SPI_AUDATA, 0xBB, 0x81); //������48k��������
	Mp3WriteRegister(SPI_BASS, 0x00, 0x55);//��������
	Mp3SetVolume(10,10);//��������
    wait(1); //��ʱ1ms
    	
    	//��vs1003����4���ֽ���Ч���ݣ���������SPI����
   	Mp3SelectData();
	SPIPutChar(0);
	SPIPutChar(0);
	SPIPutChar(0);
	SPIPutChar(0);
	Mp3DeselectData();

}
/**********************************************************/
/*  �������� :  Mp3Reset                                  */
/*  �������� �� vs1003Ӳ����λ                            */
/*  ����     :  ��                                        */
/*  ����ֵ   :  ��                                        */
/*--------------------------------------------------------*/
void Mp3Reset(void)
{	
	Mp3PutInReset();//xReset = 0   ��λvs1003      
	wait(200);//��ʱ100ms
	SPIPutChar(0xff);//����һ���ֽڵ���Ч���ݣ�����SPI����
	Mp3DeselectControl();   //xCS = 1
	Mp3DeselectData();     //xDCS = 1
	Mp3ReleaseFromReset(); //xRESET = 1
	wait(200);            //��ʱ100ms
	while (MP3_DREQ == 0);//�ȴ�DREQΪ��

    wait(200);            //��ʱ100ms
 	Mp3SetVolume(50,50);  
    Mp3SoftReset();//vs1003��λ
}


bool CheckVS1003B_DRQ(void)
{
	bool bResult;

	bResult =MP3_DREQ;
	return(bResult);
}

/***********************************************************/
/*  �������� :  VsSineTest                                 */
/*  �������� �� vs1003���Ҳ��ԣ����ú�������whileѭ���У�  */
/*              ����ܳ�������һ��һ�͵�������֤������ͨ�� */                            
/*  ����     :  ��                                         */
/*  ����ֵ   :  ��                                         */
/*---------------------------------------------------------*/
void VsSineTest(void)
{
	Mp3PutInReset();  //xReset = 0   ��λvs1003
	wait(200);        //��ʱ100ms        
	SPIPutChar(0xff);//����һ���ֽڵ���Ч���ݣ�����SPI����
	Mp3DeselectControl();  
	Mp3DeselectData();     
	Mp3ReleaseFromReset(); 
	wait(200);	               
	Mp3SetVolume(50,50);  

 	Mp3WriteRegister(SPI_MODE,0x08,0x20);//����vs1003�Ĳ���ģʽ
	while (MP3_DREQ == 0);     //�ȴ�DREQΪ��
 	Mp3SelectData();       //xDCS = 1��ѡ��vs1003�����ݽӿ�
 	
 	//��vs1003�������Ҳ������0x53 0xef 0x6e n 0x00 0x00 0x00 0x00
 	//����n = 0x24, �趨vs1003�����������Ҳ���Ƶ��ֵ��������㷽����vs1003��datasheet
   	SPIPutChar(0x53);      
	SPIPutChar(0xef);      
	SPIPutChar(0x6e);      
	SPIPutChar(0x24);      
	SPIPutChar(0x00);      
	SPIPutChar(0x00);
	SPIPutChar(0x00);
	SPIPutChar(0x00);
//	wait(500);
	wait(250);
	wait(250);
	Mp3DeselectData();//����ִ�е������Ӧ���ܴӶ�������һ����һƵ�ʵ�����
  
        //�˳����Ҳ���
	Mp3SelectData();
	SPIPutChar(0x45);
	SPIPutChar(0x78);
	SPIPutChar(0x69);
	SPIPutChar(0x74);
	SPIPutChar(0x00);
	SPIPutChar(0x00);
	SPIPutChar(0x00);
	SPIPutChar(0x00);
//	wait(500);
	wait(250);
	wait(250);

	Mp3DeselectData();

        //�ٴν������Ҳ��Բ�����nֵΪ0x44���������Ҳ���Ƶ������Ϊ�����ֵ
    Mp3SelectData();       
	SPIPutChar(0x53);      
	SPIPutChar(0xef);      
	SPIPutChar(0x6e);      
	SPIPutChar(0x44);      
	SPIPutChar(0x00);      
	SPIPutChar(0x00);
	SPIPutChar(0x00);
	SPIPutChar(0x00);
//	wait(500);
	wait(250);
	wait(250);

	Mp3DeselectData(); 

	//�˳����Ҳ���
	Mp3SelectData();
	SPIPutChar(0x45);
	SPIPutChar(0x78);
	SPIPutChar(0x69);
	SPIPutChar(0x74);
	SPIPutChar(0x00);
	SPIPutChar(0x00);
	SPIPutChar(0x00);
	SPIPutChar(0x00);
//	wait(500);
	wait(250);
	wait(250);

	Mp3DeselectData();
 }

void test_1003_PlayMP3File();

void TestVS1003B(void)
{
	Mp3Reset();
	VsSineTest();
	Mp3SoftReset();
	test_1003_PlayMP3File();
}

//д�Ĵ�������������ַ������
void VS1003B_WriteCMD(unsigned char addr, unsigned int dat)
{
/*
	VS1003B_XDCS_H();
	VS1003B_XCS_L();
	VS1003B_WriteByte(0x02);
	//delay_Nus(20);
	VS1003B_WriteByte(addr);
	VS1003B_WriteByte(dat>>8);
	VS1003B_WriteByte(dat);
	//delay_Nus(200);
	VS1003B_XCS_H();
*/
	Mp3WriteRegister(addr,dat>>8,dat);
}

//���Ĵ��������� ��ַ ��������
unsigned int VS1003B_ReadCMD(unsigned char addr)
{
/*
	unsigned int temp;
	unsigned char temp1;
	VS1003B_XDCS_H();
	VS1003B_XCS_L();
	VS1003B_WriteByte(0x03);
	//delay_Nus(20);
	VS1003B_WriteByte(addr);
	temp=  VS1003B_ReadByte();
	temp=temp<<8;
	temp1= VS1003B_ReadByte();
	temp=temp|temp1;;
	VS1003B_XCS_H();
	return temp;
*/
	return(Mp3ReadRegister(addr));
}

//д���ݣ���������
void VS1003B_WriteDAT(unsigned char dat)
{
//	VS1003B_XDCS_L();
//	VS1003B_WriteByte(dat);
//	VS1003B_XDCS_H();
//	VS1003B_XCS_H();

   	Mp3SelectData();
	SPIPutChar(dat);
	Mp3DeselectData();
	Mp3DeselectControl();

}

//����������
void VS1003B_SetVirtualSurroundOn(void)
{
	uchar ucRepeatCount;
	uint uiModeValue;

	ucRepeatCount =0;

	while(1)//дʱ�ӼĴ���
	{
		uiModeValue = VS1003B_ReadCMD(0x00);
		if(uiModeValue & 0x0001)
		{
			break;
		}
		else
		{
			uiModeValue |= 0x0001;
			VS1003B_WriteCMD(0,uiModeValue);
		}
		ucRepeatCount++;
		if(ucRepeatCount++ >10 )break;
	}

}

//�رջ�����
void VS1003B_SetVirtualSurroundOff(void)
{
	uchar ucRepeatCount;
	uint uiModeValue;

	ucRepeatCount =0;

	while(1)//дʱ�ӼĴ���
	{
		uiModeValue = VS1003B_ReadCMD(0x00);
		if(uiModeValue & 0x0001)
		{
			break;
		}
		else
		{
			uiModeValue |= 0x0001;
			VS1003B_WriteCMD(0,uiModeValue);
		}
		ucRepeatCount++;
		if(ucRepeatCount++ >10 )break;
	}

}

//��ǿ����
//��ڲ���	1.ǿ��0-15
//			2.Ƶ��0-15 (X10Hz)
void VS1003B_SetBassEnhance(uchar ucValue, ucFrequencyID)
{
	uchar ucRepeatCount;
	uint uiWriteValue;
	uint uiReadValue;	

	ucRepeatCount =0;

	uiWriteValue = VS1003B_ReadCMD(0x02);

	uiWriteValue &= 0xFF00;
	uiWriteValue |= ucValue<<4;
	uiWriteValue &= (ucFrequencyID & 0x0F);

	while(1)//дʱ�ӼĴ���
	{

		VS1003B_WriteCMD(2,uiWriteValue);
		uiReadValue = VS1003B_ReadCMD(0x02);
		
		if(uiReadValue == uiWriteValue)
		{
			break;
		}
		ucRepeatCount++;
		if(ucRepeatCount++ >10 )break;
	}

}


 uint uiVolumeCount;		//��ǰ����ֵ

//VS1003��ʼ����0�ɹ� 1ʧ��
unsigned char VS1003B_Init()
{
	unsigned char retry;
/*
	PORT_INI();
	DDRB|=0xa0;
	VS1003B_DDR &=~(1<<VS1003B_DREQ);
	//delay_Nus(50);
	VS1003B_XCS_H();
	VS1003B_XDCS_H();
	VS1003B_XRESET_L();
	VS1003B_Delay(0xffff);
	VS1003B_XRESET_H();//ʹ��оƬ
	VS1003B_SPI_Low();//���Ե�Ƶ����
	VS1003B_Delay(0xffff);//��ʱ
*/
Mp3Reset();

	retry=0;
	while(VS1003B_ReadCMD(0x00) != 0x0800)//дmode�Ĵ���
	{
		VS1003B_WriteCMD(0x00,0x0800);
		if(retry++ >10 )break;//{PORTB|=_BV(PB1);break;}
	}
	retry=0;
	/*while(VS1003B_ReadCMD(0x02) != 0x75)//дmode�Ĵ���
	{
		VS1003B_WriteCMD(0x02,0x75);
		if(retry++ >10 )break;//{PORTB|=_BV(PB1);break;}
	}*/
	retry=0;
	while(VS1003B_ReadCMD(0x03) != 0x9800)//дʱ�ӼĴ���
	{
		VS1003B_WriteCMD(0x03,0x9800);
		if(retry++ >10 )break;
	}
	retry=0;
//	while(VS1003B_ReadCMD(0x0b) != 0x1111)//������
//	{
//		VS1003B_WriteCMD(0x0b,0x1111);
//		if(retry++ >10 )break;
//	}
	while(VS1003B_ReadCMD(0x0b) != uiVolumeCount)//������
	{
		VS1003B_WriteCMD(0x0b,uiVolumeCount);
		if(retry++ >10 )break;
	}

//	VS1003B_SPI_High();//����ٶȣ�ȫ������
	if(retry > 10)return 1;
	return 0;
}

//VS1003�����λ
void VS1003B_SoftReset()
{
	VS1003B_WriteCMD(0x00,0x0804);//д��λ
//	VS1003B_Delay(0xffff);//��ʱ������1.35ms
	wait(2);
}

void VS1003B_Fill2048Zero()
{
	unsigned char i,j;

	for(i=0;i<64;i++)
	{
		if(CheckVS1003B_DRQ())
		{
			Mp3SelectData();

			for(j=0;j<32;j++)
			{

				VS1003B_WriteDAT(0x00);
			}
			Mp3DeselectData();
		}
	}
}


void test_1003_PlayMP3File() 
{
   unsigned int data_pointer;unsigned char i;
	unsigned int uiCount;

	uiCount = sizeof(MusicData);
	data_pointer=0; 
	VS1003B_SoftReset();
    while(uiCount>0)
  	{ 
	   if(CheckVS1003B_DRQ())
      	{
    		for(i=0;i<32;i++)
           	{
     			VS1003B_WriteDAT(MusicData[data_pointer]);
     			data_pointer++;
            }
			uiCount -= 32;
         }
    }
	VS1003B_Fill2048Zero();
}
