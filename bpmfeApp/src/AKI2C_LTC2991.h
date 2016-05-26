/*
 * AKI2C_LTC2991.h
 *
 *  Created on: May 24, 2016
 *      Author: hinkokocevar
 */

#ifndef _AKI2C_LTC2991_H_
#define _AKI2C_LTC2991_H_

#include "AKI2C.h"

#define AKI2C_LTC2991_STATUS_LOW_REG			0x00
#define AKI2C_LTC2991_STATUS_HIGH_REG			0x01
#define AKI2C_LTC2991_CH_ENABLE_REG				0x01
//#define AKI2C_LTC2991_TRIGGER_REG				0x01
#define AKI2C_LTC2991_CONTROL1_REG				0x06
#define AKI2C_LTC2991_CONTROL2_REG				0x07
#define AKI2C_LTC2991_CONTROL3_REG				0x08
#define AKI2C_LTC2991_V1_MSB_REG				0x0A
#define AKI2C_LTC2991_V1_LSB_REG				0x0B
#define AKI2C_LTC2991_V2_MSB_REG				0x0C
#define AKI2C_LTC2991_V2_LSB_REG				0x0D
#define AKI2C_LTC2991_V3_MSB_REG				0x0E
#define AKI2C_LTC2991_V3_LSB_REG				0x0F
#define AKI2C_LTC2991_V4_MSB_REG				0x10
#define AKI2C_LTC2991_V4_LSB_REG				0x11
#define AKI2C_LTC2991_V5_MSB_REG				0x12
#define AKI2C_LTC2991_V5_LSB_REG				0x13
#define AKI2C_LTC2991_V6_MSB_REG				0x14
#define AKI2C_LTC2991_V6_LSB_REG				0x15
#define AKI2C_LTC2991_V7_MSB_REG				0x16
#define AKI2C_LTC2991_V7_LSB_REG				0x17
#define AKI2C_LTC2991_V8_MSB_REG				0x18
#define AKI2C_LTC2991_V8_LSB_REG				0x19
#define AKI2C_LTC2991_TINT_MSB_REG				0x1A
#define AKI2C_LTC2991_TINT_LSB_REG				0x1B
#define AKI2C_LTC2991_VCC_MSB_REG				0x1C
#define AKI2C_LTC2991_VCC_LSB_REG				0x1D

/* Enable all Vx channels for voltage monitoring,
 * enable Internal temperature and Vcc monitoring */
#define AKI2C_LTC2991_CH_ENABLE_VAL				0xF8
/* Use filtering for Vx channels */
#define AKI2C_LTC2991_CONTROL1_VAL				0x88
#define AKI2C_LTC2991_CONTROL2_VAL				0x88
/* Use filtering for Internal Temperature */
#define AKI2C_LTC2991_CONTROL3_VAL				0x08

#define AKI2C_LTC2991_ReadString                "AKI2C_LTC2991_READ"
#define AKI2C_LTC2991_TriggerString             "AKI2C_LTC2991_TRIGGER"
#define AKI2C_LTC2991_V1ValueString             "AKI2C_LTC2991_V1_VALUE"
#define AKI2C_LTC2991_V1OffsetString            "AKI2C_LTC2991_V1_OFFSET"
#define AKI2C_LTC2991_V1FactorString            "AKI2C_LTC2991_V1_FACTOR"
#define AKI2C_LTC2991_V2ValueString             "AKI2C_LTC2991_V2_VALUE"
#define AKI2C_LTC2991_V2OffsetString            "AKI2C_LTC2991_V2_OFFSET"
#define AKI2C_LTC2991_V2FactorString            "AKI2C_LTC2991_V2_FACTOR"
#define AKI2C_LTC2991_V3ValueString             "AKI2C_LTC2991_V3_VALUE"
#define AKI2C_LTC2991_V3OffsetString            "AKI2C_LTC2991_V3_OFFSET"
#define AKI2C_LTC2991_V3FactorString            "AKI2C_LTC2991_V3_FACTOR"
#define AKI2C_LTC2991_V4ValueString             "AKI2C_LTC2991_V4_VALUE"
#define AKI2C_LTC2991_V4OffsetString            "AKI2C_LTC2991_V4_OFFSET"
#define AKI2C_LTC2991_V4FactorString            "AKI2C_LTC2991_V4_FACTOR"
#define AKI2C_LTC2991_V5ValueString             "AKI2C_LTC2991_V5_VALUE"
#define AKI2C_LTC2991_V5OffsetString            "AKI2C_LTC2991_V5_OFFSET"
#define AKI2C_LTC2991_V5FactorString            "AKI2C_LTC2991_V5_FACTOR"
#define AKI2C_LTC2991_V6ValueString             "AKI2C_LTC2991_V6_VALUE"
#define AKI2C_LTC2991_V6OffsetString            "AKI2C_LTC2991_V6_OFFSET"
#define AKI2C_LTC2991_V6FactorString            "AKI2C_LTC2991_V6_FACTOR"
#define AKI2C_LTC2991_V7ValueString             "AKI2C_LTC2991_V7_VALUE"
#define AKI2C_LTC2991_V7OffsetString            "AKI2C_LTC2991_V7_OFFSET"
#define AKI2C_LTC2991_V7FactorString            "AKI2C_LTC2991_V7_FACTOR"
#define AKI2C_LTC2991_V8ValueString             "AKI2C_LTC2991_V8_VALUE"
#define AKI2C_LTC2991_V8OffsetString            "AKI2C_LTC2991_V8_OFFSET"
#define AKI2C_LTC2991_V8FactorString            "AKI2C_LTC2991_V8_FACTOR"
#define AKI2C_LTC2991_VccValueString            "AKI2C_LTC2991_VCC_VALUE"
#define AKI2C_LTC2991_TIntValueString           "AKI2C_LTC2991_TINT_VALUE"

