

#include "dht11.h"

static uint8_t dht11_read_byte(void);
static void    dht11_set_output_mode(void);
static void    dht11_set_input_mode(void);

void dht11_init(void)
{
    DHT11_GPIO_CLK_EN();
    dht11_set_output_mode();
    DHT11_DATA_1();
}

static void dht11_set_output_mode(void)
{
    // dht11 ������ DS18B20 һ��,ʹ�ÿ�©ģʽ����ʼ������������, ����Ͷ�ȡʧ����
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin   = DHT11_GPIO_PIN;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(DHT11_GPIO_PORT, &GPIO_InitStructure);
}

static void dht11_set_input_mode(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin   = DHT11_GPIO_PIN;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IPU;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(DHT11_GPIO_PORT, &GPIO_InitStructure);
}

static uint8_t dht11_read_byte(void)
{
    uint8_t i, temp = 0;

    for (i = 0; i < 8; i++) {
        // ÿbit��50us�͵�ƽ���ÿ�ʼ����ѯֱ���ӻ����� ��50us �͵�ƽ ����
        while (Bit_RESET == DHT11_DATA_N()) {}

        /**
         * DHT11 ��26~28us�ĸߵ�ƽ��ʾ��0������70us�ߵ�ƽ��ʾ��1����
         * ͨ����� x us��ĵ�ƽ��������������״ ��x ���������ʱ
         */
        DHT11_DELAY_US(40);  // ��ʱx us �����ʱ��Ҫ��������0������ʱ�伴��

        // ���� 1
        if (Bit_SET == DHT11_DATA_N()) {
            // �ȴ�����1�ĸߵ�ƽ����
            while (Bit_SET == DHT11_DATA_N()) {}
            temp |= (uint8_t)(0x01 << (7 - i));  // MSB
        }
        // ���� 0
        else {
            temp &= (uint8_t) ~(0x01 << (7 - i));  // MSB
        }
    }

    return temp;
}

bool dht11_read_temp_and_humidity(dht11_data_t* data)
{
    dht11_set_output_mode();

    // �������� 18ms
    DHT11_DATA_0();
    DHT11_DELAY_MS(18);

    // �������� 30us
    DHT11_DATA_1();
    DHT11_DELAY_US(30);

    // �жϴӻ��Ƿ��е͵�ƽ��Ӧ�ź�

    dht11_set_input_mode();

    if (DHT11_DATA_N() == Bit_RESET) {
        // ��ѯֱ���ӻ����� ��80us �͵�ƽ ��Ӧ�źŽ���
        while (DHT11_DATA_N() == Bit_RESET) {}

        // ��ѯֱ���ӻ������� 80us �ߵ�ƽ �����źŽ���
        while (DHT11_DATA_N() == Bit_SET) {}

        /* ���ݽ���
         * һ�����������ݴ���Ϊ40bit����λ�ȳ�
         * 8bit ʪ������ + 8bit ʪ��С�� + 8bit �¶����� + 8bit �¶�С�� + 8bit У���
         */
        data->humi_int  = dht11_read_byte();
        data->humi_deci = dht11_read_byte();
        data->temp_int  = dht11_read_byte();
        data->temp_deci = dht11_read_byte();
        data->check_sum = dht11_read_byte();

        // ��������
        DHT11_DATA_1();

        // ����У��
        return data->check_sum == (data->humi_int + data->humi_deci + data->temp_int + data->temp_deci);
    }

    return false;
}
