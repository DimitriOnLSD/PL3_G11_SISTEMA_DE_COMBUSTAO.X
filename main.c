#include "mcc_generated_files/mcc.h"
#include <string.h>

#define VALVE_CONTROL_LENGTH 20

volatile adc_result_t adc_result;
volatile bool update = false; // adc update flag
bool new_data = false;
bool show_main_menu = true;
volatile uint64_t time = 0;

char valve_control[VALVE_CONTROL_LENGTH] = "";

bool alarm_disabled = false; // disable the alarm
bool user_override = false; // user can change valve opening manually if true

uint8_t rx_data;
uint8_t option;
uint8_t valve;
float pressure;
uint8_t min_pressure_threshold = 10;
uint8_t max_pressure_threshold = 150;

// 0.263658V -> 0kPa
// 4.87084V -> 250kPa
// y=mx+b
// m=(y2-y1)/(x2-x1) = (250 - 0)/(4.87084 - 0.263658) = 54.263104865403624167658234469574 ~= 54.263105
// b=y2-m*x1 = 0 - 54.263105 * 0.263658 = -14.306901702602588740796434783779 ~= -14.306917
const static float m = 54.263105;
const static float b = -14.306917;

unsigned char cnt_char = 0;
unsigned char s[4];

/*
                         Main application
 */

void interrupt_handler(void) {
    alarm_disabled = !alarm_disabled;
    EXT_INT0_InterruptFlagClear();
}

void timer0_interrupt_handler(void) {
    LATAbits.LATA5 = !LATAbits.LATA5;
    INTCONbits.TMR0IF = 0;
}

void timer1_interrupt_handler(void) {
    ADC_StartConversion();
    PIR1bits.TMR1IF = 0;
}

void adc_interrupt_handler(void) {
    update = true;
    adc_result = ADC_GetConversionResult();
}

void timer0_state(bool state) {
    if (state)
        INTCONbits.TMR0IE = 1;
    else
        INTCONbits.TMR0IE = 0;
}

void timer1_state(bool state) {
    if (state)
        PIE1bits.TMR1IE = 1;
    else
        PIE1bits.TMR1IE = 0;
}

void timer2_state(bool state) {
    if (state)
        PIE1bits.TMR2IE = 1;
    else
        PIE1bits.TMR2IE = 0;
}

void ccp1_state(bool state) {
    if (state)
        PIE1bits.CCP1IE = 1;
    else
        PIE1bits.CCP1IE = 0;
}

void ccp2_state(bool state) {
    if (state)
        PIE2bits.CCP2IE = 1;
    else
        PIE2bits.CCP2IE = 0;
}

void ccp3_state(bool state) {
    if (state)
        PIE4bits.CCP3IE = 1;
    else
        PIE4bits.CCP3IE = 0;
}

void led_state(bool state) {
    if (state)
        LATAbits.LATA5 = 1;
    else
        LATAbits.LATA5 = 0;
}

void turnOffAlarm() {
    led_state(false);
    ccp1_state(false);
    timer0_state(false);
    // timer2_state(false);
    TMR2_StopTimer();
    CCPR1L = 0;
}

void activateAlarm() {
    ccp1_state(true);
    timer0_state(true);
    // timer2_state(true);
    TMR2_StartTimer();
    CCPR1L = 50;
}

bool pressureOutsideThreshold() {
    return (pressure <= min_pressure_threshold || pressure >= max_pressure_threshold);
}

void updatePressureFromADC() {
    if (update) {
        const float maxADCValue = 1023.0;
        const float maxVoltage = 5.0;

        float adcVoltage = (float) adc_result * maxVoltage / maxADCValue;
        pressure = m * adcVoltage + b;
    }
}

void main_menu() {
    EUSART1_Write(12);
    if (user_override)
        printf("\r\n[1] - Controlo da pressao - Modo: Manual");
    else
        printf("\r\n[1] - Controlo da pressao - Modo: Automatico");
    printf("\r\n[2] - Definicao do grau de abertura da valvula");
    printf("\r\n[3] - Definicao dos limiares de alarme para a pressao (MIN/MAX)");
    printf("\r\n");
    printf("\r\nValor de pressao: %.2f kPa \r\nAbertura da valvula: %d", pressure, valve);
    printf("\r\nOpcao: ");
}

