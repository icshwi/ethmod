/*
 * AKI2C_AT24LC64.h
 *
 *  Created on: May 16, 2016
 *      Author: hinkokocevar
 */

#ifndef _AKI2C_AT24LC64_H_
#define _AKI2C_AT24LC64_H_

#include "AKI2C.h"

#define AKI2C_AT24LC64_MAX_SZ			65536

#define AKI2C_AT24LC64_DataString                     "AKI2C_AT24LC64_DATA"
#define AKI2C_AT24LC64_SizeString                     "AKI2C_AT24LC64_SIZE"
#define AKI2C_AT24LC64_OffsetString                   "AKI2C_AT24LC64_OFFSET"
#define AKI2C_AT24LC64_LengthString                   "AKI2C_AT24LC64_LENGTH"
#define AKI2C_AT24LC64_ReadString                     "AKI2C_AT24LC64_READ"
#define AKI2C_AT24LC64_WriteString                    "AKI2C_AT24LC64_WRITE"

/*
 * Chip       : Atmel/Microchip 24LC64
 * Function   : EEPROM memory
 * Bus        : I2C
 * Access     : TCP/IP socket on AK-NORD XT-PICO-SX
 */
class AKI2C_AT24LC64: public AKI2C {
public:
	AKI2C_AT24LC64(const char *portName, const char *ipPort,
	        int devCount, const char *devAddrs,
			int muxAddr, int muxBus,
			int priority, int stackSize);
	virtual ~AKI2C_AT24LC64();

    /* These are the methods that we override from AKI2C */
    virtual asynStatus readInt8Array(asynUser *pasynUser, epicsInt8 *value,
                                        size_t nElements, size_t *nIn);
    virtual asynStatus writeInt8Array(asynUser *pasynUser, epicsInt8 *value,
                                        size_t nElements);
    void report(FILE *fp, int details);

protected:
    /* Our parameter list */
    int AKI2C_AT24LC64_Data;
#define FIRST_AKI2C_AT24LC64_PARAM AKI2C_AT24LC64_Data
    int AKI2C_AT24LC64_Size;
    int AKI2C_AT24LC64_Offset;
    int AKI2C_AT24LC64_Length;
    int AKI2C_AT24LC64_Read;
    int AKI2C_AT24LC64_Write;
#define LAST_AKI2C_AT24LC64_PARAM AKI2C_AT24LC64_Write

private:
    asynStatus write(int addr, unsigned char *data, unsigned short len, unsigned int off);
    asynStatus read(int addr, unsigned char *data, unsigned short *len, unsigned int off);
};

#define NUM_AKI2C_AT24LC64_PARAMS ((int)(&LAST_AKI2C_AT24LC64_PARAM - &FIRST_AKI2C_AT24LC64_PARAM + 1))

#endif /* _AKI2C_AT24LC64_H_ */
