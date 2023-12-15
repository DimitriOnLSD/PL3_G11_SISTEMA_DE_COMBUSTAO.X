#ifndef _FLAG_CLEAR_H
#define _FLAG_CLEAR_H

#define TMR0_InterruptFlagClear() (INTCONbits.TMR0IF = 0)
#define TMR1_InterruptFlagClear() (PIR1bits.TMR1IF = 0)
#define TMR2_InterruptFlagClear() (PIR1bits.TMR2IF = 0)

#define TMR4_InterruptFlagClear() (PIR5bits.TMR4IF = 0)

#endif