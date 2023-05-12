#include "MusicPlayer.h"
#include "VS1053.h"	  
#include "ff.h"   
#include "lcd.h"
#include "lcd_init.h"
//#include "bsp_usart1.h"

uint8_t 	databuf[4096]={0};	
FIL 		fmp3;
uint8_t *name;

//�������ȼ�
#define song_TASK_PRIO		2
//�����ջ��С	
#define song_STK_SIZE 		128
//������
TaskHandle_t SongTask_Handler;
//������
void song_task(void *pvParameters);

//����һ��ָ���ĸ���				     	   									    	 
void mp3_play_song(uint8_t *pname)
{	 
  name=pname;
	if(SongTask_Handler!=NULL)
	vTaskDelete(SongTask_Handler);//ɾ������
	xTaskCreate((TaskFunction_t )song_task,            //������
							(const char*    )"song_task",          //��������
							(uint16_t       )song_STK_SIZE,        //�����ջ��С
							(void*          )NULL,                  //���ݸ��������Ĳ���
							(UBaseType_t    )song_TASK_PRIO,       //�������ȼ�
							(TaskHandle_t*  )&SongTask_Handler);   //������ 
}

//task������
void song_task(void *pvParameters)
{
	uint16_t 	br;
	uint8_t 	res;	 	   
	uint16_t 	i=0;  	
	
	VS_Restart_Play();  					//�������� 
	VS_Set_All();        					//������������Ϣ 			 
	VS_Reset_DecodeTime();					//��λ����ʱ�� 	
	
//	VS_Set_Vol(254);//����Ϊ�������

	res=f_open(&fmp3,(const TCHAR*)name,FA_OPEN_EXISTING|FA_READ);//���ļ�	
	
	if(res==0)//�򿪳ɹ�.
	{ 
		VS_SPI_SpeedHigh();	//����						   
		while(1)
		{
			res=f_read(&fmp3,databuf,4096,(UINT*)&br);//����4096���ֽ�  
			i=0;
			do//������ѭ��
			{  	
				if(VS_Send_MusicData(databuf+i)==0)//��VS1053������Ƶ����
				{
					i+=32;
				}   	    
			}while(i<4096);//ѭ������4096���ֽ� 				
			if(br!=4096||res!=0)
			{
				break;//������.		  
			} 							 
		}		
	}  
	f_close(&fmp3);
	Show_Str(10,180,100,24,"���Ž���",24,0);
	vTaskDelete(SongTask_Handler);//ɾ������
}


























