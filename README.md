# AM263Px-testing

# Problems Encountered with ADC Module ADS7953

1. Hardware Chip Select Control </br>
   - It is better to control the Chip Select pin manually by usin GPIO API. That way it is possible to ensure that the right slave device is selected in the right time.

2. Data wires namings</br>
   - SPI_D0 (MOSI) on the microcontroller has to be connected to SDI on the ADC.</br>
   - SPI_D1 (MISO) on the microcontroller has to be connected to SDO on the ADC.</br>
   - Note: in SysConfig Input select should be selected as D1.

 4. SPI configuration</br>
    - Mode 0 (POL0, PHA0) works with ADC</br>
    - Clock speed of 1MHz</br>
    - Chip Select Polarity - Low

5. ADC Powering the board</br>
   - ADC Board should be powered with both Digital and Analog Power.
