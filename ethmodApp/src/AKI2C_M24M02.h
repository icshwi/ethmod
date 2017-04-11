/*
 * AKI2C_M24M02.h
 *
 *  Created on: April 10, 2016
 *      Author: hinkokocevar
 */

#ifndef _AKI2C_M24M02_H_
#define _AKI2C_M24M02_H_

#include "AKI2C.h"

#define AKI2C_M24M02_MAX_SZ			65536

#define AKI2C_M24M02_DataString                     "AKI2C_M24M02_DATA"
#define AKI2C_M24M02_SizeString                     "AKI2C_M24M02_SIZE"
#define AKI2C_M24M02_OffsetString                   "AKI2C_M24M02_OFFSET"
#define AKI2C_M24M02_LengthString                   "AKI2C_M24M02_LENGTH"
#define AKI2C_M24M02_ReadString                     "AKI2C_M24M02_READ"
#define AKI2C_M24M02_WriteString                    "AKI2C_M24M02_WRITE"

/*
 * Chip       : ST M24M02
 * Function   : EEPROM memory
 * Bus        : I2C
 * Access     : TCP/IP socket on AK-NORD XT-PICO-SX
 */
class AKI2C_M24M02: public AKI2C {
public:
	AKI2C_M24M02(const char *portName, const char *ipPort,
	        int devCount, const char *devInfos,
			int priority, int stackSize);
	virtual ~AKI2C_M24M02();

    /* These are the methods that we override from AKI2C */
    virtual asynStatus readInt8Array(asynUser *pasynUser, epicsInt8 *value,
                                        size_t nElements, size_t *nIn);
    virtual asynStatus writeInt8Array(asynUser *pasynUser, epicsInt8 *value,
                                        size_t nElements);
    void report(FILE *fp, int details);

protected:
    /* Our parameter list */
    int AKI2C_M24M02_Data;
#define FIRST_AKI2C_M24M02_PARAM AKI2C_M24M02_Data
    int AKI2C_M24M02_Size;
    int AKI2C_M24M02_Offset;
    int AKI2C_M24M02_Length;
    int AKI2C_M24M02_Read;
    int AKI2C_M24M02_Write;
#define LAST_AKI2C_M24M02_PARAM AKI2C_M24M02_Write

private:
    asynStatus write(int addr, unsigned char *data, unsigned short len, unsigned int off);
    asynStatus read(int addr, unsigned char *data, unsigned short *len, unsigned int off);
};

#define NUM_AKI2C_M24M02_PARAMS ((int)(&LAST_AKI2C_M24M02_PARAM - &FIRST_AKI2C_M24M02_PARAM + 1))

#endif /* _AKI2C_M24M02_H_ */
