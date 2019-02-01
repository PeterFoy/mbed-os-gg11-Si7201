/***************************************************************************//**
 * @file SILABS_RHT.cpp
 * @brief Driver class for the Silicon Labs si70xx series I2C RHT sensors
 *******************************************************************************
 * @section License
 * <b>(C) Copyright 2015 Silicon Labs, http://www.silabs.com</b>
 *******************************************************************************
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 * DISCLAIMER OF WARRANTY/LIMITATION OF REMEDIES: Silicon Labs has no
 * obligation to support this Software. Silicon Labs is providing the
 * Software "AS IS", with no express or implied warranties of any kind,
 * including, but not limited to, any implied warranties of merchantability
 * or fitness for any particular purpose or warranties against infringement
 * of any proprietary rights of a third party.
 *
 * Silicon Labs will not be liable for any consequential, incidental, or
 * special damages, or any other relief, or for any claim by any third party,
 * arising from your use of this Software.
 *
 ******************************************************************************/
 
#include <mbed.h>
#include "SILABS_RHT.h"
#include "I2C.h"
 
/*******************************************************************************
 *******************************   DEFINES   ***********************************
 ******************************************************************************/
 
/** @cond DO_NOT_INCLUDE_WITH_DOXYGEN */
 
/** Si7013 Read Temperature Command */
#define SI7013_READ_TEMP        0xE0 /* Read previous T data from RH measurement
                                        command*/
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
 **************************   GLOBAL FUNCTIONS   *******************************
 ******************************************************************************/
 
namespace silabs
{
 
    SILABS_RHT::SILABS_RHT(mbed::I2C * i2c)
    {
        //Save pointer to i2c peripheral
        _i2c = i2c;
        _internalCallback.attach(this, &SILABS_RHT::_cbHandler);
        _completionCallbackPtr = NULL;
 
        _address = 0;
        _rhData = 0;
        _tData = 0;
 
        _state = RHT_IDLE;
    }
 
    /*
     * Get device bus address
     */
    inline uint8_t SILABS_RHT::get_address(SILABS_RHT_device_t device)
    {
        switch(device)
        {
        case si7013:
            return SI7013_ADDR;
        case si7020:
        case si7021:
            return SI7021_ADDR;
        default:
            return 0;
        }
    }
 
    /*
     * Get last measured temperature data
     */
    int32_t SILABS_RHT::get_temperature()
    {
        return _tData;
    }
 
    /*
     * Get last measured relative humidity data
     */
    uint32_t SILABS_RHT::get_humidity()
    {
        return _rhData;
    }
 
    /*
     * Get current state of the sensor, active or inactive. Need to call check_availability first.
     */
    bool SILABS_RHT::get_active()
    {
        if(_state == RHT_IDLE) return false;
        else return true;
    }
 
    /*
     * Perform measurement
     */
    int SILABS_RHT::measure(SILABS_RHT_device_t deviceType, cbptr_t callback)
    {
        /* Check to see we're available */
        if((_state != RHT_IDLE) && (_state != RHT_ACTIVE)) return SILABS_RHT_ERROR_BUSY;
 
        /* Set internal state */
        _state = RHT_MEASURING;
        _completionCallbackPtr = callback;
 
        _tx_buf[0] = SI7013_READ_RH;
 
        /* Get device bus address */
        _address = get_address(deviceType);
        if (_address == 0)
        {
            _state = RHT_IDLE;
            return SILABS_RHT_ERROR_ARGUMENT;
        }
 
        /* Start transfer */
        if(_i2c->transfer(_address, (char*)_tx_buf, 1, (char*)_rx_buf, 2, _internalCallback, I2C_EVENT_ALL) != 0)
        {
            _state = RHT_IDLE;
            return SILABS_RHT_ERROR_I2C_BUSY;
        }
 
        return SILABS_RHT_OK;
    }
 
    /*
     * Check if the sensor is active and responding
     */
    int SILABS_RHT::check_availability(SILABS_RHT_device_t deviceType, cbptr_t callback)
    {
        /* Check to see we're available */
        if((_state != RHT_IDLE) && (_state != RHT_ACTIVE)) return SILABS_RHT_ERROR_BUSY;
 
        /* Set internal state */
        _state = RHT_PINGING;
        _completionCallbackPtr = callback;
 
        _tx_buf[0] = SI7013_READ_ID2_1;
        _tx_buf[1] = SI7013_READ_ID2_2;
 
        /* Get device bus address */
        _address = get_address(deviceType);
        if (_address == 0)
        {
            _state = RHT_IDLE;
            return SILABS_RHT_ERROR_ARGUMENT;
        }
 
        /* Start transfer */
        if(_i2c->transfer(_address, (char*)_tx_buf, 2, (char*)_rx_buf, 8, _internalCallback, I2C_EVENT_ALL) != 0)
        {
            _state = RHT_IDLE;
            return SILABS_RHT_ERROR_I2C_BUSY;
        }
 
        return SILABS_RHT_OK;
    }
 
    void SILABS_RHT::_cbHandlerTimeout( void )
    {
        this->_cbHandler(0);
    }
 
    void SILABS_RHT::_cbHandler( int event )
    {
        if (_state == RHT_IDLE || _state == RHT_ACTIVE)
        {
            return;
        }
        else if (event & I2C_EVENT_ERROR_NO_SLAVE)
        {
            /* Slave is no longer responding */
            _state = RHT_IDLE;
        }
        else if (event & I2C_EVENT_ERROR)
        {
            /* Do nothing when an I2C error occurred */
        }
        else if (_state == RHT_PINGING)
        {
            _state = RHT_ACTIVE;
 
            /* check ID byte */
            switch(_rx_buf[0])
            {
            case SI7013_DEVICE_ID:
            case SI7020_DEVICE_ID:
            case SI7021_DEVICE_ID:
                /* ID byte indicates one of the supported devices */
                break;
            default:
                /* Unsupported device */
                _state = RHT_IDLE;
            }
        }
        else if (_state == RHT_MEASURING)
        {
            /* Store raw RH info */
            _rhData = ((uint32_t)_rx_buf[0] << 8) + (_rx_buf[1] & 0xFC);
            /* Convert value to milli-percent */
            _rhData = (((_rhData) * 15625L) >> 13) - 6000;
 
            /* Read back the temperature value */
            _state = RHT_TEMPERATURE;
 
            _tx_buf[0] = SI7013_READ_TEMP;
 
            if(_i2c->transfer(_address, (char*)_tx_buf, 1, (char*)_rx_buf, 2, _internalCallback, I2C_EVENT_ALL) == 0)
            {
                // Succeeded in adding the transfer, so return here to avoid calling the callback.
                return;
            }
 
            // Did not succeed in adding the temperature transfer, so bail.
            _state = RHT_ACTIVE;
        }
        else if (_state == RHT_TEMPERATURE)
        {
            /* Store raw temperature info */
            _tData = ((uint32_t)_rx_buf[0] << 8) + (_rx_buf[1] & 0xFC);
            /* Convert to milli-degC */
            _tData = (((_tData) * 21965L) >> 13) - 46850;
 
            /* Go back to idling */
            _state = RHT_ACTIVE;
        }
        else if (_state == RHT_FWREV)
        {
            _state = RHT_ACTIVE;
        }
 
        /* If a callback has been registered, call it */
        if (_completionCallbackPtr != NULL)
        {
            _completionCallbackPtr();
            _completionCallbackPtr = NULL;
        }
    }
 
} // namespace silabs