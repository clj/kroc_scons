#include "config.h"
#include <cdefBF537.h>
#include "i2cwrite.h"

#define PRESCALE120M 12  // factor = 12, 120MHz/10MHz
#define SSYNC asm("ssync;")

void i2cwrite(unsigned char i2c_device, unsigned char *i2c_data, unsigned int pair_count)
{    
    int i, j;    
    *pTWI_FIFO_CTL |= XMTFLUSH; // Clear the TX FIFO
    *pTWI_MASTER_STAT = BUFWRERR | BUFRDERR | DNAK | ANAK | LOSTARB; // Clear all status error
    SSYNC;
    *pTWI_FIFO_CTL = 0; // Clear the bit manually
    *pTWI_CONTROL = TWI_ENA | PRESCALE120M | SCCB; // PRESCALE = fsclk/10MHz
    *pTWI_CLKDIV = CLKLOW(67) | CLKHI(33);  // For 100KHz SCL speed: CLKDIV = (1/100KHz)/(1/10MHz) = 100
                        //SCL symetric: CLKHI = 50, CLKLOW = 50
    *pTWI_MASTER_ADDR = i2c_device; // Target address (7-bits plus the read/write bit the TWI controls
    for (i = 0; i < pair_count; i++) // # of configurations to send to the sensor
    {
        *pTWI_XMT_DATA8 = *i2c_data++; // Pointer to an array and load a value 
        *pTWI_MASTER_CTL = 0x80 | MEN; // Start transmission of 2 bytes

        while (*pTWI_FIFO_STAT == XMTSTAT) // wait to load the next sample into the TX FIFO
            SSYNC;
            *pTWI_XMT_DATA8 = *i2c_data++; 
            SSYNC;
        while ((*pTWI_INT_STAT & MCOMP) == 0) // Wait until transmission complete and MCOMP is set
            SSYNC;        
            *pTWI_INT_STAT = XMTSERV | MCOMP; // service TWI for next transmission
    }
}

