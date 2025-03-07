/****************************************************************************************
 * SCDADMCT_DemoPhaseSingleStructure_mainFile.c
 *
 *  Created on: Dec 19, 2024
 *      Author: dan
 *  Descr: Main file of the program
 *
 *
 */

/****************************************************************************************
 * INCLUDE AREA
 */
#include <msp430.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>

/****************************************************************************************
 * DATA TYPES
 */

/*
 * Global pointer for the message template
 */
typedef struct {
    /* Pointer tomessage buffer */
    char *data;
    /* Message size */
    size_t size;
    /* Flag to signal that the message is ready to be sendt trough UART TX Bufffer */
    bool ready;
} DynamicBuffer;

/*
 * Baud rate option array: [Baud Rate 0, Baud Rate 1]
 */
#define BR_SET_0 0x03
#define BR_SET_1 0x00
const uint8_t COM_UART_BROption[2] = {BR_SET_0, BR_SET_1};

/*
 * Baud rate setting for BRClk - AClk and baudrate of 9600bps
 */
#define BR_MSET 0x9200

/*
 * TB0_Divider_CCR
 */

/* 1- 1 second, 2- 1/2 seconds, 4- 1/4 seconds */
#define TB0_DELAY_SECONDS 4
#if TB0_DELAY_SECONDS == 1
    #define TB0_CCR0_DIV 32768u
#elif TB0_DELAY_SECONDS == 2
    #define TB0_CCR0_DIV 16384u
#elif TB0_DELAY_SECONDS == 4
    #define TB0_CCR0_DIV 8192u
#endif
/*
 * TB1_Divider_CCR
 */
#define TB1_CCR0_DIV 20000u

/*
 * SG90 Servo Positions
 */

/* 0° -> 1.5 ms [IDEAL: 1500ms --> REAL: 1750ms]*/
#define SG90_0DEG 1750u
/* 90° -> 2 ms [IDEAL: 2000ms --> REAL: 2750ms]*/
#define SG90_P90DEG 2750u
/* -90° -> 1 ms [IDEAL: 1000ms --> REAL: 730ms]*/
#define SG90_N90DEG 730u
/* 1° -> ms => */
#define SG90_1DEG 11u
/* Calib tolerances */
#define SG90_45DEG_CALTOL 500
#define SG90_30DEG_CALTOL 200

/* SG90 degree motion + null terminator */
volatile char UART_RX_Buff[4];
volatile uint8_t nrOfDegrees;
volatile uint8_t setNrOfDegrees;

/*
 * UART settings
 */
#define UART_HDL_KEYBOARD 0u
#define UART_HDL_LABVIEWSERIAL 1u

/*
 * SG90 Calib macors
 */
#define SG90_CALIB_TIME_MS 250
#define SG90_LONG_CALIB 0u
#define SG90_SHRT_CALIB 1u

/*
 * Timer B0 counter
 */
volatile long tb0_cnt;

/*
 * Clock system frequency divider factor
 */
#define CS_DF 487u

/****************************************************************************************
 * END OF DATA TYPES
 */

/****************************************************************************************
 * FUNCTION PROTOTYPES
 */

/*********************************_DYNAMIC_BUFFER_**************************************/

/****************************************************************************************
 * Func name: allocateMessageBuffer
 * Descr: Prototype, set a dynamic message template size in the buffer.
 * @param size: The message string to be set as template size.
 */
void allocateMessageBuffer(size_t size);

/****************************************************************************************
 * Func name: setMessageInBuffer
 * Descr: Prototype, set a dynamic message template.
 * @param message: The message string to be set as template.
 */
void setMessageInBuffer(const char *message);

/****************************************************************************************
 * Func name: freeMessageBuffer
 * Descr: Prototype, free the memory allocated for the message template.
 * @param: none
 */
void freeMessageBuffer();

/***********************************_CLOCK_SYSTEM_**************************************/

/****************************************************************************************
 * Func name: ClockSystem_ConfigureClockSystem
 * Descr: Function prototype of ClockSystem_ConfigureClockSystem
 * @params:
 *
 *
 */
void ClockSystem_ConfigureClockSystem(void);

/****************************************************************************************
 * Func name: ClockSystem_Callback
 * Descr: This interface is a callback for CS functions
 * @params: function pointer
 *
 *
 */
