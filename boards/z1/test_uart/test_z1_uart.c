#include <msp430.h>


typedef unsigned char uint8_t;

#define MSP430_INITIAL_CPU_SPEED   (8000000ul)

#define BAUDRATE    (115200ul)

#define BAUD_RATE_MAJOR   (int)(MSP430_INITIAL_CPU_SPEED / BAUDRATE)
#define BAUD_RATE_MINOR   (int)(((MSP430_INITIAL_CPU_SPEED / BAUDRATE) - BAUD_RATE_MAJOR) * 8)

void clock_init(void)
{
    /* DCO calibration/configuration */
    DCOCTL  = 0x9a;   /* values for 8 MHz */
    BCSCTL1 = 0x0d;

    /* other clock configuration */
    BCSCTL1 |= XT2OFF;    /* XT2 is not connected in Zolertia Z1 */
    BCSCTL2  = 0x00;      /* get MCLK and SMCLK from DCO, without divisor */
    BCSCTL3  = XCAP_1;    /* default value for LFXT1 capacitor and freq. */
}

void uart_init(void)
{
    /* Port 3:
     *  P3.0 is the radio (CC2420) active-low chip select -> output, GPIO, default to Vcc
     *  P3.1 is SPI's MOSI pin -> output, special function, default to GND
     *  P3.2 is SPI's MISO pin -> input, special function, default to GND
     *  P3.3 is SPI's CLK pin -> output, special function, default to GND
     *  P3.4 is USCI0 UART TX pin -> output, special function, default to Vcc
     *  P3.5 is USCI0 UART RX pin -> input, special function, default to Vcc
     *  P3.6 is USCI1 UART TX pin -> output, special function, default to GND
     *  P3.7 is USCI1 UART RX pin -> input, special function, default to GND
     * NOTES :
     *  - Z1 only uses the USCI0 SPI channel
     *  - UART0 is connected to the micro-USB port (via the CP2102 chip)
     */
    P3SEL = 0xFE;    /* Port3 Select: 11111110 = 0xFE */
    P3OUT = 0x31;    /* Port3 Output: 00110001 = 0x31 */
    P3DIR = 0x5B;    /* Port3 Direction: 01011011 = 0x5B */

    /* Port 5:
     *  I2C, and GPIO (LEDs, flash)
     *  P5.0 controls TMP102 power (active high) -> output, GPIO, default to GND
     *  P5.1 is I2C's SDA (data) pin -> input (by default/changeable), special function, default to GND
     *  P5.2 is I2C's SCL (clock) pin -> output, special function, default to GND
     *  P5.3 is not assigned by default
     *  P5.4 controls one of Z1's LEDs (active low) -> output, GPIO, default to Vcc
     *  P5.5 controls one of Z1's LEDs (active low) -> output, GPIO, default to Vcc
     *  P5.6 controls one of Z1's LEDs (active low) -> output, GPIO, default to Vcc
     *  P5.7 is the Flash chip (M25P16) active-low HOLD line -> output, GPIO, default to Vcc
     * NOTES :
     *  - Z1 only uses the USCI1 I2C channel
     *  - P5.3 controls the +5V aux. power regulator in Z1 "starter pack"
     */
    P5SEL = 0x06;    /* Port5 Select: 00000110 = 0x06 */
    P5OUT = 0xF0;    /* Port5 Output: 11110000 = 0xF0 */
    P5DIR = 0xF5;    /* Port5 Direction: 11110101 = 0xF5 */
#define LED_BLUE_ON        P5OUT &=~0x40
#define LED_BLUE_OFF       P5OUT |= 0x40

    UCA0CTL1  = UCSWRST;         /* hold UART module in reset state while we configure it */

/* Add the line "#define UART_SLOW_MODE 1" to use a slower
   (and hopefully more robust) configuration for the UART */
#if UART_SLOW_MODE
    UCA0CTL1 |= UCSSEL_1;        /* source UART's BRCLK from 32768 Hz ACLK  */
    UCA0MCTL  = UCBRS_3;         /* low-frequency baud rate generation,
                                    modulation type 3 */

    /* 9600 baud is the maximum that can be sourced from a 32KiHz clock */
    UCA0BR0   = 3;
    UCA0BR1   = 0;
#else
    UCA0CTL1 |= UCSSEL_2;        /* source UART's BRCLK from 8 MHz SMCLK  */
    UCA0MCTL  = UCBRS_3;         /* low-frequency baud rate generation,
                                    modulation type 4 */

    /* 115200 baud rate (max. recommended for a "classic" serial port) */
    UCA0BR0 = BAUD_RATE_MAJOR;
    UCA0BR1 = BAUD_RATE_MINOR;
#endif

    /* remaining registers : set to default */
    UCA0CTL0  = 0x00;   /* put in asynchronous (== UART) mode, LSB first */
    UCA0STAT  = 0x00;   /* reset status flags */

    /* clear UART-related interrupt flags */
    IFG2 &= ~(UCA0RXIFG | UCA0TXIFG);

    /* configuration done, release reset bit => start UART */
    UCA0CTL1 &= ~UCSWRST;

    /* enable UART0 RX interrupt, disable UART0 TX interrupt */
    IE2 |= UCA0RXIE;
    IE2 &= ~UCA0TXIE;
}

int uart_putchar(int c)
{
    /* wait for a previous transmission to end */
    while ((UCA0STAT & UCBUSY)) {
        __asm__("nop");
    }
    /* load TX byte buffer */
    UCA0TXBUF = (uint8_t) c;
    /* wait for this byte to be transmitted */
    while ((UCA0STAT & UCBUSY)) {
        __asm__("nop");
    }

    return c;
}

void uart_puts(char *s)
{
    do {
        uart_putchar(*s);
    } while (*++s);
}

/* the interrupt handler for UART reception */
__attribute__((__interrupt__(USCIAB0RX_VECTOR))) void usart1irq(void)
{
    int c;

//    enter_isr();
    LED_BLUE_ON;

    /* Check status register for receive errors. */
    if (UCA0STAT & UCRXERR) {
        if (UCA0STAT & UCFE) {
            uart_puts("UART RX framing error\n");
        }
        if (UCA0STAT & UCOE) {
            uart_puts("UART RX overrun error\n");
        }
        if (UCA0STAT & UCPE) {
            uart_puts("UART RX parity error\n");
        }
        if (UCA0STAT & UCBRK) {
            uart_puts("UART RX break condition -> error\n");
        }
        /* Clear error flags by forcing a dummy read. */
        c = UCA0RXBUF;
    } else {
    	/* All went well -> let's signal the reception to adequate callbacks */
    	c = UCA0RXBUF;
    	uart_putchar(c);
    }

    LED_BLUE_OFF;

//    exit_isr();
}

#define STR_BUF_SIZE 10

char *int_to_str(int n, char *sbuf)
{
    sbuf += STR_BUF_SIZE;
    do {
        sbuf--;
        *sbuf = (n % 10) + '0';
        n /= 10;
    } while (n > 0);
    return sbuf;
}


int main(void)
{
    int n = 0;
    char buf[STR_BUF_SIZE + 1];
    buf[STR_BUF_SIZE] = '\0';

    WDTCTL = WDTPW + WDTHOLD;

    clock_init();
    uart_init();
    while (++n) {
        uart_puts("Test UART output #");
        uart_puts(int_to_str(n, buf));
        uart_puts("\r\n");
    }

    return 0;
}
