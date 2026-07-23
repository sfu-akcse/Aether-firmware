/**
 * @file SCSerial.cpp
 * @brief ESP-IDF UART implementation for Feetech serial servos
 */

#include "SCSerial.h"
#include "freertos/FreeRTOS.h"
#include "esp_err.h"

/**
 * @brief Construct an uninitialized UART interface
 */
SCSerial::SCSerial()
{
	IOTimeOut = 100;
	Err = 0;
	uartNum = UART_NUM_MAX;
	driverInstalled = false;
	txBufLen = 0;
}

/**
 * @brief Constructor with endianness parameter
 * 
 * @param End Endianness flag (0=little-endian, 1=big-endian)
 */
SCSerial::SCSerial(u8 End):SCS(End)
{
	IOTimeOut = 100;
    Err = 0;
    uartNum = UART_NUM_MAX;
    driverInstalled = false;
    txBufLen = 0;
}

/**
 * @brief Constructor with endianness and response level
 * 
 * @param End Endianness flag
 * @param Level Response level
 */
SCSerial::SCSerial(u8 End, u8 Level):SCS(End, Level)
{
	IOTimeOut = 100;
    Err = 0;
    uartNum = UART_NUM_MAX;
    driverInstalled = false;
    txBufLen = 0;
}

/**
 * @brief Initialize the UART interface
 *
 * @param baudRate UART baud rate
 * @param uartNum ESP32 UART controller
 * @param txPin UART transmit GPIO (tx)
 * @param rxPin UART receive GPIO (rx)
 * @return true on success, false on failure
 */
bool SCSerial::begin(int baudRate, uart_port_t uartNum, int txPin, int rxPin)
{
	// validate baud rate
    if (baudRate <= 0 ||
        uartNum < UART_NUM_0 ||
        uartNum >= UART_NUM_MAX) {
        Err = ESP_ERR_INVALID_ARG;
        return false;
    }

	// delete previously installed driver
    if (driverInstalled) {
    	const esp_err_t deleteResult = uart_driver_delete(this->uartNum);

    	if (deleteResult != ESP_OK) {
        	Err = deleteResult;
        	return false;
    	}

    	driverInstalled = false;
    	this->uartNum = UART_NUM_MAX;
    	txBufLen = 0;
	}

	// receive-buffer size
    constexpr int rxBufferSize = 256;

	// install UART driver
    esp_err_t result = uart_driver_install(
        uartNum,
        rxBufferSize,
        0,
        0,
        nullptr,
        0
    );

    if (result != ESP_OK) {
        Err = result;
        return false;
    }

	// configure 8N1
    uart_config_t uartConfig = {};
    uartConfig.baud_rate = baudRate;
    uartConfig.data_bits = UART_DATA_8_BITS;
    uartConfig.parity = UART_PARITY_DISABLE;
    uartConfig.stop_bits = UART_STOP_BITS_1;
    uartConfig.flow_ctrl = UART_HW_FLOWCTRL_DISABLE;

	// apply setitings to  selected UART controller
    result = uart_param_config(uartNum, &uartConfig);

    if (result != ESP_OK) {
        uart_driver_delete(uartNum);
        Err = result;
        return false;
    }

	// connect UART to GPIO
    result = uart_set_pin(
        uartNum,
        txPin,
        rxPin,
        UART_PIN_NO_CHANGE,
        UART_PIN_NO_CHANGE
    );

	// cleanup after failure
    if (result != ESP_OK) {
        uart_driver_delete(uartNum);
        Err = result;
        return false;
    }

	// save successful state
    this->uartNum = uartNum;
    driverInstalled = true;
    txBufLen = 0;
    Err = ESP_OK;

    return true;
}

/**
 * @brief Change the UART baud rate
 *
 * @param baudRate New baud rate
 * @return 1 on success, -1 on failure
 */
int SCSerial::setBaudRate(int baudRate)
{
    if (!driverInstalled) {
        Err = ESP_ERR_INVALID_STATE;
        return -1;
    }

    if (baudRate <= 0) {
        Err = ESP_ERR_INVALID_ARG;
        return -1;
    }

	// use new baud rate and save result
    esp_err_t result = uart_set_baudrate(uartNum, static_cast<uint32_t>(baudRate));

    if (result != ESP_OK) {
        Err = result;
        return -1;
    }

    Err = ESP_OK;
    return 1;
}