void ClockSystem_Callback(void(*fptr)(void));

/*************************************_UART_eUSCI_**************************************/

/****************************************************************************************
 * Func name: UART_COM_ConfigureUart
 * Descr: Function prototype of UART_COM_ConfigureUart
 * @params: none
 *
 *
 */
void UART_COM_ConfigureUart();

/****************************************************************************************
 * Func name: UART_COM_TransmitMessage
 * Descr: Function prototype for UART_COM_TransmitMessage
 * @params: none; data read from message pointer
 *
 *
 */
void UART_COM_TransmitMessage();

/****************************************************************************************
 * Func name: UART_COM_Callback
 * Descr: Function prototype of UART_COM callback
 * @params: fptr
 *
 *
 */
void UART_COM_Callback(void(*fptr)(void));

/****************************************************************************************
 * Func name: UART_COM_handle_UartRxBuff
 * Descr: Prototype for handle_UartRxBuff function. Hndles the angle for servo received form User trough uart rx buff
 * @param: uint8_t buff_idx, char received_char, uint8_t digit
 */
void UART_COM_handle_UartRxBuff(uint8_t buff_idx, char received_char, uint8_t digit);

/*************************************_TIMER_B_*****************************************/

/****************************************************************************************
 * Func name: TB_ConfigureTimerB0
 * Descr: Prototype of TB_ConfigureTimerB0
 * @params: none
 *
 *
 */
void TB_ConfigureTimerB0();

/****************************************************************************************
 * Func name: TB_ConfigureTimerB1
 * Descr: Prototype of TB_ConfigureTimerB1
 * @params:
 *
 *
 */
void TB_ConfigureTimerB1();

/****************************************************************************************
 * Func name: TB_Callback
 * Descr: Prototype of TB_Callback Callback
 * @params: fptr
 *
 *
 */
void TB_Callback(void(*fptr)(void));

/***********************************_WATCHDOG_TIMER_*************************************/

/****************************************************************************************
 * Func name: WDT_ConfigureWDT
 * Descr: Prototype of WDT_ConfigureWDT
 * @params: none
 *
 *
 */
void WDT_ConfigureWDT();

/****************************************************************************************
 * Func name: WDT_Callback
 * Descr: Prototype of WDT_Callback
 * @params: fptr
 *
 *
 */
void WDT_Callback(void(*fptr)(void));

/***********************************_SERVO_CONTROL_*************************************/

/****************************************************************************************
 * Func name: SG90_Calibration
 * Descr: Prototype for motor calibration.
 * @param: unsigned int calib_time, unsigned int sg90_firstAngle, unsigned int sg90_secondAngle
 */
void SG90_Calibration(unsigned int calib_time, unsigned int sg90_firstAngle, unsigned int sg90_secondAngle);

/****************************************************************************************
 * Func name: SG90_setAngle
 * Descr: Definition for SG90_setAngle function. Sets the angle of the servo motor
 * @param: nrOfDegrees
 */
void SG90_setAngle(uint8_t nrOfDegrees);

/****************************************************************************************
 * Func name: delay_ms
 * Descr: Prototype for delay_ms function.
 * @param: unsigned int ms
 */
void delay_ms(unsigned int ms);

/****************************************************************************************
 * END OF FUNCTION PROTOTYPES
 */


/****************************************************************************************
 * SETTINGS
 */

/* Init Dynamic Buffer */
DynamicBuffer messageBuffer = {NULL, 0, false};

/***************************************_MAIN_PROGRAM_**********************************/

/****************************************************************************************
 * Func name: main
 * Descr: main function of the program
 * @params: void
 *
 *
 */
