/*
 * AKI2C_TCA9555.h
 *
 *  Created on: May 19, 2016
 *      Author: hinxx
 */

#ifndef _AKI2C_TCA9555_H_
#define _AKI2C_TCA9555_H_


#include "AKI2C.h"

#define AKI2C_TCA9555_INPUT0_REG				0x00
#define AKI2C_TCA9555_INPUT1_REG				0x01
#define AKI2C_TCA9555_OUTPUT0_REG				0x02
#define AKI2C_TCA9555_OUTPUT1_REG				0x03
#define AKI2C_TCA9555_POLARITY0_REG				0x04
#define AKI2C_TCA9555_POLARITY1_REG				0x05
#define AKI2C_TCA9555_DIRECTION0_REG			0x06
#define AKI2C_TCA9555_DIRECTION1_REG			0x07

#define AKI2C_TCA9555_MAX_PINS					16

#define AKI2C_TCA9555_ReadString                "AKI2C_TCA9555_READ"
#define AKI2C_TCA9555_LevelString               "AKI2C_TCA9555_LEVEL"
#define AKI2C_TCA9555_PolarityString            "AKI2C_TCA9555_POLARITY"
#define AKI2C_TCA9555_DirectionString           "AKI2C_TCA9555_DIRECTION"
#define AKI2C_TCA9555_LevelPin0String           "AKI2C_TCA9555_LEVEL_PIN0"
#define AKI2C_TCA9555_LevelPin1String           "AKI2C_TCA9555_LEVEL_PIN1"
#define AKI2C_TCA9555_LevelPin2String           "AKI2C_TCA9555_LEVEL_PIN2"
#define AKI2C_TCA9555_LevelPin3String           "AKI2C_TCA9555_LEVEL_PIN3"
#define AKI2C_TCA9555_LevelPin4String           "AKI2C_TCA9555_LEVEL_PIN4"
#define AKI2C_TCA9555_LevelPin5String           "AKI2C_TCA9555_LEVEL_PIN5"
#define AKI2C_TCA9555_LevelPin6String           "AKI2C_TCA9555_LEVEL_PIN6"
#define AKI2C_TCA9555_LevelPin7String           "AKI2C_TCA9555_LEVEL_PIN7"
#define AKI2C_TCA9555_LevelPin8String           "AKI2C_TCA9555_LEVEL_PIN8"
#define AKI2C_TCA9555_LevelPin9String           "AKI2C_TCA9555_LEVEL_PIN9"
#define AKI2C_TCA9555_LevelPin10String          "AKI2C_TCA9555_LEVEL_PIN10"
#define AKI2C_TCA9555_LevelPin11String          "AKI2C_TCA9555_LEVEL_PIN11"
#define AKI2C_TCA9555_LevelPin12String          "AKI2C_TCA9555_LEVEL_PIN12"
#define AKI2C_TCA9555_LevelPin13String          "AKI2C_TCA9555_LEVEL_PIN13"
#define AKI2C_TCA9555_LevelPin14String          "AKI2C_TCA9555_LEVEL_PIN14"
#define AKI2C_TCA9555_LevelPin15String          "AKI2C_TCA9555_LEVEL_PIN15"
#define AKI2C_TCA9555_DirPin0String             "AKI2C_TCA9555_DIR_PIN0"
#define AKI2C_TCA9555_DirPin1String             "AKI2C_TCA9555_DIR_PIN1"
#define AKI2C_TCA9555_DirPin2String             "AKI2C_TCA9555_DIR_PIN2"
#define AKI2C_TCA9555_DirPin3String             "AKI2C_TCA9555_DIR_PIN3"
#define AKI2C_TCA9555_DirPin4String             "AKI2C_TCA9555_DIR_PIN4"
#define AKI2C_TCA9555_DirPin5String             "AKI2C_TCA9555_DIR_PIN5"
#define AKI2C_TCA9555_DirPin6String             "AKI2C_TCA9555_DIR_PIN6"
#define AKI2C_TCA9555_DirPin7String             "AKI2C_TCA9555_DIR_PIN7"
#define AKI2C_TCA9555_DirPin8String             "AKI2C_TCA9555_DIR_PIN8"
#define AKI2C_TCA9555_DirPin9String             "AKI2C_TCA9555_DIR_PIN9"
#define AKI2C_TCA9555_DirPin10String            "AKI2C_TCA9555_DIR_PIN10"
#define AKI2C_TCA9555_DirPin11String            "AKI2C_TCA9555_DIR_PIN11"
#define AKI2C_TCA9555_DirPin12String            "AKI2C_TCA9555_DIR_PIN12"
#define AKI2C_TCA9555_DirPin13String            "AKI2C_TCA9555_DIR_PIN13"
#define AKI2C_TCA9555_DirPin14String            "AKI2C_TCA9555_DIR_PIN14"
#define AKI2C_TCA9555_DirPin15String            "AKI2C_TCA9555_DIR_PIN15"

