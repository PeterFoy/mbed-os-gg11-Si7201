#include "mbed.h"
//#include "SILABS_RHT.h"


/*******************************************************************************
 *******************************   DEFINES   ***********************************
 ******************************************************************************/
 
/** @cond DO_NOT_INCLUDE_WITH_DOXYGEN */
 
/** Si7013 Read Temperature Command */
#define SI7013_READ_TEMP_POST   0xE0 /* Read previous T data from RH measurement
                                        command*/
#define SI7013_READ_TEMP        0xE3 /* Stand-alone read temperature command */
 
/** Si7013 Read RH Command */
#define SI7013_READ_RH          0xE5 /* Perform RH (and T) measurement. */
/** Si7013 Read ID */
#define SI7013_READ_ID1_1       0xFA
#define SI7013_READ_ID1_2       0x0F
#define SI7013_READ_ID2_1       0xFc
#define SI7013_READ_ID2_2       0xc9
/** Si7013 Read Firmware Revision */
#define SI7013_READ_FWREV_1     0x84
#define SI7013_READ_FWREV_2     0xB8
 
/** I2C device address for Si7013 */
#define SI7013_ADDR      0x82
/** I2C device address for Si7021 */
#define SI7021_ADDR      0x80
 
 
/** Device ID value for Si7013 */
#define SI7013_DEVICE_ID 0x0D
/** Device ID value for Si7020 */
#define SI7020_DEVICE_ID 0x14
/** Device ID value for Si7021 */
#define SI7021_DEVICE_ID 0x15
 
/** @endcond */
/*******************************************************************************
 *******************************  Variables  ***********************************
 ******************************************************************************/
 
Serial pc(USBTX, USBRX);
DigitalOut myled(LED1);
DigitalOut myled2(LED2);
//LowPowerTicker toggleTicker;
DigitalOut SENS_EN(PB3);
// Read Temperature from Si7021
//I2C i2c(PI4, PI5);
I2C tempSensor(PI4, PI5); //PI4=SDA, PI5=SCL
//I2C tempSensor(PC4, PC5); //PC4=SDA, PC5=SCL on I2C1 

uint8_t  _address = 0;
uint8_t  _rx_buf[8] = {0};
uint8_t  _tx_buf[2] = {0};
 
uint32_t _rhData = 0;
int32_t  _tData = 0;
 
/*******************************************************************************
 *******************************     FUNC    ***********************************
 ******************************************************************************/
 
void readSensor(void) {
    int temp = 0;
    unsigned int humidity = 0;
    
    //send humidity command
    _tx_buf[0] = SI7013_READ_RH;
    tempSensor.write(_address, (char*)_tx_buf, 1);
    tempSensor.read(_address, (char*)_rx_buf, 2);
    
    /* Store raw RH info */
    humidity = ((uint32_t)_rx_buf[0] << 8) + (_rx_buf[1] & 0xFC);
    /* Convert value to milli-percent */
    humidity = (((humidity) * 15625L) >> 13) - 6000;
    
    //send temperature command
    _tx_buf[0] = SI7013_READ_TEMP_POST;
    tempSensor.write(_address, (char*)_tx_buf, 1);
    tempSensor.read(_address, (char*)_rx_buf, 2);
    
    /* Store raw temperature info */
    temp = ((uint32_t)_rx_buf[0] << 8) + (_rx_buf[1] & 0xFC);
    /* Convert to milli-degC */
    temp = (((temp) * 21965L) >> 13) - 46850;
    
    _tData = temp;
    _rhData = humidity;
} 
	  CMU_TypeDef *pCMU;

int main() {
		//#if defined(CMU_HFPERCLKEN0_I2C2)
 //cmuClock_I2C2 = (CMU_NODIV_REG << CMU_DIV_REG_POS) | (CMU_NOSEL_REG << CMU_SEL_REG_POS) | (CMU_HFPERCLKEN0_EN_REG << CMU_EN_REG_POS) | (_CMU_HFPERCLKEN0_I2C2_SHIFT << CMU_EN_BIT_POS)| (CMU_HFPER_CLK_BRANCH << CMU_CLK_BRANCH_POS);
 //  #endif
	
	// Clock Management Unit def in efm32gg11b_cmu.h
    pCMU = (CMU_TypeDef *) CMU_BASE; // Have to type cast to structure pointer
	  pCMU->LOCK |= CMU_LOCK_LOCKKEY_UNLOCK; 
			while ((pCMU->LOCK));  //pCMU->LOCK = 1 when locked 0 when unlocked
	  CMU_ClockDivSet(cmuClock_I2C2, cmuClkDiv_4);
	//pCMU->HFPERPRESCC |= (0x004 << 8); // 9 bits starting at bit 8
		pCMU->LOCK |= CMU_LOCK_LOCKKEY_LOCK; 
	
	//	CMU_ClockDivSet(cmuClock_I2C2, cmuClkDiv_4);
	  CMU_ClockEnable(cmuClock_HFPER, true);
    CMU_ClockEnable(cmuClock_GPIO, true);
   //CMU_ClockEnable(cmuClock_I2C2, true);
	 //CMU_ClockPrescSet(cmuClock_I2C2, 3);
   //CMU_HFRCOFreqSet(cmuHFRCOFreq_19M0Hz);
	
		pc.baud(115200);
		wait(2);
		pc.printf("CMU_HF Clock Freq: %d \r\n", CMU_ClockFreqGet(cmuClock_HF));
	  pc.printf("CMU_I2C2 Clock Freq: %d \r\n", CMU_ClockFreqGet(cmuClock_I2C2));// 50 Mhz needs to be 19 Hz
	  pc.printf("CMU_I2C1 Clock Freq: %d \r\n", CMU_ClockFreqGet(cmuClock_I2C1)); // 50 Mhz needs to be 19 MHz
	
      wait(5);
		//Enable the sensor
    SENS_EN = 1;

    //Check if the sensor is present
    _tx_buf[0] = SI7013_READ_ID2_1;
    _tx_buf[1] = SI7013_READ_ID2_2;
    
    _address = SI7021_ADDR; //TODO: update if we use another sensor
    
    tempSensor.write(_address, (char*)_tx_buf, 2);
    tempSensor.read(_address, (char*)_rx_buf, 8);
    //Check ID byte
    if(_rx_buf[0] != SI7021_DEVICE_ID) {
        pc.printf("No sensor present!\r\n");
        while(1);
    }
    
    while(1) {
        wait(1);
        readSensor();
        pc.printf("Temperature: %d,%3d\r\n", _tData / 1000, _tData % 1000);
    }
}