int main(void)
{
    /* Init program counter */
    tb0_cnt = 0;
    /* Init SG90 roation */
    nrOfDegrees = 0;
    /* @descr: Watchdog timer config with 1 second interval interrupts */
    WDT_Callback(&WDT_ConfigureWDT);
    /* @descr: Config Clock System for AClk as source clock signal and MCLK = 16 Mhz and SMCLK = 1Mhz */
    ClockSystem_Callback(&ClockSystem_ConfigureClockSystem);
    /* @descr: Config Timer B0 for ACLK as source with 1 second interval interrupts */
    TB_Callback(&TB_ConfigureTimerB0);
    /* @descr: Config Timer B1 for ACLK as source with 5% PWM Duty Cycle */
    TB_Callback(&TB_ConfigureTimerB1);
    /* @descr: Config UART using callback with settings: BRClk = AClk (32768 Hz) and BaudRate = 9600bps */
    UART_COM_Callback(&UART_COM_ConfigureUart);
    /* P6.6 ---> signal light */
    P6DIR |= BIT6; P6OUT &=~BIT6;
    /* P1.0 --> signal light */
    P1DIR |= BIT0; P1OUT &=~BIT0;
    /* P2.0 --> SG90 servo pin */
    P2DIR |= BIT0; P2SEL1 &=~ BIT0; P2SEL0 |= BIT0;
    /* Disable high-impedance mode */
    PM5CTL0 &= ~LOCKLPM5;
    /* Enable maskable interrupts */
    __enable_interrupt();
    /* Enable WDT interrupts */
    SFRIE1 |= WDTIE;
    /* Enable eUSCI UART intterupts */
    UCA1IE |= UCRXIE;

    /* SG90 Calibration: x ms pace ; set -45°~45°; set -30°~30° */
    SG90_Calibration(SG90_CALIB_TIME_MS, SG90_45DEG_CALTOL, SG90_30DEG_CALTOL);

    /******************************************************************************
     * MAIN LOOP
     *
     */
    for(;;)
    {
        /* Temporary buffer for formatting the message */
        char tempMessage[100];
        /* Format the message string including the counter value */
        snprintf(tempMessage, sizeof(tempMessage),
                 "Program counter [TB0]: %d ticks size: %d  [Servo rotation: %d deg. [temp val: %d]| PWM: %d ms] \n\r\r",
                 (int)tb0_cnt,
                 (uint8_t)messageBuffer.size,
                 (uint8_t)setNrOfDegrees,
                 (uint8_t)nrOfDegrees,
                 (uint16_t)TB1CCR1);
        /* Set message in buffer */
        setMessageInBuffer(tempMessage);

        /* Control servo*/
        SG90_setAngle(setNrOfDegrees);
    }
}

/****************************************************************************************
 * ISR`s
 */

/* UART 1 ISR   (USCI_A1_VECTOR) */
#pragma vector=USCI_A1_VECTOR
/****************************************************************************************
 * Func name: USCI_A1_ISR
 * Descr: Implementation of USCI_A1_ISR
 * @params: void
 *
 *
 */
__interrupt void USCI_A1_ISR(void)
{
  /* UART TX Buffer index */
  static uint8_t buff_idx = 0;
  /* Received char */
  char received_char = '\0';
  /* Number digit */
  uint8_t digit = 0;

  switch(__even_in_range(UCA1IV,USCI_UART_UCTXCPTIFG))
  {
    case USCI_NONE: break;
    /* Interrupts from RX */
    case USCI_UART_UCRXIFG:
        UART_COM_handle_UartRxBuff(buff_idx, received_char, digit);
        break;

    /* Interrupts from TX */
    case USCI_UART_UCTXIFG:
        break;
    case USCI_UART_UCSTTIFG: break;
    case USCI_UART_UCTXCPTIFG: break;
    default: break;
  }
}

/* TB0 ISR   (TIMER0_B0_VECTOR) */
#pragma vector = TIMER0_B0_VECTOR
/****************************************************************************************
 * Func name: Timer_B ISR
 * Descr: Implementation of Timer_B ISR
 * @params: void
 *
 *
 */
__interrupt void Timer_B (void)
{
    /* Increase TB0 Counter as program counter */
    tb0_cnt++;

    /* Signal start of message sending */
    P6OUT ^= BIT6;
    /* Check message ready flag */
    if (messageBuffer.ready)
    {
        /* Init UART_TX_BUFFER index */
        int i = 0;
        /* Send data until null ending in the debug message */
        while (messageBuffer.data[i] != '\0')
        {
            /* Wait for no interrupts from eUSCI A1 UART  */
            while (!(UCA1IFG & UCTXIFG));
            UCA1TXBUF = messageBuffer.data[i];
            i++;
        }
        /* Free message buffer and set the transmit flag to false */
        freeMessageBuffer();
    }
    __delay_cycles(10000);
}

