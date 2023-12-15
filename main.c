#include "mcc_generated_files/mcc.h"
#include <string.h>
#include "flag_clear.h"

#define MAX_INPUT_LENGTH 10
#define MAX_ADC_VALUE 1023.0
#define MAX_VREF 5.0

volatile adc_result_t adc_result;
volatile bool update = false; // adc update flag
volatile double voltage;
volatile double pressure;

bool new_data = false;
bool show_main_menu = true;
bool show_error = false;
bool set_threshold = false;
bool alarm_disabled = false; // disable the alarm
bool user_override = false; // user can change valve opening manually if true

char rx_data;
uint8_t str[MAX_INPUT_LENGTH];
uint8_t option;
uint8_t valve;
uint8_t min_pressure_threshold = 10;
uint8_t max_pressure_threshold = 150;
uint16_t count = 0;

// 0.263658V -> 0kPa
// 4.87084V -> 250kPa
// y=mx+b
// m=(y2-y1)/(x2-x1) = (250 - 0)/(4.87084 - 0.263658) = 54.263104865403624167658234469574 ~= 54.263105
// b=y2-m*x1 = 0 - 54.263105 * 0.263658 = -14.306901702602588740796434783779 ~= -14.306917
const static double m = 54.263105;
const static double b = -14.306917;

void INT_interruptHandler(void) {
    alarm_disabled = !alarm_disabled;
    EXT_INT0_InterruptFlagClear();
}

void TMR0_interruptHandler(void) {
    LATAbits.LATA5 = !LATAbits.LATA5;
    TMR0_InterruptFlagClear(); // Clear the Timer0 interrupt flag
}

void TMR1_interruptHandler(void) {
    ADC_StartConversion();
    TMR1_InterruptFlagClear(); // Clear the Timer1 interrupt flag
}

void TMR2_interruptHandler(void) {
    count++; // 1ms
    if (count >= 2000) { // 0.5 Hz
        EPWM1_LoadDutyValue(100);
        if (count >= 2500) { // signal lasts for 500 ms
            count = 0;
        }
    } else {
        EPWM1_LoadDutyValue(0);
    }
    TMR2_InterruptFlagClear(); // Clear the Timer2 interrupt flag
}

void TMR4_interruptHandler(void) {
    TMR4_InterruptFlagClear();
}

void ADC_interruptHandler(void) {
    update = true;
    adc_result = ADC_GetConversionResult();
}

void turnOffAlarm() {
    LATAbits.LATA5 = 0;
    TMR0_StopTimer();
    TMR2_StopTimer();
}

void triggerAlarm() {
    TMR0_StartTimer();
    TMR2_StartTimer();
}

bool pressureOutsideThreshold() {
    return (pressure <= min_pressure_threshold || pressure >= max_pressure_threshold);
}

void updatePressureFromADC() {
    voltage = (double) adc_result * MAX_VREF / MAX_ADC_VALUE;
    pressure = m * voltage + b;
}

void main_menu() {
    EUSART1_Write(12);
    printf("\r\n-SISTEMA DE CONTROLO DA PRESSAO DE UMA CAMARA DE COMBUSTAO-\r\n");

    printf("\r\n[1] - Controlo da pressao - Modo: %s", user_override ? "Manual" : "Automatico");
    printf("\r\n[2] - Definicao do grau de abertura da valvula");
    printf("\r\n[3] - Definicao dos limiares de alarme para a pressao (MIN/MAX)\r\n");

    printf("\r\nValor de pressao: %.2f kPa", pressure);
    printf("\r\nAbertura da valvula: %d %%\r\n", valve);

    if (show_error) {
        printf("\r\nControlo da pressao tem de ser manual para definir um grau de abertura da valvula\r\n");
    }

    printf("\r\nOpcao: ");
}

void valve_control_menu() {
    EUSART1_Write(12);
    printf("\r\n[1] - 0%%");
    printf("\r\n[2] - 25%%");
    printf("\r\n[3] - 50%%");
    printf("\r\n[4] - 75%%");
    printf("\r\n[5] - 100%%");
    printf("\r\n[0] - Sair (Controlo da pressao -> Automatico)\r\n");

    printf("\r\nOpcao: ");
}

uint8_t setPressureThreshold(uint8_t new_threshold) {
    uint8_t i = 0;
    bool set_threshold = true;
    while (set_threshold) {
        if (EUSART1_is_rx_ready()) {
            rx_data = EUSART1_Read();
            if (rx_data >= '0' && rx_data <= '9' && i < MAX_INPUT_LENGTH - 1) {
                str[i++] = rx_data;
                EUSART1_Write(rx_data);
            } else if (rx_data == 8 && i > 0) { // Backspace
                i--;
                EUSART1_Write(8);
                EUSART1_Write(' '); // Clear the character on terminal
                EUSART1_Write(8);
            } else if (rx_data == 13) { // Enter
                set_threshold = false;
                str[i] = '\0';
                return new_threshold = atoi(str);
            }
        }
    }
}

void main(void) {
    SYSTEM_Initialize();
    ADC_SelectChannel(MPX4250);

    INT0_SetInterruptHandler(INT_interruptHandler);
    TMR0_SetInterruptHandler(TMR0_interruptHandler);
    TMR1_SetInterruptHandler(TMR1_interruptHandler);
    TMR2_SetInterruptHandler(TMR2_interruptHandler);
    TMR4_SetInterruptHandler(TMR4_interruptHandler);
    ADC_SetInterruptHandler(ADC_interruptHandler);

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
                triggerAlarm();
            else
                turnOffAlarm();
        }

        if (user_override) {
            EPWM2_LoadDutyValue(valve);
        } else {
            if (pressure < 30) {
                EPWM2_LoadDutyValue(0);
                EPWM3_LoadDutyValue(0);
            } else if (pressure >= 30 && pressure < 60) {
                EPWM2_LoadDutyValue(100);
                EPWM3_LoadDutyValue(100);
            } else if (pressure >= 60 && pressure < 90) {

            } else if (pressure >= 90 && pressure < 120) {

            } else {
                
            }
        }

        if (EUSART1_is_rx_ready() && !set_threshold) {
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
            show_error = false;
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
                        show_error = true;
                        show_main_menu = true;
                    } else {
                        valve_control_menu();
                    }
                    break;
                case '3':
                    EUSART1_Write(12);
                    printf("\r\nValor minimo atual: %d kPa\t Valor maximo atual: %d kPa\n", min_pressure_threshold, max_pressure_threshold);
                    printf("\r\nNovo valor minimo: ");
                    min_pressure_threshold = setPressureThreshold(min_pressure_threshold);
                    printf("\r\nNovo valor maximo: ");
                    max_pressure_threshold = setPressureThreshold(max_pressure_threshold);
                    show_main_menu = true;
                    break;
            }
            new_data = false;
        }
    }
}