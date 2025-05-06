#include <kernel/dpl/DebugP.h>
#include "ti_drivers_config.h"
#include "ti_drivers_open_close.h"
#include "ti_board_open_close.h"
#include <drivers/hw_include/tistdtypes.h>
#include <drivers/mcspi.h>
#include <drivers/gpio.h>

#define ADS7953_CMD(channel) (0x1000 | ((channel & 0x0F) << 7)) // MACRO for generating a command for ADC in Manual Mode

#define I2C_READ_LEN 1
#define BQ25751_I2C_ADDRESS 0x6B

MCSPI_Handle gSpiHandle = NULL;

uint32_t csBaseAddr; // for controlling CS

uint16_t readADC(uint8_t channel);
static void i2c_read_error_handler(int32_t status);


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

void i2c_test(void *args){
    int32_t         status;
    uint32_t        i2cReadTargetAddr;
    uint8_t         rxBuffer[I2C_READ_LEN];
    I2C_Handle      i2cHandle;
    I2C_Transaction i2cTransaction;

    Drivers_open();
    Board_driversOpen();

    i2cReadTargetAddr = BQ25751_I2C_ADDRESS;
    i2cHandle = gI2cHandle[CONFIG_I2C0];

    I2C_Transaction_init(&i2cTransaction);

    i2cTransaction.readBuf      = rxBuffer;
    i2cTransaction.readCount    = I2C_READ_LEN;
    i2cTransaction.targetAddress = i2cReadTargetAddr;

    // status = I2C_transfer(i2cHandle, &i2cTransaction);
    // if(status == I2C_STS_SUCCESS)
    // {
    //     DebugP_log("[I2C] Response: %u\r\n", rxBuffer[0]);
    // }
    // else
    // {
    //     i2c_read_error_handler(i2cTransaction.status);
    // }

    status = I2C_probe(i2cHandle, i2cReadTargetAddr);

    if (status == SystemP_SUCCESS){
        DebugP_log("Device found");
    }
    else{
        i2c_read_error_handler(i2cTransaction.status);
    }

    Board_driversClose();
    Drivers_close();

    return;
}

uint16_t readADC(uint8_t channel){
    uint16_t command = ADS7953_CMD(channel);

    SPI_ReadWrite(command); // Since response comes only in the third package, doing 2 empty reads

    SPI_ReadWrite(command);

    return SPI_ReadWrite(command); // here reading actual data
}

static void i2c_read_error_handler(int32_t status)
{
    switch(status)
    {
        case I2C_STS_ERR:
            DebugP_logError("[I2C] Generic error occurred");
            break;
        case I2C_STS_ERR_TIMEOUT:
            DebugP_logError("[I2C] Timeout error occurred");
            break;
        case I2C_STS_ERR_NO_ACK:
            DebugP_logError("[I2C] No acknowledgement received");
            break;
        case I2C_STS_ERR_ARBITRATION_LOST:
            DebugP_logError("[I2C] Arbitration lost");
            break;
        case I2C_STS_ERR_BUS_BUSY:
            DebugP_logError("[I2C] Bus Bus Busy error occurred");
            break;
    }

    return;
}