/* WDT ISR   (WDT_VECTOR) */
#pragma vector=WDT_VECTOR
/****************************************************************************************
 * Func name: WDT_ISR
 * Descr: Implementation of WDT_ISR
 * @params: void
 *
 *
 */
__interrupt void WDT_ISR(void)
{
    /* Placeholder */
}

/****************************************************************************************
 * END OF ISR`s
 */

/****************************************************************************************
 * DEFINITIONS FOR CALLBACKS
 */

/****************************************************************************************
 * Func name: UART_COM_Callback
 * Descr: Implementation of UART_COM_Callback
 * @params: fptr
 *
 *
 */
void UART_COM_Callback(void(*fptr)(void))
{
    (*fptr)();
}

/****************************************************************************************
 * Func name: ClockSystem_Callback
 * Descr: Implementation of ClockSystem_Callback
 * @params: fptr
 *
 *
 */
void ClockSystem_Callback(void(*fptr)(void))
{
    (*fptr)();
}

/****************************************************************************************
 * Func name: TB_Callback
 * Descr: Implementation of TB_Callback
 * @params: fptr
 *
 *
 */
void TB_Callback(void(*fptr)(void))
{
    (*fptr)();
}

/****************************************************************************************
 * Func name: WDT_Callback
 * Descr: Implementation of WDT_Callback
 * @params: fptr
 *
 *
 */
void WDT_Callback(void(*fptr)(void))
{
    (*fptr)();
}

/****************************************************************************************
 * END OF DEFINITIONS FOR CALLBACKS
 */


/****************************************************************************************
 * DEFINITIONS FOR MODULES CONFIG FUNCTIONS
 */

/****************************************************************************************
 * Func name: WDT_ConfigureWDT
 * Descr: Implementation of WDT_ConfigureWDT
 * @params: void
 *
 *
 */
void WDT_ConfigureWDT()
{
    /*
    * Formula for WDT Timer ISR freq.:
    * SMCLK --> t_clk = 1/f = 1/ 1.000.000  = 1 * 10^-6 = 1 ms
    * Qx = t_int / t_clk  => t_int = Qx * t_clk
    *  => 8.38 = Qx * t_clk
    *  => Qx = 8.38 / t_clk
    *  => Qx = 2 / 1* 10^-6 = 8.388 * 10^6 ~ 2^23
    *  => Qx = 2^23 => WDTIS_2
    */

    /* WDT not STOP; SMCLK ; Interval ; ... ; WDT CLK Source */
    WDTCTL = WDTPW | WDTHOLD_0 | WDTSSEL__SMCLK | WDTTMSEL_1 | WDTCNTCL_1 | WDTIS_2;
}

/****************************************************************************************
 * Func name: TB_ConfigureTimerB0
 * Descr: Implementation of TB_ConfigureTimerB0
 * @params:
 *
 *
 */
void TB_ConfigureTimerB0()
{
    /*
     * TB0.0 --> Freq of messages trough UART_TX_RX
     */
    /* TBCCR0 interrupt enabled */
    TB0CCTL0 |= CCIE;
    /* T = 1s  Timer_B Capture/Compare  Register; Divider fAclk/32768 => 1s delay */
    TB0CCR0 = TB0_CCR0_DIV;
    /* ACLK, UP mode */
    TB0CTL = TBSSEL__ACLK | MC__UP;
}

/****************************************************************************************
 * Func name: TB_ConfigureTimerB1
 * Descr: Implementation of TB_ConfigureTimerB1
 * @params:
 *
 *
 */
void TB_ConfigureTimerB1()
{
    /*
     * TB1.1 --> PWM Control for SG90 Servomotor
     */

    /* Set PWM period to 20000 */
    TB1CCR0 = TB1_CCR0_DIV;
    /* Reset/set mode for CCR1 */
    TB1CCTL1 = OUTMOD_7;
    /* SMCLK, up mode, clear TBR */
    TB1CTL = TBSSEL_2 | MC_1 | TBCLR;
}

/****************************************************************************************
 * Func name: UART_COM_ConfigureUart
 * Descr: Implementation of UART_COM_ConfigureUart
 * @params: none
 *
 *
 */
