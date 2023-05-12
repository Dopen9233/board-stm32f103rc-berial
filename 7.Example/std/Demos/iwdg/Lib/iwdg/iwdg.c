#include "iwdg.h"

// IWDG ������ 40KHz LSI ��ʱ����, ��û�и�λԤ���ж�

/**
 * @param [in] prescaler ��Ƶ�� 0~7 (3bit), ��Ƶ���� = 4*2^prescaler (���ֵֻ����256)
 * @param [in] reload ��װ��ֵ 11-bit
 *
 * @note ���ʱ�� Tout=((4*2^prescaler)*reload)/40 (ms).
 */
void iwdg_init(uint8_t prescaler, uint16_t reload)
{
    IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);
    IWDG_SetPrescaler(prescaler);  // ��Ԥ��Ƶֵ
    IWDG_SetReload(reload);        // ����װ��ֵ
    IWDG_ReloadCounter();
    IWDG_Enable();
}

void iwdg_feed(void)
{
    IWDG_ReloadCounter();
}
