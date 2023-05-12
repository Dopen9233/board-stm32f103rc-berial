#include "board.h"

#define TAG               "main"
#define DBG_LVL           SDK_DEBUG

#define ONCHIP_FLASH_SIZE 256         // ������С (��λΪKB)
#define ONCHIP_FLASH_BASE 0x08000000  // ��ʼ��ַ

#if ONCHIP_FLASH_SIZE < 256
#define STM32_SECTOR_SIZE 1024  // �ֽ�
#else
#define STM32_SECTOR_SIZE 2048
#endif

uint16_t OnChipFlash_BUF[STM32_SECTOR_SIZE / 2];  // �����2K�ֽ�

vu16 OnChipFlash_ReadHalfWord(uint32_t faddr);
void OnChipFlash_Write(uint32_t WriteAddr, uint16_t* pBuffer, uint16_t NumToWrite);
void OnChipFlash_Read(uint32_t ReadAddr, uint16_t* pBuffer, uint16_t NumToRead);

vu16 OnChipFlash_ReadHalfWord(uint32_t faddr) { return *(vu16*)faddr; }

void OnChipFlash_Write_NoCheck(uint32_t WriteAddr, uint16_t* pBuffer, uint16_t NumToWrite)
{
    uint16_t i;
    for (i = 0; i < NumToWrite; i++) {
        FLASH_ProgramHalfWord(WriteAddr, pBuffer[i]);
        WriteAddr += 2;
    }
}

void OnChipFlash_Write(uint32_t WriteAddr, uint16_t* pBuffer, uint16_t NumToWrite)
{
    uint32_t secpos;     // ������ַ
    uint16_t secoff;     // ������ƫ�Ƶ�ַ(16λ�ּ���)
    uint16_t secremain;  // ������ʣ���ַ(16λ�ּ���)
    uint16_t i;
    uint32_t offaddr;                                                                                            // ȥ��0X08000000��ĵ�ַ
    if (WriteAddr < ONCHIP_FLASH_BASE || (WriteAddr >= (ONCHIP_FLASH_BASE + 1024 * ONCHIP_FLASH_SIZE))) return;  // �Ƿ���ַ
    FLASH_Unlock();                                                                                              // ����
    offaddr   = WriteAddr - ONCHIP_FLASH_BASE;                                                                   // ʵ��ƫ�Ƶ�ַ.
    secpos    = offaddr / STM32_SECTOR_SIZE;                                                                     // ������ַ  0~127 for STM32F103RBT6
    secoff    = (offaddr % STM32_SECTOR_SIZE) / 2;                                                               // �������ڵ�ƫ��(2���ֽ�Ϊ������λ.)
    secremain = STM32_SECTOR_SIZE / 2 - secoff;                                                                  // ����ʣ��ռ��С
    if (NumToWrite <= secremain) secremain = NumToWrite;                                                         // �����ڸ�������Χ
    while (1) {
        OnChipFlash_Read(secpos * STM32_SECTOR_SIZE + ONCHIP_FLASH_BASE, OnChipFlash_BUF, STM32_SECTOR_SIZE / 2);  // ������������������
        for (i = 0; i < secremain; i++)                                                                            // У������
        {
            if (OnChipFlash_BUF[secoff + i] != 0XFFFF)
                break;  // ��Ҫ����
        }
        if (i < secremain)  // ��Ҫ����
        {
            FLASH_ErasePage(secpos * STM32_SECTOR_SIZE + ONCHIP_FLASH_BASE);  // �����������
            for (i = 0; i < secremain; i++)                                   // ����
            {
                OnChipFlash_BUF[i + secoff] = pBuffer[i];
            }
            OnChipFlash_Write_NoCheck(secpos * STM32_SECTOR_SIZE + ONCHIP_FLASH_BASE, OnChipFlash_BUF, STM32_SECTOR_SIZE / 2);  // д����������
        } else
            OnChipFlash_Write_NoCheck(WriteAddr, pBuffer, secremain);  // д�Ѿ������˵�,ֱ��д������ʣ������.
        if (NumToWrite == secremain)
            break;  // д�������
        else        // д��δ����
        {
            secpos++;                 // ������ַ��1
            secoff = 0;               // ƫ��λ��Ϊ0
            pBuffer += secremain;     // ָ��ƫ��
            WriteAddr += secremain;   // д��ַƫ��
            NumToWrite -= secremain;  // �ֽ�(16λ)���ݼ�
            if (NumToWrite > (STM32_SECTOR_SIZE / 2))
                secremain = STM32_SECTOR_SIZE / 2;  // ��һ����������д����
            else
                secremain = NumToWrite;  // ��һ����������д����
        }
    }
    FLASH_Lock();  // ����
}

