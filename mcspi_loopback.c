#include <kernel/dpl/DebugP.h>
#include "ti_drivers_config.h"
#include "ti_drivers_open_close.h"
#include "ti_board_open_close.h"
#include <drivers/hw_include/tistdtypes.h>
#include <drivers/mcspi.h>
#include <drivers/gpio.h>

#define ADS7953_CMD(channel) (0x1000 | ((channel & 0x0F) << 7)) // MACRO for generating a command for ADC in Manual Mode

MCSPI_Handle gSpiHandle = NULL;

uint32_t csBaseAddr; // for controlling CS

uint16_t readADC(uint8_t channel);


uint16_t SPI_ReadWrite(uint16_t data) {
    MCSPI_Transaction spiTransaction;
    uint16_t txData = data;
    uint16_t rxData = 0;

    MCSPI_Transaction_init(&spiTransaction);
    spiTransaction.channel   = gConfigMcspi1ChCfg[0].chNum;
    spiTransaction.dataSize  = 16;          // 16-bit for ADS7953
    spiTransaction.csDisable = true;       // Manual Control CS
    spiTransaction.count     = 1;
    spiTransaction.txBuf     = (void *)&txData;
    spiTransaction.rxBuf     = (void *)&rxData;
    spiTransaction.args      = NULL;

    GPIO_pinWriteLow(csBaseAddr, CS_GPIO_PIN);

    if (MCSPI_transfer(gSpiHandle, &spiTransaction) != SystemP_SUCCESS) {
        DebugP_log("SPI transfer failed!\r\n");
        return 0xFFFF; // Error value
    }

    GPIO_pinWriteHigh(csBaseAddr, CS_GPIO_PIN);

    return rxData & 0x0FFF; // Getting the value from the package
}

void spi_test(void *args) {
    Drivers_open();
    Board_driversOpen();

    gSpiHandle = gMcspiHandle[CONFIG_MCSPI1];

    csBaseAddr = (uint32_t) AddrTranslateP_getLocalAddr(CS_GPIO_BASE_ADDR);

    GPIO_setDirMode(csBaseAddr, CS_GPIO_PIN, CS_GPIO_DIR);

    uint16_t command = ADS7953_CMD(0);  // Example: Read channel 0

    DebugP_log("%d", command);

    while (true){
        
        uint16_t response1 = readADC(1);
        uint16_t response2 = readADC(2);

        DebugP_log("Channel 1: %d, Channel 2: %d\r\n", response1, response2);
    }

    Board_driversClose();
    Drivers_close();
}

uint16_t readADC(uint8_t channel){
    uint16_t command = ADS7953_CMD(channel);

    SPI_ReadWrite(command); // Since response comes only in the third package, doing 2 empty reads

    SPI_ReadWrite(command);

    return SPI_ReadWrite(command); // here reading actual data
}