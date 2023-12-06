#include "mcc_generated_files/mcc.h"

volatile bool update = false; // adc update flag
volatile adc_result_t adc_result;
volatile uint64_t time = 0;

float voltage = 0;
bool alarm_disabled = false; // disable the alarm
bool user_override = false; // user can change valve opening manually if true

uint8_t pressure;
uint8_t min_pressure = 10;
uint8_t max_presure = 150;

// 0.263658V -> 0kPa
// 4.87084V -> 250kPa
// y=mx+b
// m=(y2-y1)/(x2-x1) = (250 - 0)/(4.87084 - 0.263658) = 54.263104865403624167658234469574 ~= 54.263105
// b=y2-m*x1 b       = 0 - 54.263105 * 0.263658 = -14.306901702602588740796434783779 ~= -14.306917
static float m = 54.263105;
static float b = -14.306917;

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

void adc_interrupt_handler(void) {
    update = true;
    adc_result = ADC_GetConversionResult();
}

void main(void) {
    SYSTEM_Initialize();
    ADC_SelectChannel(MPX4250);

    INT0_SetInterruptHandler(interrupt_handler);
    TMR0_SetInterruptHandler(timer0_interrupt_handler);
    TMR1_SetInterruptHandler(timer1_interrupt_handler);
    ADC_SetInterruptHandler(adc_interrupt_handler);

    INTERRUPT_GlobalInterruptHighEnable();
    INTERRUPT_GlobalInterruptLowEnable();
    INTERRUPT_PeripheralInterruptEnable();

    while (1) {
        if (update) {
            voltage = (float) adc_result * 5.0 / 1023.0;
            pressure = m * voltage + b;
            update = false;
        }

        if (alarm_disabled) {
            INTCONbits.TMR0IE = 0;
            LATAbits.LATA5 = 0;
        } else {
            if (pressure <= min_pressure || pressure >= max_presure) {
                // led 2hz buzzer 0.5hz
                INTCONbits.TMR0IE = 1;
            } else {
                // led and buzzer (pwm) off
                INTCONbits.TMR0IE = 0;
            }
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
    }
}
/**
 End of File
 */