void UART_COM_ConfigureUart()
{
    /* Configure UART pins; set 2-UART pin as second function */
    P1SEL0 |= BIT6 | BIT7;
    /*  P4.3 -> TxD;P4.2 -> RxD - for Osciloscope; assign TxD si RxD functions */
    P4SEL0 = BIT2 | BIT3;
    /* UART Config
    * 1. eUSCI_Ax Control Word Register 0 --> Select Reset Enable --> UCA1CTLW0 |= UCSWRST
    * 2. eUSCI_Ax Control Word Register 0 --> CLK src: BRCLK = AClk (32.768 Hz) --> UCA1CTLW0 |= UCSSEL_1
    * 3. Set baudrate for 9600 bps: (22.3.10 Setting a Baud Rate User's Guide(MODULE&REGISTER_DESCR).pdf)
    *      - N = F_BRCLK/BAUD_RATE: 32768 / 9600 =~ 3.41
    *      - N > 16 => NO => OS16 = 0 (Oversampling) UCA1BR0 = INT(N) = 0x3 ; UCA1BR1 = 0x00;
    *      - eUSCI_Ax Modulation Control Word Register:
    *          - First 2bytes: UCBRSx = 0x00; 3rd & 4th: UCBRFx (3rd) & OS16(4th) (UCBRSx [15-8] UCBRFx [7-4] OS16 bit [0]
    *          - Frcat(N) = 0.41 -> from the table select: 0x92 --> UCBRSx = 0x92
    *          - ==> UCA1MCTLW = 0x9200
    *
    */
    /* eUSCI_Ax Control Word Register 0 -> SET TO -> Software reset enable */
    UCA1CTLW0 |= UCSWRST;
    /* eUSCI_Ax Control Word Register 0 -> SET TO -> AClk (32768 Hz) */
    UCA1CTLW0 |= UCSSEL_1;
    /* USCI A1 Baud Rate 0 */
    UCA1BR0 = COM_UART_BROption[0];
    /* USCI A1 Baud Rate 1 */
    UCA1BR1 = COM_UART_BROption[1];
    /* eUSCI_Ax Modulation Control Word Register */
    UCA1MCTLW = BR_MSET;
    /* Initialize eUSCI */
    UCA1CTLW0 &= ~UCSWRST;
}

/****************************************************************************************
 * Func name: ClockSystem_ConfigureClockSystem
 * Descr: Implementation of ClockSystem_ConfigureClockSystem
 * @params:
 *
 *
 */
void ClockSystem_ConfigureClockSystem()
{
    /* FRAM Controller Control Register 0 */
    FRCTL0 = FRCTLPW | NWAITS_2;
    /* Disable FLL */
    __bis_SR_register(SCG0);
    /* Frequency taken from REFOCLK si FLLREFDIV = 0 */
    CSCTL3 |= SELREF__REFOCLK | FLLREFDIV_0;
    /* Clean registers DCO si MOD */
    CSCTL0 = 0;
    /* DCOFTRIMEN: Freq.Trim ; ... ; ... ; DCORSEL: Select MCLK Freq */
    CSCTL1 = DCOFTRIMEN_1 | DCOFTRIM0 | DCOFTRIM1 | DCORSEL_5;
    /* Configuration for division and multiplication factor, FLLD = 1 si FLLN = f_mult - 1
     * Calcul: --> f_mult = f_MCLK / F_REFOCLK = 16.000.000 Hz / 32768 Hz = 488-1 = 487
     *
     */
    CSCTL2 = FLLD_0 + CS_DF;
    __delay_cycles(3);
    /* Enable FLL */
    __bic_SR_register(SCG0);
    /* Default DCODIV as MCLK and SMCLK source set default REFO(~32768Hz) as ACLK source, ACLK = 32768Hz */
    CSCTL4 = SELMS__DCOCLKDIV | SELA__REFOCLK;
    /* MCLK = 16MHz si SMCLK = 1MHz */
    CSCTL5 = DIVM__16  | DIVS_0;

}


/****************************************************************************************
 * END OF DEFINITIONS FOR MODULES CONFIG FUNCTIONS
 */

/****************************************************************************************
 * FUNCTION DEFINITIONS
 */

/****************************************************************************************
 * Func name: allocateMessageBuffer
 * Descr: Set a dynamic message template size in the buffer.
 * @param size: The message string to be set as template size.
 */
