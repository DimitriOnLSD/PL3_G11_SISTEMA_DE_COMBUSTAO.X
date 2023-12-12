#include "mcc_generated_files/mcc.h"
#include <string.h>

volatile adc_result_t adc_result;
volatile bool update = false; // adc update flag
bool newData = true;
bool show_main_menu = true;
volatile uint64_t time = 0;

char valve_control[] = "";

bool alarm_disabled = false; // disable the alarm
bool user_override = false; // user can change valve opening manually if true

uint8_t rxData;
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
    INTCONbits.INT0IF = 0;
}

void timer0_interrupt_handler(void) {
    LATAbits.LATA5 = !LATAbits.LATA5;
    INTCONbits.TMR0IF = 0;
}

void timer1_interrupt_handler(void) {
    ADC_StartConversion();
    PIR1bits.TMR1IF = 0;
}

void timer2_interrupt_handler(void) {
    EPWM1_LoadDutyValue(100);
    PIR1bits.CCP1IF = 0;
    PIR1bits.TMR2IF = 0;
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

void timer3_state(bool state) {
    if (state)
        PIE2bits.TMR3IE = 1;
    else
        PIE2bits.TMR3IE = 0;
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
    timer2_state(false);
}

void activateAlarm() {
    ccp1_state(true);
    timer0_state(true);
    timer2_state(true);
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

void changeString(bool user_override) {
    if (user_override)
        valve_control[6] = "Manual";
    else
        valve_control[10] = "Automatico";
}

void main_menu() {
    EUSART1_Write(12);
    printf("\r\n[1] - Controlo da pressao - Modo: %s", valve_control);
    printf("\r\n[2] - Definicao do grau de abertura da valvula");
    printf("\r\n[3] - Definicao dos limiares de alarme para a pressao (MIN/MAX)");
    printf("\r\nValor de pressao: %f \tAbertura da valvula: %d", pressure, valve);
}

void main(void) {
    SYSTEM_Initialize();
    ADC_SelectChannel(MPX4250);

    INT0_SetInterruptHandler(interrupt_handler); // Disable alarm
    TMR0_SetInterruptHandler(timer0_interrupt_handler);
    TMR1_SetInterruptHandler(timer1_interrupt_handler); // ADC update 1ms
    TMR2_SetInterruptHandler(timer2_interrupt_handler);
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

        if (!user_override) {
            if (pressure < 30) {
                // 0% valve open
            } else if (pressure >= 30 && pressure < 60) {
                // 25% valve open
            } else if (pressure >= 60 && pressure < 90) {
                // 50% valve open
            } else if (pressure >= 90 && pressure < 120) {
                // 75% valve open
            } else {
                // 100% valve open
            }
        } else {
            // control valve through user input. Will be fixed value
        }

        if (EUSART1_is_rx_ready()) {
            rxData = EUSART1_Read();
            EUSART1_Write(rxData);
            if ((rxData >= '0' && rxData <= '9') || rxData == 13) // if [0:9] or ENTER
            {
                newData = true;
                option = rxData;
            }
        }

        main_menu();

        if (newData) {
            switch (option) {
                case 0:
                    break;
                case 1:
                    user_override = !user_override;
                    changeString(user_override);
                    break;
                case 2:
                    if (user_override) {
                        printf("\r\n Controlo da pressao tem de ser manual para definir um grau de abertura da valvula");
                    } else {
                        // control valve through user input. Will be fixed value
                    }
                    show_main_menu = false;
                    break;
                case 3:
                    printf("\r\nValor minimo atual: %d\t Valor maximo atual: %d", min_pressure_threshold, max_pressure_threshold);
                    printf("\r\nNovo valor minimo: ");
                    s[cnt_char] = rxData;
                    if (cnt_char == 2 || rxData == 13) {
                        if (cnt_char == 2)
                            cnt_char++;
                        s[cnt_char] = '\0';
                        min_pressure_threshold = atoi(s);
                    } else {
                        cnt_char++;
                    }
                    printf("\r\nNovo valor maximo: ");
                    s[cnt_char] = rxData;
                    if (cnt_char == 2 || rxData == 13) {
                        if (cnt_char == 2)
                            cnt_char++;
                        s[cnt_char] = '\0';
                        max_pressure_threshold = atoi(s);
                    } else {
                        cnt_char++;
                    }
                    break;
                default:
                    break;
            }
            newData = false;
        }
    }
}
/**
 End of File
 */