/*
 * Chip       : Linear LTC2991
 * Function   : voltage / temperature sensor
 * Bus        : I2C
 * Access     : TCP/IP socket on AK-NORD XT-PICO-SX
 */
class AKI2C_LTC2991: public AKI2C {
public:
	AKI2C_LTC2991(const char *portName, const char *ipPort,
	        int devCount, const char *devAddrs,
			int muxAddr, int muxBus,
			int priority, int stackSize);
	virtual ~AKI2C_LTC2991();

    /* These are the methods that we override from AKI2C */
    virtual asynStatus writeInt32(asynUser *pasynUser, epicsInt32 value);
    void report(FILE *fp, int details);
    /* These are new methods */

protected:
    /* Our parameter list */
    int AKI2C_LTC2991_Read;
#define FIRST_AKI2C_LTC2991_PARAM AKI2C_LTC2991_Read
    int AKI2C_LTC2991_Trigger;
    int AKI2C_LTC2991_V1_Value;
    int AKI2C_LTC2991_V1_Factor;
    int AKI2C_LTC2991_V1_Offset;
    int AKI2C_LTC2991_V2_Value;
    int AKI2C_LTC2991_V2_Factor;
    int AKI2C_LTC2991_V2_Offset;
    int AKI2C_LTC2991_V3_Value;
    int AKI2C_LTC2991_V3_Factor;
    int AKI2C_LTC2991_V3_Offset;
    int AKI2C_LTC2991_V4_Value;
    int AKI2C_LTC2991_V4_Factor;
    int AKI2C_LTC2991_V4_Offset;
    int AKI2C_LTC2991_V5_Value;
    int AKI2C_LTC2991_V5_Factor;
    int AKI2C_LTC2991_V5_Offset;
    int AKI2C_LTC2991_V6_Value;
    int AKI2C_LTC2991_V6_Factor;
    int AKI2C_LTC2991_V6_Offset;
    int AKI2C_LTC2991_V7_Value;
    int AKI2C_LTC2991_V7_Factor;
    int AKI2C_LTC2991_V7_Offset;
    int AKI2C_LTC2991_V8_Value;
    int AKI2C_LTC2991_V8_Factor;
    int AKI2C_LTC2991_V8_Offset;
    int AKI2C_LTC2991_Vcc_Value;
    int AKI2C_LTC2991_TInt_Value;
#define LAST_AKI2C_LTC2991_PARAM AKI2C_LTC2991_TInt_Value

private:
    void convertToVoltage(int addr, int valueParam,
    		int offsetParam, int factorParam, unsigned int raw);
    void convertToTemperature(int addr, int valueParam, unsigned int raw);
    asynStatus write(int addr, unsigned char reg, unsigned char val);
    asynStatus read(int addr, unsigned char reg);
};

#define NUM_AKI2C_LTC2991_PARAMS ((int)(&LAST_AKI2C_LTC2991_PARAM - &FIRST_AKI2C_LTC2991_PARAM + 1))

#endif /* _AKI2C_LTC2991_H_ */
