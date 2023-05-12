#include "board.h"

#define TAG     "main"
#define DBG_LVL SDK_DEBUG

void EXTI0_IRQHandler(void)
{
    if (EXTI_GetITStatus(EXTI_Line0) != RESET) {
        LOGD("key press (PA0)");
        EXTI_ClearITPendingBit(EXTI_Line0);
    }
}

void PWR_ExitStopMode(void)
{
    /* After wake-up from STOP reconfigure the system clock */

    // ʹ�� HSE
    RCC_HSEConfig(RCC_HSE_ON);
    while (RCC_GetFlagStatus(RCC_FLAG_HSERDY) == RESET) {}

    // ʹ�� PLL
    RCC_PLLCmd(ENABLE);
    while (RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET) {}

    // ����ʱ��Դ
    RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);
    while (RCC_GetSYSCLKSource() != 0x08) {}
    /**
     * 0x00: HSI used as system clock
     * 0x04: HSE used as system clock
     * 0x08: PLL used as system clock
     */
}

int main(void)
{
    RCC_ClocksTypeDef clock_status_wakeup, clock_status_config;
    uint8_t           clock_source_wakeup, clock_source_config;

    board_init();

    LOGD("hello world");

    for (;;) {
        if (uart2_rxflg) {  // �յ����з���������ģʽ
            uart2_rxflg = 0;

            LOGD("enter stop mode");

            // ����ֹͣģʽ�����õ�ѹ������Ϊ�͹���ģʽ���ȴ��жϻ���(�������ж�,��֧�ִ��ڻ���)
            PWR_EnterSTOPMode(PWR_Regulator_LowPower, PWR_STOPEntry_WFI);

            // �ջ���ʱ��ʱ��ʹ�õ��� HSI
            clock_source_wakeup = RCC_GetSYSCLKSource();
            RCC_GetClocksFreq(&clock_status_wakeup);

            // �˴����������, ������ UART ��ʱ�ӱ��ر��ˣ����飩
            LOGD("[is uart ok?]");

            // �������ú��ʱ�� HSE + PLLCLK
            PWR_ExitStopMode();

            clock_source_config = RCC_GetSYSCLKSource();
            RCC_GetClocksFreq(&clock_status_config);

            LOGD("[WAKEUP]");
            LOGD("SYSCLK:%d", clock_status_wakeup.SYSCLK_Frequency);
            LOGD("HCLK:%d", clock_status_wakeup.HCLK_Frequency);
            LOGD("PCLK1:%d", clock_status_wakeup.PCLK1_Frequency);
            LOGD("PCLK2:%d", clock_status_wakeup.PCLK2_Frequency);
            LOGD("CLKSRC:%d", clock_source_wakeup);  // 0:HSI,8:PLLCLK

            LOGD("[RECONFIG]");
            LOGD("SYSCLK:%d", clock_status_config.SYSCLK_Frequency);
            LOGD("HCLK:%d", clock_status_config.HCLK_Frequency);
            LOGD("PCLK1:%d", clock_status_config.PCLK1_Frequency);
            LOGD("PCLK2:%d", clock_status_config.PCLK2_Frequency);
            LOGD("CLKSRC:%d", clock_source_config);

            LOGD("exit stop mode");
        }
    }
}
