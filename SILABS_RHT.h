/***************************************************************************//**
 * @file SILABS_RHT.h
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
 
#ifndef SILABS_RHT_H
#define SILABS_RHT_H
 
#include "platform.h"
#include <mbed.h>
 
typedef void (*cbptr_t)(void);
 
#define SILABS_RHT_ERROR_BUSY       -1
#define SILABS_RHT_ERROR_I2C_BUSY   -2
#define SILABS_RHT_NO_ACTION        -3
#define SILABS_RHT_ERROR_ARGUMENT   -4
#define SILABS_RHT_OK               0
 
typedef enum {
    RHT_IDLE,           // Object initialized
    RHT_ACTIVE,         // Object initialized and sensor available
    RHT_MEASURING,      // In the process of getting an RH measurement
    RHT_TEMPERATURE,    // In the process of reading temperature measurement
    RHT_PINGING,        // In the process of polling the device
    RHT_FWREV           // In the process of reading the device's firmware revision
} SILABS_RHT_state_t;
 
typedef enum {
    si7013,
    si7020,
    si7021
} SILABS_RHT_device_t;
 
namespace silabs {
class SILABS_RHT {
 
public:
 
    SILABS_RHT(I2C * i2c);
 
    /*
     * Get last measured temperature data
     * return: int32_t = temperature in millidegrees centigrade
     */
    int32_t get_temperature();
 
    /*
     * Get last measured relative humidity data
     * return: uint32_t = relative humidity value in milli-percent
     */
    uint32_t get_humidity();
 
    /*
     * Get current state of the sensor, active or inactive. Need to call check_availability first after initialization.
     * return: true if the last operation (check_active or measure) was a success, false if not (device did not respond).
     */
    bool get_active();
 
    /*
     * Perform measurement.
     * Asynchronous callback can be provided (type void (*)(void)).
     * return: 0 if successful, else one of the defined error codes.
     */
    int measure(SILABS_RHT_device_t deviceType, cbptr_t callback = NULL);
 
    /*
     * Check if the sensor is active and responding. This will update the get_active value.
     * Asynchronous callback can be provided (type void (*)(void)).
     * return: 0 if successful, else one of the defined error codes.
     */
    int check_availability(SILABS_RHT_device_t deviceType, cbptr_t callback = NULL);
 
protected:
    mbed::I2C *_i2c;
 
    event_callback_t _internalCallback;
    SILABS_RHT_state_t _state;
    cbptr_t  _completionCallbackPtr;
 
    uint8_t  _address;
    uint8_t  _rx_buf[8];
    uint8_t  _tx_buf[2];
 
    uint32_t _rhData;
    int32_t  _tData;
 
 
 
    /**
     * Callback handler for internal I2C transfers.
     */
    void _cbHandler( int event );
 
    /**
     * Callback handler for internal I2C transfers triggered by timeout.
     */
    void _cbHandlerTimeout( void );
 
    /**
     * Internal lookup table for I2C address by device type.
     */
    uint8_t get_address(SILABS_RHT_device_t device);
};
 
} // namespace silabs
 
#endif //SILABS_RHT_H