void menu_valve_control() {
    EUSART1_Write(12);
    printf("\r\n[1] - 0");
    printf("\r\n[2] - 25");
    printf("\r\n[3] - 50");
    printf("\r\n[4] - 75");
    printf("\r\n[5] - 100");
    printf("\r\n[0] - Sair (Controlo da pressao -> Automatico)");
}

void main(void) {
    SYSTEM_Initialize();
    ADC_SelectChannel(MPX4250);

    // EXT_INT0_InterruptDisable();
    // EXT_INT0_InterruptFlagClear()

    // TMR0_StartTimer();
    // TMR0_StopTimer():

    // TMR1_StartTimer();
    // TMR1_StopTimer();

    // TMR2_StartTimer();
    // TMR2_StopTimer();

    INT0_SetInterruptHandler(interrupt_handler); // Disable alarm
    TMR0_SetInterruptHandler(timer0_interrupt_handler);
    TMR1_SetInterruptHandler(timer1_interrupt_handler); // ADC update 1ms
    ADC_SetInterruptHandler(adc_interrupt_handler);

    INTERRUPT_GlobalInterruptHighEnable();
    INTERRUPT_GlobalInterruptLowEnable();
    INTERRUPT_PeripheralInterruptEnable();

    while (1) {
        if (update) {
            updatePressureFromADC();
            update = false;
        }

        if (alarm_disabled) {
            turnOffAlarm();
        } else {
            if (pressureOutsideThreshold())
                activateAlarm();
            else
                turnOffAlarm();
        }

        // if (user_override) {
        //     // control valve through user input. Will be fixed value
        // } else {
        //     if (pressure < 30) {
        //         // 0% valve open
        //     } else if (pressure >= 30 && pressure < 60) {
        //         // 25% valve open
        //     } else if (pressure >= 60 && pressure < 90) {
        //         // 50% valve open
        //     } else if (pressure >= 90 && pressure < 120) {
        //         // 75% valve open
        //     } else {
        //         // 100% valve open
        //     }
        // }

        if (EUSART1_is_rx_ready()) {
            rx_data = EUSART1_Read();
            EUSART1_Write(rx_data);
            if ((rx_data >= '0' && rx_data <= '9') || rx_data == 13) // if [0:9] or ENTER
            {
                new_data = true;
                option = rx_data;
            }
        }

        if (show_main_menu) {
            main_menu();
            show_main_menu = false;
        }

        if (new_data) {
            switch (option) {
                case 0:
                    break;
                case '1':
                    user_override = !user_override;
                    show_main_menu = true;
                    break;
                case '2':
                    if (!user_override) {
                        EUSART1_Write(12);
                        printf("\r\n Controlo da pressao tem de ser manual para definir um grau de abertura da valvula");
                    } else {
                        menu_valve_control();
                    }
                    show_main_menu = false;
                    break;
                case '3':
                    // EUSART1_Write(12);
                    // printf("\r\nValor minimo atual: %d\t Valor maximo atual: %d", min_pressure_threshold, max_pressure_threshold);
                    // printf("\r\nNovo valor minimo: ");
                    // s[cnt_char] = rx_data;
                    // if (cnt_char == 2 || rx_data == 13) {
                    //     if (cnt_char == 2)
                    //         cnt_char++;
                    //     s[cnt_char] = '\0';
                    //     min_pressure_threshold = atoi(s);
                    // } else {
                    //     cnt_char++;
                    // }
                    // EUSART1_Write(12);
                    // printf("\r\nNovo valor maximo: ");
                    // s[cnt_char] = rx_data;
                    // if (cnt_char == 2 || rx_data == 13) {
                    //     if (cnt_char == 2)
                    //         cnt_char++;
                    //     s[cnt_char] = '\0';
                    //     max_pressure_threshold = atoi(s);
                    // } else {
                    //     cnt_char++;
                    // }
                    // break;
                default:
                    break;
            }
            new_data = false;
        }
    }
}
/**
 End of File
 */