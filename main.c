#include "mcc_generated_files/mcc.h"

volatile adc_result_t adc_result;
volatile bool update = false; // adc update flag
volatile uint64_t time = 0;

bool alarm_disabled = false; // disable the alarm
bool user_override = false; // user can change valve opening manually if true

uint8_t pressure;
uint8_t min_pressure_threshold = 10;
uint8_t max_pressure_threshold = 150;

// 0.263658V -> 0kPa
// 4.87084V -> 250kPa
// y=mx+b
// m=(y2-y1)/(x2-x1) = (250 - 0)/(4.87084 - 0.263658) = 54.263104865403624167658234469574 ~= 54.263105
// b=y2-m*x1 = 0 - 54.263105 * 0.263658 = -14.306901702602588740796434783779 ~= -14.306917
const static float m = 54.263105;
const static float b = -14.306917;

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

        float adcVoltage = (float)adc_result * maxVoltage / maxADCValue;
        pressure = m * adcVoltage + b;
    }
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
    }
}
/**
 End of File
 */