void allocateMessageBuffer(size_t size)
{
    /* Check if the data is allocated */
    if(messageBuffer.data != NULL)
    {
        /* Clear buffer */
        free(messageBuffer.data);
        messageBuffer.data = NULL;
    }
    /* Allocate data dynamically */
    messageBuffer.data = (char*)malloc(size);
    /* Get the size if the memory is allocated */
    if(messageBuffer.data != NULL)
    {
        messageBuffer.size = size;
        messageBuffer.ready = false;
    }
    else
    {
        messageBuffer.size = 0;
    }
    /* For debug turn on/off led P1.0 to signal data allocation */
    if (messageBuffer.data == NULL)
    {
        /* Turn on LED to signal failure */
        P1OUT ^= BIT0;
    }
    else
    {
        /* Turn off LED to signal success */
        P1OUT &= ~BIT0;
    }
}

/****************************************************************************************
 * Func name: setMessageInBuffer
 * Descr: Set a dynamic message template.
 * @param message: The message string to be set as template.
 */
void setMessageInBuffer(const char* message)
{
    /* No memory allocated -> alloc memory for the message */
    if(messageBuffer.data == NULL)
    {
        allocateMessageBuffer(strlen(message) + 1);
    }

    /* Copy message to buffer and mark last character as NULL ending */
    if(messageBuffer.data != NULL)
    {
        strncpy(messageBuffer.data, message, messageBuffer.size - 1);
        messageBuffer.data[messageBuffer.size - 1] = '\0';
        /* Mark message as ready to be sent trough UART TX Buffer */
        messageBuffer.ready = true;
    }
}

/****************************************************************************************
 * Func name: freeMessageBuffer
 * Descr: Free the memory allocated for the message template.
 * @param: none
 */
void freeMessageBuffer()
{
    if (messageBuffer.data != NULL)
    {
        free(messageBuffer.data);
        messageBuffer.data = NULL;
        messageBuffer.size = 0;
        messageBuffer.ready = false;
        /* Signal end of message sending */
        P6OUT &= ~BIT6;
    }
}

/****************************************************************************************
 * Func name: SG90_Calibration
 * Descr: Initial calibration for SG90_Servo.
 * @param: unsigned int calib_time
 */
void SG90_Calibration(unsigned int calib_time, unsigned int sg90_firstAngle, unsigned int sg90_secondAngle)
{
#if SG90_LONG_CALIB == 1 && SG90_SHRT_CALIB == 0
    int setup_cycle = 3;
    while(setup_cycle--)
    {
        if(setup_cycle == 2)
        {
            /* +90°, -90° și 0° at x second pace */
            TB1CCR1 = SG90_P90DEG; delay_ms(calib_time);
            TB1CCR1 = SG90_N90DEG;  delay_ms(calib_time);
            TB1CCR1 = SG90_0DEG; delay_ms(calib_time);
        }
        else if(setup_cycle == 1)
        {
            /* +45°, -45° și 0° at x second pace */
            TB1CCR1 = SG90_P90DEG - sg90_firstAngle ; delay_ms(calib_time);
            TB1CCR1 = SG90_N90DEG + sg90_firstAngle; delay_ms(calib_time);
            TB1CCR1 = SG90_0DEG; delay_ms(calib_time);
        }
        else
        {
            /* +30°, -30° și 0° at x second pace */
            TB1CCR1 = SG90_P90DEG - sg90_secondAngle; delay_ms(calib_time);
            TB1CCR1 = SG90_N90DEG + sg90_secondAngle; delay_ms(calib_time);
            TB1CCR1 = SG90_0DEG; delay_ms(calib_time);
        }
    }
#elif SG90_SHRT_CALIB == 1 && SG90_LONG_CALIB == 0
    /* 0° at x second pace */
    TB1CCR1 = SG90_0DEG; delay_ms(calib_time);
#endif
}

/****************************************************************************************
 * Func name: delay_ms
 * Descr: Definition for delay_ms function.
 * @param: unsigned int ms
 */
void delay_ms(unsigned int ms)
{
    while (ms--)
    {
        __delay_cycles(1000);
    }
}

/****************************************************************************************
 * Func name: UART_COM_handle_UartRxBuff
 * Descr: Definition for UART_COM_handle_UartRxBuff function. Hndles the angle for servo received form User trough uart rx buff
 * @param: uint8_t buff_idx, char received_char, uint8_t digit
 */
