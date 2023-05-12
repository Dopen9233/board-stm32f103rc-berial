#ifndef __qd_H
#define __qd_H
#include "sys.h"

#define LED0 PAout(0)
#define LED1 PAout(1)
#define LED2 PAout(2)

#define KEYN 1   	//����ʹ�ü�������
#define KEYCS 60   //���峤���ж�ʱ��
#define KEYSS 60   //����˫���ж�ʱ��


typedef struct {
  u8 s[KEYN+1];	//�洢����λ��
	u8 z;	//�洢����״̬
} KEY;
extern KEY key;

extern const unsigned char kl[];
extern const unsigned char pm[];
	
void LED_init(void);
void KEY_init(void);

void tset(void);
	
//void ZD_init(void);//�ⲿ�ж�
//void TIME_init(u16 per,u16 psc);
//void TIME_PWM_init(u16 per,u16 psc);

void key_scan();

void RCC_ClkConfiguration(void);//����ϵͳʱ��


#endif
