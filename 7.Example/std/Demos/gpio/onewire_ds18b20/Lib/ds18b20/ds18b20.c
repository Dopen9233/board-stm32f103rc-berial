#include "ds18b20.h"

void ds18b20_init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;

    DS18B20_GPIO_CLK_EN();
    GPIO_InitStruct.GPIO_Mode  = GPIO_Mode_Out_OD;  // ��©ģʽ(�ҽ������������)��, �ɶ���д (���忴�ֲ�)
    GPIO_InitStruct.GPIO_Pin   = DS18B20_GPIO_PIN;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(DS18b20_GPIO_PORT, &GPIO_InitStruct);
}

uint8_t ds18b20_start(void)
{
    uint8_t ack;
    DS18B20_DQ_0();
    delay_us(500);
    DS18B20_DQ_1();
    delay_us(60);
    ack = DS18B20_DQ_N();
    delay_us(180);
    DS18B20_DQ_1();
    return ack;
}

void ds18b20_write_byte(uint8_t byte)
{
    uint8_t i;
    for (i = 0; i < 8; i++) {
        // write 1
        if (byte & 0x01) {
            DS18B20_DQ_0();
            delay_us(2);
            DS18B20_DQ_1();
            delay_us(60);
        }
        // write 0
        else {
            DS18B20_DQ_0();
            delay_us(60);
            DS18B20_DQ_1();
            delay_us(2);
        }
        byte >>= 1;
    }
}

uint8_t ds18b20_read_byte(void)
{
    uint8_t i;
    uint8_t byte;
    for (i = 0; i < 8; i++) {
        DS18B20_DQ_0();
        delay_us(2);
        DS18B20_DQ_1();
        delay_us(12);
        byte >>= 1;
        if (DS18B20_DQ_N())
            byte |= 0x80;
        delay_us(50);
    }
    return byte;
}

float ds18B20_read_temperature(int16_t* temp)
{
    uint8_t tl, th;

    int16_t t;

    if (ds18b20_start())
        return 0x7fff;

    ds18b20_write_byte(0xCC);  // skip rom
    ds18b20_write_byte(0x44);  // convert, �����¶�ת��

    if (ds18b20_start())
        return 0x7fff;

    ds18b20_write_byte(0xCC);  //  ����64λROM��ַ��ֱ����DS18B20���¶�ת�����������һ���ӻ�����
    ds18b20_write_byte(0xBE);  // �� DS18B20 �ڲ�RAM��9�ֽڵ��¶�����
    tl = ds18b20_read_byte();  // ��8λ
    th = ds18b20_read_byte();  // ��8λ

    t = (th << 8) + tl;

    if (temp) *temp = t;

    return t * 0.0625;
}
