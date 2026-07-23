/**
 * @file SCSerial.h
 * @brief ESP-IDF UART interface for Feetech serial servos
 *
 * Provides the low-level UART communication used by the SCS protocol
 * layer, including initialization, baud-rate configuration, buffered
 * transmission, reception, buffer flushing, and driver cleanup.
 *
 * @see SCS.h
 */

#ifndef SCSERIAL_H
#define SCSERIAL_H

#include "SCS.h"
#include "driver/uart.h"

class SCSerial : public SCS
{
public:
	SCSerial();
	SCSerial(u8 End);
	SCSerial(u8 End, u8 Level);

	// prevent multiple objects from managing the same UART driver
	SCSerial(const SCSerial&) = delete;
	SCSerial& operator=(const SCSerial&) = delete;

protected:
	int writeSCS(unsigned char *nDat, int nLen);// Output nLen bytes
	int readSCS(unsigned char *nDat, int nLen);// Input nLen bytes
	int writeSCS(unsigned char bDat);// Output 1 byte
	void rFlushSCS();//
	void wFlushSCS();//
public:
	unsigned long int IOTimeOut; // receive timeout in milliseconds
	int Err;
public:
	virtual int getErr(){  return Err;  }
	virtual int setBaudRate(int baudRate);
	virtual bool begin(
		int baudRate,
		uart_port_t uartNum,
		int txPin,
		int rxPin
	);
	virtual void end() noexcept;
protected:
	uart_port_t uartNum;
	bool driverInstalled;
	unsigned char txBuf[SCSERVO_BUFFER_SIZE];
	int txBufLen;
};

#endif