/**
 * @brief Read bytes from the UART receive buffer
 *
 * @param nDat Destination buffer
 * @param nLen Maximum number of bytes to read
 * @return Number of bytes read, or -1 on error
 */
int SCSerial::readSCS(unsigned char* nDat, int nLen)
{
    if (!driverInstalled) {
        Err = ESP_ERR_INVALID_STATE;
        return -1;
    }

	// validate the destination
    if (nDat == nullptr || nLen <= 0) { // valid memory buffer or positive # of bytes
        Err = ESP_ERR_INVALID_ARG;
        return -1;
    }

    int totalRead = 0;
    const TickType_t timeoutTicks = pdMS_TO_TICKS(IOTimeOut); // convert ms to freeRTOS ticks

    while (totalRead < nLen) {
        int bytesRead = uart_read_bytes(uartNum, nDat + totalRead, static_cast<uint32_t>(nLen - totalRead), timeoutTicks);

        if (bytesRead < 0) { // handles read failure
            Err = ESP_FAIL;
            return totalRead > 0 ? totalRead : -1; // returns partial byte count
        }

        if (bytesRead == 0) { // timeout expired
            break;
        }

        totalRead += bytesRead;
    }

    Err = ESP_OK;
    return totalRead;
}

/**
 * @brief Write data buffer to transmit buffer
 * 
 * Copies data to internal transmit buffer. Data is sent when wFlushSCS() is called.
 * Includes NULL pointer and buffer overflow protection.
 * 
 * @param nDat Pointer to data to write
 * @param nLen Number of bytes to write
 * @return Current buffer length on success, -1 on error
 */
int SCSerial::writeSCS(unsigned char* nDat, int nLen)
{
    if (nDat == nullptr || nLen <= 0) {
        Err = ESP_ERR_INVALID_ARG;
        return -1;
    }

    if (nLen > SCSERVO_BUFFER_SIZE - txBufLen) {
        Err = ESP_ERR_NO_MEM;
        return -1;
    }

    while (nLen-- > 0) {
        txBuf[txBufLen++] = *nDat++;
    }

    Err = ESP_OK;
    return txBufLen;
}

/**
 * @brief Write single byte to transmit buffer
 * 
 * @param bDat Byte to write
 * @return Current buffer length on success, -1 if buffer full
 */
int SCSerial::writeSCS(unsigned char bDat)
{
    if (txBufLen >= SCSERVO_BUFFER_SIZE) {
        Err = ESP_ERR_NO_MEM;
        return -1;
    }

    txBuf[txBufLen++] = bDat;
    Err = ESP_OK;
    return txBufLen;
}

/**
 * @brief Discard unread UART receive data
 */
void SCSerial::rFlushSCS()
{
    if (!driverInstalled) {
        Err = ESP_ERR_INVALID_STATE;
        return;
    }

    Err = uart_flush_input(uartNum);
}

/**
 * @brief Send all buffered data through UART
 */
void SCSerial::wFlushSCS()
{
    if (!driverInstalled) {
        Err = ESP_ERR_INVALID_STATE;
        return;
    }

    if (txBufLen == 0) { // if buffer is empty
        Err = ESP_OK;
        return;
    }

    const int bytesToWrite = txBufLen; // save packet length

	// send packet
    const int bytesWritten = uart_write_bytes(uartNum, txBuf, static_cast<size_t>(bytesToWrite));

	// verify bytes were accepted
    if (bytesWritten != bytesToWrite) {
        txBufLen = 0;
        Err = ESP_FAIL;
        return;
    }

	// wait until physical transmission finishes
    const esp_err_t result = uart_wait_tx_done(uartNum, pdMS_TO_TICKS(IOTimeOut));

    txBufLen = 0; // clear packet buffer
    Err = result;
}

/**
 * @brief Uninstall the UART driver and reset the interface
 */
void SCSerial::end() noexcept
{
    if (!driverInstalled) {
        return;
    }

    Err = uart_driver_delete(uartNum);

	// reset to initial state after
    if (Err == ESP_OK) {
        driverInstalled = false;
        uartNum = UART_NUM_MAX;
        txBufLen = 0;
    }
}