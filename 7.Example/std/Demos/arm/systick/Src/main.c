#include "stm32f10x.h" 

int main(void)
{
	// �������ļ����ѽ�ϵͳʱ������Ϊ72M��
	
	while(1)
	{
		LED(OFF);
		SysTick_Delay_ms(500);
		LED(ON);
		SysTick_Delay_ms(500);
	}
}