void OnChipFlash_Read(uint32_t ReadAddr, uint16_t* pBuffer, uint16_t NumToRead)
{
    uint16_t i;
    for (i = 0; i < NumToRead; i++) {
        pBuffer[i] = OnChipFlash_ReadHalfWord(ReadAddr);
        ReadAddr += 2;
    }
}

/////////////////

typedef void (*iapfun)(void);

// FLASH �ռ����:
// IAP: 0X08000000~0X0800FFFF
// APP: 0x08010000~
#define FLASH_APP1_ADDR 0x08010000

void iap_load_app(uint32_t appxaddr);                                        // ִ��flash/RAM�����app����
void iap_write_appbin(uint32_t appxaddr, uint8_t* appbuf, uint32_t applen);  // ��ָ����ַ��ʼ,д��bin

iapfun   jump2app;
uint16_t iapbuf[1024];

// appxaddr:Ӧ�ó������ʼ��ַ
// appbuf:Ӧ�ó���CODE.
// appsize:Ӧ�ó����С(�ֽ�).
void iap_write_appbin(uint32_t addr, uint8_t* buf, uint32_t size)
{
    uint16_t t;
    uint16_t i = 0;
    uint16_t temp;
    uint32_t fwaddr = addr;  // ��ǰд��ĵ�ַ
    uint8_t* dfu    = buf;
    for (t = 0; t < size; t += 2) {
        temp = (dfu[1] << 8) | dfu[0];
        dfu += 2;
        iapbuf[i++] = temp;
        if (i == 1024) {
            i = 0;
            OnChipFlash_Write(fwaddr, iapbuf, 1024);
            fwaddr += 2048;
        }
    }
    if (i) OnChipFlash_Write(fwaddr, iapbuf, i);  // remain
}

// clang-format off

// ��ջ����ַ ( r0 = addr )
__asm void MSR_MSP(uint32_t addr) 
{
    MSR MSP, r0 // set Main Stack value
    BX r14
}

// clang-format on

// ��ת��Ӧ�ó����
// appxaddr:�û�������ʼ��ַ.
void iap_load_app(uint32_t appxaddr)
{
    if (((*(vu32*)appxaddr) & 0x2FFE0000) == 0x20000000)  // ���ջ����ַ�Ƿ�Ϸ�.
    {
        jump2app = (iapfun) * (vu32*)(appxaddr + 4);  // �û��������ڶ�����Ϊ����ʼ��ַ(��λ��ַ)
        MSR_MSP(*(vu32*)appxaddr);                    // ��ʼ��APP��ջָ��(�û��������ĵ�һ�������ڴ��ջ����ַ)
        jump2app();                                   // ��ת��APP.
    }
}

void DisplayMenu(void)
{
    println("------- IAP -------");
    println("1: Burn FLASH APP");
    println("2: Run FLASH APP");
    println("3: Erase SRAM APP");
    println("4: Run SRAM APP");
}

#define SRAM_APP_ADDR 0X20001000
#define USART_REC_LEN (40 * 1024)

#define RXSTA_RXOPS   0
#define RXSTA_RXBIN   1

uint8_t  USART_RX_BUF[USART_REC_LEN] __attribute__((at(SRAM_APP_ADDR)));
uint16_t USART_RX_STA = 0;
uint16_t USART_RX_CNT = 0;

