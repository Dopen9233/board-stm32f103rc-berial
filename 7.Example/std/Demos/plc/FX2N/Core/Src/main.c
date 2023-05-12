
// Flash >= 256K, RAM >= 48K

#include "abs_addr.h"

extern u8 rx_end;

void NVIC_Configuration(void);

u32 startup __at(0x2000D5F0);
u8          power_down;
u8          Run_Flag = 1;

int main(void)
{
    u16 LED = 0;

    startup = 0X55AA55AA;

    io_config();  // IO��
    y_refresh();

    usart_init();  // ����

    TIM5_Init();  // ��ʱ��

    NVIC_Configuration();

    power_down = 5;
    while (1) {
        y_refresh();
        x_refresh();
        all_prog_process();

        switch (rx_end)  //  ����FX2N ����������͵�����
        {
            case 1:
                rx_end = 0,
                Process_switch(),
                TX_Process(),
                LED = 800;
                break;  // ������������ĳ���

            case 5:
                rx_end = 0,
                TX_Process(),
                LED = 800;
                break;  // ����һ�η���ָ��

            default:
                if (LED) --LED;
                break;
        }

            //	�͵�ѹ���  �ϵ籣������
#if 0
        if(!PVD)	   
        {
            if(Timer[0]==0)
            recover_data();
            if(Timer[0]<=60000)
            Timer[0]++;
        }
        else
        {
            all_data[0x180/2]=0;
            if(Timer[0]>=100)
            backup_data();
            Timer[0]=0;
        }
#endif
    }
}

#ifdef __GNUC__
int __io_putchar(int ch)
#else
int fputc(int ch, FILE* f)
#endif
{
    USART_SendData(USART2, (uint8_t)ch);
    while (USART_GetFlagStatus(USART2, USART_FLAG_TC) == RESET)
        ;
    return ch;
}

void NVIC_Configuration(void)
{
    NVIC_InitTypeDef NVIC_InitStructure;

    NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x0000);
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_3);

    NVIC_InitStructure.NVIC_IRQChannel                   = USART1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 5;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority        = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd                = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    NVIC_InitStructure.NVIC_IRQChannel                   = TIM5_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 4;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority        = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd                = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}