void UART_COM_handle_UartRxBuff(uint8_t buff_idx, char received_char, uint8_t digit)
{
#if UART_HDL_KEYBOARD == 1 && UART_HDL_LABVIEWSERIAL == 0
    /* Input data from UART TX Buffer */
    received_char = UCA1RXBUF;

    /* Check for termination character (e.g., '\n' or '\0' or '\r') */
    if (received_char == '\n' || received_char == '\0' || received_char == '\r')
    {
        /* Store permanent value */
        memcpy((void *)&setNrOfDegrees, (const void *)&nrOfDegrees, sizeof(nrOfDegrees));
        /* Reset the buffer index */
        buff_idx = 0;
        /* Reset the number of degrees value */
        nrOfDegrees = 0;
        /* Clear the buffer */
        memset((void *)&UART_RX_Buff, 0, sizeof(UART_RX_Buff));
    }

    /* If the character is a digit (0-9), process it */
    if (received_char >= '0' && received_char <= '9')
    {
        /* Convert ASCII '0'-'9' to integer 0-9 */
        digit = received_char - '0';
        /* Accumulate the number */
        nrOfDegrees = nrOfDegrees * 10 + digit;

        /* Trim the max nr. of degrees */
        if(nrOfDegrees > 180) nrOfDegrees = 180;

        /* Store the character in the buffer */
        if (buff_idx < 3)
        {
            UART_RX_Buff[buff_idx++] = received_char;
        }
        else
        {
            /* Reset buffer and index if it is full */
            buff_idx = 0;
            memset((void *)&UART_RX_Buff, 0, sizeof(UART_RX_Buff));
            UART_RX_Buff[buff_idx++] = received_char;
        }
    }
    else
    {
        /* Handle invalid character (non-numeric) */
    }

#elif UART_HDL_KEYBOARD == 0 && UART_HDL_LABVIEWSERIAL == 1
    /* Input data from UART TX Buffer */
    received_char = UCA1RXBUF;

    /* Check for termination character (e.g., '\n' or '\0' or '\r') */
    if (received_char == '\n' || received_char == '\0' || received_char == '\r')
    {
        /* Store permanent value */
        memcpy((void *)&setNrOfDegrees, (const void *)&nrOfDegrees, sizeof(nrOfDegrees));
        /* Reset the buffer index */
        buff_idx = 0;
        /* Reset the number of degrees value */
        nrOfDegrees = 0;
        /* Clear the buffer */
        memset((void *)&UART_RX_Buff, 0, sizeof(UART_RX_Buff));
    }

    /* If the character is a digit (0-9), process it */
    if (received_char >= '0' && received_char <= '9')
    {
        /* Convert ASCII '0'-'9' to integer 0-9 */
        digit = received_char - '0';
        /* Accumulate the number */
        nrOfDegrees = nrOfDegrees * 10 + digit;

        /* Trim the max nr. of degrees */
        if(nrOfDegrees > 180) nrOfDegrees = 180;

        /* Store the character in the buffer */
        if ((volatile char*)buff_idx < UART_RX_Buff)
        {
            UART_RX_Buff[buff_idx++] = received_char;
        }
        else
        {
            /* Reset buffer and index if it is full */
            buff_idx = 0;
            memset((void *)&UART_RX_Buff, 0, sizeof(UART_RX_Buff));
            UART_RX_Buff[buff_idx++] = received_char;
        }
    }
    else
    {
        /* Handle invalid character (non-numeric) */
    }

#endif
}



/****************************************************************************************
 * Func name: SG90_setAngle
 * Descr: Definition for SG90_setAngle function. Sets the angle of the servo motor
 * @param: nrOfDegrees
 */
void SG90_setAngle(uint8_t nrOfDegrees)
{
    int deg = nrOfDegrees;

    /* Change deg in interval 90-180 to interval to 0 -90 */
    if(deg >= 0 && deg <= 90)
    {
        /* Set angle */
        TB1CCR1 = SG90_0DEG + deg * SG90_1DEG;
    }
    else if(deg >= 90 && deg <= 180)
    {
        /* Set angle */
        TB1CCR1 = SG90_0DEG - (deg - 90) * SG90_1DEG;
    }
}

/****************************************************************************************
 * END OF FUNCTION DEFINITIONS
 */



