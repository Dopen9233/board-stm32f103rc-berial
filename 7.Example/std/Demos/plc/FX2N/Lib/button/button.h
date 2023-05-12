#ifndef BUTTON_H
#define BUTTON_H

#include "board.h"

#define BTN_NAME_MAX              32  // �������Ϊ32�ֽ�

/* ��������ʱ��40ms, �����������Ϊ20ms
 ֻ��������⵽40ms״̬�������Ϊ��Ч����������Ͱ��������¼�
*/

#define CONTINUOS_TRIGGER         0  // �Ƿ�֧�����������������Ļ��Ͳ�Ҫ��ⵥ˫���볤����

/* �Ƿ�֧�ֵ���&˫��ͬʱ���ڴ��������ѡ�����궨��Ļ�����˫�����ص���ֻ�����������ӳ���Ӧ��
   ��Ϊ�����жϵ���֮���Ƿ񴥷���˫�������ӳ�ʱ����˫�����ʱ�� BUTTON_DOUBLE_TIME��
   ���������������궨�壬���鹤����ֻ���ڵ���/˫���е�һ����������˫����Ӧ��ʱ��ᴥ��һ�ε�����
   ��Ϊ˫����������һ�ΰ��²����ͷ�֮��Ų����� */
#define SINGLE_AND_DOUBLE_TRIGGER 1

/* �Ƿ�֧�ֳ����ͷŲŴ��������������궨�壬��ô�����ͷ�֮��Ŵ������γ�����
   �����ڳ���ָ��ʱ���һֱ�������������������� BUTTON_LONG_CYCLE ���� */
#define LONG_FREE_TRIGGER         0

#define BUTTON_DEBOUNCE_TIME      2   // ����ʱ��          (n-1)*��������
#define BUTTON_CONTINUOS_CYCLE    1   // ������������ʱ��  (n-1)*��������
#define BUTTON_LONG_CYCLE         1   // ������������ʱ��  (n-1)*��������
#define BUTTON_DOUBLE_TIME        15  // ˫�����ʱ��      (n-1)*��������  ������200-600ms
#define BUTTON_LONG_TIME          50  /* ����n�� ((n-1)*�������� ms)����Ϊ�����¼� */

#define TRIGGER_CB(event)     \
    if (btn->callback[event]) \
    btn->callback[event]((button_t*)btn)

typedef void (*button_cbk_t)(void*);  // �����¼��ص�

typedef enum {
    BUTTON_DOWM = 0,
    BUTTON_UP,
    BUTTON_DOUBLE,
    BUTTON_LONG,
    BUTTON_LONG_FREE,
    BUTTON_CONTINUOS,
    BUTTON_CONTINUOS_FREE,
    BUTTON_ALL_RIGGER,
    number_of_event,
    NONE_TRIGGER
} button_event_e;

typedef struct button {
    uint8_t (*read_level)(void);  // ��������ƽ

    char name[BTN_NAME_MAX];

    uint8_t current_state : 4;  // ��ǰ״̬ current state, ���»���
    uint8_t last_state    : 4;  // �ϴ�״̬ last state, ˫���ж�
    uint8_t trigger_level : 2;  // ������ƽ trigger level
    uint8_t current_level : 2;  // ��ǰ��ƽ current level

    uint8_t trigger_event;  // ��ǰ�������¼�

    button_cbk_t callback[number_of_event];

    uint8_t t_cycle;  // ������������

    uint8_t t_count;     // ��ʱ
    uint8_t t_debounce;  // ����ʱ��

    uint8_t t_press;  // ������������ʱ��

    struct button* next;

} button_t;

void button_create(const char* name, button_t* btn, uint8_t (*read_level)(void), uint8_t trigger_level);
void button_attach(button_t* btn, button_event_e btn_event, button_cbk_t btn_callback);
void button_process(button_t* btn);
void button_cycle_process(void);
void button_delete(button_t* btn);

void button_enumrate_name(void);

uint8_t button_get_event(button_t* btn);
uint8_t button_get_state(button_t* btn);

#if 0

// ͨ�ûص�

void button_process_callback(void* btn)
{
    uint8_t event = button_get_event(btn);

    switch (event) {
        case BUTTON_DOWM: {
            break;
        }
        case BUTTON_UP: {
            break;
        }
        case BUTTON_DOUBLE: {
            break;
        }
        case BUTTON_LONG: {
            break;
        }
        case BUTTON_LONG_FREE: {
            break;
        }
        case BUTTON_CONTINUOS: {
            break;
        }
        case BUTTON_CONTINUOS_FREE: {
            break;
        }
    }
}

#endif

#endif