void USART1_Init(void)
{
    {
        GPIO_InitTypeDef GPIO_InitStructure;

        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);

        GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_9;  // TX
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;
        GPIO_Init(GPIOA, &GPIO_InitStructure);

        GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_10;  // RX
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
        GPIO_Init(GPIOA, &GPIO_InitStructure);
    }
    {
        USART_InitTypeDef USART_InitStructure;

        RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);

        USART_InitStructure.USART_BaudRate            = 115200;
        USART_InitStructure.USART_WordLength          = USART_WordLength_8b;
        USART_InitStructure.USART_StopBits            = USART_StopBits_1;
        USART_InitStructure.USART_Parity              = USART_Parity_No;
        USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
        USART_InitStructure.USART_Mode                = USART_Mode_Rx | USART_Mode_Tx;

        USART_Init(USART1, &USART_InitStructure);
        USART_Cmd(USART1, ENABLE);

        USART_ClearFlag(USART1, USART_FLAG_TC);
        USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
    }
    {
        NVIC_InitTypeDef NVIC_InitStructure;

        NVIC_InitStructure.NVIC_IRQChannel                   = USART1_IRQn;
        NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
        NVIC_InitStructure.NVIC_IRQChannelSubPriority        = 3;
        NVIC_InitStructure.NVIC_IRQChannelCmd                = ENABLE;

        NVIC_Init(&NVIC_InitStructure);
    }
}

void USART1_IRQHandler(void)
{
    uint8_t ch;

    if (USART_GetITStatus(USART1, USART_IT_RXNE)) {
        ch = USART_RX_BUF[USART_RX_CNT++] = USART_ReceiveData(USART1);

        switch (USART_RX_STA) {
            case RXSTA_RXOPS: break;
            default: break;
        }

        if (USART_RX_CNT < USART_REC_LEN) {
            USART_RX_BUF[USART_RX_CNT++] =
        } else {
            USART_RX_CNT = 0;
        }
    }

    USART_ClearFlag(USART1, USART_FLAG_TC);
}

#define IS_FLASH_APP(addr) (0x08000000 == ((*(vu32*)(addr + 4)) & 0xFF000000))
#define IS_SRAM_APP(addr)  (0x20000000 == ((*(vu32*)(addr + 4)) & 0xFF000000))

int main(void)
{
    uint16_t app_size = 0;

    board_init();

    USART1_Init();

    DisplayMenu();

    while (1) {
        if (USART_RX_CNT) {
            if (app_size == USART_RX_CNT) {
                USART_RX_CNT = 0;
                println("app size: %d bytes", app_size);
                if (IS_FLASH_APP(SRAM_APP_ADDR)) {
                    iap_write_appbin(FLASH_APP1_ADDR, USART_RX_BUF, app_size);
                    println("update success");
                } else {
                    println("illegal flash app");  // �� FLASH Ӧ�ó���
                }
            } else {
                app_size = USART_RX_CNT;
            }
        }

        delay_ms(10);

        if (uart2_rxflg) {
            uart2_rxflg = 0;
            switch (uart2_rxbuf[0]) {
                case '2': {
                    if (IS_FLASH_APP(FLASH_APP1_ADDR)) {
                        LOGI("��ʼִ��FLASH�û�����");
                        iap_load_app(FLASH_APP1_ADDR);  // ִ��FLASH APP����
                    } else {
                        LOGI("Illegal FLASH APP");  // ��FLASHӦ�ó���,�޷�ִ��
                    }
                    break;
                }
                case '4': {
                    if (IS_SRAM_APP(SRAM_APP_ADDR)) {
                        LOGI("��ʼִ��SRAM�û�����");
                        iap_load_app(SRAM_APP_ADDR);  // SRAM��ַ
                    } else {
                        LOGI("Illegal SRAM APP");  // ��SRAMӦ�ó���,�޷�ִ��
                    }

                    break;
                }
            }
        }
    }
}
