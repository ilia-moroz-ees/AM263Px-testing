#include <kernel/dpl/DebugP.h>
#include "ti_drivers_config.h"
#include "ti_drivers_open_close.h"
#include "ti_board_open_close.h"
#include <drivers/hw_include/tistdtypes.h>
#include <drivers/mcspi.h>
#include <drivers/gpio.h>

#define ADS7953_CMD(channel) (0x1000 | ((channel & 0x0F) << 7))

MCSPI_Handle gSpiHandle = NULL;

uint32_t csBaseAddr;


uint16_t SPI_ReadWrite(uint16_t data) {
    MCSPI_Transaction spiTransaction;
    uint16_t txData = data;
    uint16_t rxData = 0;

    MCSPI_Transaction_init(&spiTransaction);
    spiTransaction.channel   = gConfigMcspi1ChCfg[0].chNum;
    spiTransaction.dataSize  = 16;          // 16-bit for ADS7953
    spiTransaction.csDisable = true;       // Hardware auto-controls CS
    spiTransaction.count     = 1;
    spiTransaction.txBuf     = (void *)&txData;
    spiTransaction.rxBuf     = (void *)&rxData;
    spiTransaction.args      = NULL;

    GPIO_pinWriteLow(csBaseAddr, CS_GPIO_PIN);
    ClockP_usleep(1000);

    if (MCSPI_transfer(gSpiHandle, &spiTransaction) != SystemP_SUCCESS) {
        DebugP_log("SPI transfer failed!\r\n");
        return 0xFFFF; // Error value
    }
    ClockP_usleep(1000);

    GPIO_pinWriteHigh(csBaseAddr, CS_GPIO_PIN);

    return rxData & 0x0FFF;
}

void mcspi_loopback_main(void *args) {
    Drivers_open();
    Board_driversOpen();

    gSpiHandle = gMcspiHandle[CONFIG_MCSPI1];

    csBaseAddr = (uint32_t) AddrTranslateP_getLocalAddr(CS_GPIO_BASE_ADDR);

    GPIO_setDirMode(csBaseAddr, CS_GPIO_PIN, CS_GPIO_DIR);

    uint16_t command = ADS7953_CMD(5);  // Example: Read channel 0

    DebugP_log("%d", command);

    while (true){
        SPI_ReadWrite(command);
        ClockP_usleep(1000);

        SPI_ReadWrite(command);
        ClockP_usleep(1000);

        uint16_t response = SPI_ReadWrite(command); // here reading actual data

        DebugP_log("Response: %d\r\n", response);
    }

    Board_driversClose();
    Drivers_close();
}