#ifndef MAIN_H
#define MAIN_H

/* VARIABLES */

/**
 * @struct valve
 * @brief Structure to manage valve parameters.
 */
struct valve
{
    int current_angle;    /**< Stores valve current opening angle */
    int current_angle_percentage;    /**< Stores valve current opening angle in percentage */
    int target_angle;     /**< Sets target open angle for valve */
    int previous_angle;   /**< Stores last valve angle */
    int user_input_angle; /**< User-defined input angle */
};

/**
 * @struct sensor
 * @brief Structure to manage sensor thresholds and data.
 */
struct sensor
{
    int min_threshold;            /**< Sets minimum pressure threshold for combustion chamber */
    int max_threshold;            /**< Sets maximum pressure threshold for combustion chamber */
    volatile double current_data; /**< Current sensor data */
    double previous_data;         /**< Previous sensor data */
};

/**
 * @struct data
 * @brief Structure to manage general data and ADC updates.
 */
struct data
{
    volatile double voltage;      /**< Voltage data */
    volatile adc_result_t result; /**< ADC result */
    volatile bool update;         /**< Flag indicating ADC update */
};
/* FLAG CLEAR */

#define TMR0_InterruptFlagClear() (INTCONbits.TMR0IF = 0) // Clears Timer 0 interrupt flag
#define TMR1_InterruptFlagClear() (PIR1bits.TMR1IF = 0)   // Clears Timer 1 interrupt flag
#define TMR2_InterruptFlagClear() (PIR1bits.TMR2IF = 0)   // Clears Timer 2 interrupt flag
#define TMR4_InterruptFlagClear() (PIR5bits.TMR4IF = 0)   // Clears Timer 4 interrupt flag

/* INTERRUPT HANDLERS */

/**
 * @brief Button Interrupt Service Routine (ISR) for toggling the alarm state.
 *
 * This ISR is triggered by a button press and toggles the alarm state.
 * Ensure the button interrupt is properly configured in hardware.
 *
 * @note This function should be registered as an ISR for the button interrupt.
 *
 * @warning Avoid lengthy operations or delays within this ISR, as it might affect other system operations.
 *
 * @details This ISR toggles the 'alarm_disabled' variable, controlling the alarm state.
 *          It also clears the interrupt flag for the external interrupt.
 *
 * @see alarm_disabled To check or modify the state of the alarm.
 * @see EXT_INT0_InterruptFlagClear() To clear the interrupt flag.
 */
void INT0_interruptHandler(void);

/**
 * @brief Timer 0 Interrupt Service Routine (ISR) toggling an LED.
 *
 * This ISR is triggered by Timer 0 at a frequency of 2Hz controlled by the alarm.
 * Ensure Timer 0, alarm, and LED configurations are properly set.
 *
 * @note This function should be registered as an ISR for Timer 0 overflow interrupt.
 *
 * @warning Avoid lengthy operations or delays within this ISR, as it might affect timer accuracy.
 *
 * @details This ISR toggles the state of an LED at a frequency of 2Hz.
 *          - The LED state is toggled (ON/OFF) each time this ISR is triggered by Timer 0.
 *          - It also clears the interrupt flag for Timer 0.
 *
 * @see LED_LAT To control the LED state.
 * @see TMR0_InterruptFlagClear() To clear the Timer 0 interrupt flag.
 * @see ConfigureTimer0() To set up Timer 0 frequency and triggering mechanism.
 * @see alarmISR() To understand the alarm configuration triggering Timer 0.
 */
void TMR0_interruptHandler(void);

/**
 * @brief Timer 1 Interrupt Service Routine (ISR) triggering ADC conversion for the MPX4250 pressure sensor.
 *
 * This ISR is triggered by Timer 1 overflow and starts an ADC conversion for the MPX4250 sensor.
 * Ensure Timer 1, ADC, and sensor configurations are properly set.
 *
 * @note This function should be registered as an ISR for Timer 1 overflow interrupt.
 *
 * @warning Avoid lengthy operations or delays within this ISR, as it might affect timer accuracy and ADC conversions.
 *
 * @details This ISR initiates an ADC conversion specifically for the MPX4250 sensor upon Timer 1 overflow.
 *          - The ADC is started for the MPX4250 sensor data acquisition.
 *          - It clears the interrupt flag for Timer 1 to handle subsequent interrupts.
 *
 * @see ADC_StartConversion() To start the ADC conversion.
 * @see configureTimer1() To set up and configure Timer 1.
 * @see MPX4250 datasheet for sensor details and specifications.
 */
void TMR1_interruptHandler(void);

/**
 * @brief Timer 2 Interrupt Service Routine (ISR) controlling a buzzer signal.
 *
 * This ISR is triggered by Timer 2 overflow and controls a buzzer signal.
 * Ensure Timer 2 and buzzer configurations are properly set.
 *
 * @note This function should be registered as an ISR for Timer 2 overflow interrupt.
 *
 * @warning Avoid lengthy operations or delays within this ISR, as it might affect timer accuracy and buzzer signal timing.
 *
 * @details This ISR manages the buzzer signal based on a specific timing pattern:
 *          - It increments a count variable representing milliseconds.
 *          - Controls the buzzer signal ON/OFF based on the count value:
 *            - For a specified duration, the buzzer is ON with a certain duty cycle (BUZZER_DC).
 *            - The signal lasts for a specific duration (BUZZER_DURATION) and then resets.
 *
 * @param BUZZER_DC Duty cycle for the buzzer signal when active.
 * @param BUZZER_DURATION Duration of the buzzer signal.
 * @param TMR2_COUNTER Timer 2 counter value for the desired timing.
 *
 * @see EPWM1_LoadDutyValue() To load the duty cycle for the buzzer signal.
 * @see configureTimer2() To set up and configure Timer 2.
 * @see Buzzer datasheet for specific signal requirements and timing.
 */
void TMR2_interruptHandler(void);

/**
 * @brief ADC Interrupt Service Routine (ISR) handling conversion results.
 *
 * This ISR is triggered by the completion of an ADC conversion and updates a result variable.
 * Ensure ADC configurations and result variable initialization are properly set.
 *
 * @note This function should be registered as an ISR for the ADC conversion complete interrupt.
 *
 * @warning Ensure correct initialization of the ADC module and result variable before enabling this ISR.
 *
 * @details This ISR updates a flag to indicate a new conversion result and retrieves the ADC conversion result.
 *          - It sets a flag 'update' to indicate a new conversion result is available.
 *          - Reads and stores the latest ADC conversion result in 'adc_result'.
 *
 * @param update Flag indicating if a new conversion result is available.
 * @param adc_result Variable storing the latest ADC conversion result.
 *
 * @see ADC_GetConversionResult() To retrieve the ADC conversion result.
 * @see configureADC() To set up and configure the ADC module.
 * @see ADC datasheet for specific conversion and result retrieval details.
 */
void ADC_interruptHandler(void);

/* ALARM HANDLER */

void turnOffAlarm();
void triggerAlarm();

/* PRESSURE HANDLERS */

bool pressureOutsideThreshold();
void updatePressureFromADC();
int setPressureThreshold(int new_threshold);

/* MENU HANDLERS */

void main_menu();
void valve_control_menu();

/* STEPPER MOTOR HANDLERS */

void rotateSteps(int steps);

/* MAIN */

void main(void);

#endif