/** Driver for AK-NORD XT-PICO-SXL I2C port expander access over TCP/IP socket */
/*
 * Chip       : TI TCA9555
 * Function   : port expander
 * Bus        : I2C
 * Access     : TCP/IP socket on AK-NORD XT-PICO-SX
 */
class AKI2C_TCA9555: public AKI2C {
public:
	AKI2C_TCA9555(const char *portName, const char *ipPort,
	        int devCount, const char *devInfos,
			int priority, int stackSize);
	virtual ~AKI2C_TCA9555();

    /* These are the methods that we override from AKI2C */
    virtual asynStatus writeInt32(asynUser *pasynUser, epicsInt32 value);
    void report(FILE *fp, int details);
    /* These are new methods */

protected:
    /* Our parameter list */
    int AKI2C_TCA9555_Read;
#define FIRST_AKI2C_TCA9555_PARAM AKI2C_TCA9555_Read
    int AKI2C_TCA9555_Level;
    int AKI2C_TCA9555_Polarity;
    int AKI2C_TCA9555_Direction;
    int AKI2C_TCA9555_LevelPin0;
    int AKI2C_TCA9555_LevelPin1;
    int AKI2C_TCA9555_LevelPin2;
    int AKI2C_TCA9555_LevelPin3;
    int AKI2C_TCA9555_LevelPin4;
    int AKI2C_TCA9555_LevelPin5;
    int AKI2C_TCA9555_LevelPin6;
    int AKI2C_TCA9555_LevelPin7;
    int AKI2C_TCA9555_LevelPin8;
    int AKI2C_TCA9555_LevelPin9;
    int AKI2C_TCA9555_LevelPin10;
    int AKI2C_TCA9555_LevelPin11;
    int AKI2C_TCA9555_LevelPin12;
    int AKI2C_TCA9555_LevelPin13;
    int AKI2C_TCA9555_LevelPin14;
    int AKI2C_TCA9555_LevelPin15;
    int AKI2C_TCA9555_DirPin0;
    int AKI2C_TCA9555_DirPin1;
    int AKI2C_TCA9555_DirPin2;
    int AKI2C_TCA9555_DirPin3;
    int AKI2C_TCA9555_DirPin4;
    int AKI2C_TCA9555_DirPin5;
    int AKI2C_TCA9555_DirPin6;
    int AKI2C_TCA9555_DirPin7;
    int AKI2C_TCA9555_DirPin8;
    int AKI2C_TCA9555_DirPin9;
    int AKI2C_TCA9555_DirPin10;
    int AKI2C_TCA9555_DirPin11;
    int AKI2C_TCA9555_DirPin12;
    int AKI2C_TCA9555_DirPin13;
    int AKI2C_TCA9555_DirPin14;
    int AKI2C_TCA9555_DirPin15;
#define LAST_AKI2C_TCA9555_PARAM AKI2C_TCA9555_DirPin15

private:
    int changeBit(int val, int bit, int level);
    asynStatus read(int addr, unsigned char reg, unsigned short *val);
    asynStatus write(int addr, unsigned char reg, unsigned short val);
    asynStatus writeLevel(int addr, unsigned char param, unsigned short val);
    asynStatus readLevelAll(int addr);
    asynStatus writeDirection(int addr, unsigned char param, unsigned short val);
    asynStatus readDirectionAll(int addr);
};

#define NUM_AKI2C_TCA9555_PARAMS ((int)(&LAST_AKI2C_TCA9555_PARAM - &FIRST_AKI2C_TCA9555_PARAM + 1))

#endif /* _AKI2C_TCA9555_H_ */
