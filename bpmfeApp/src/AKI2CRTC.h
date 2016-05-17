/*
 * AKI2CRTC.h
 *
 *  Created on: May 17, 2016
 *      Author: hinxx
 */

#ifndef _AKI2CRTC_H_
#define _AKI2CRTC_H_

#include "AKI2C.h"

#define AKI2CRTCDevAddrString                  "AKI2CRTC_DEV_ADDR"
#define AKI2CRTCMuxAddrString                  "AKI2CRTC_MUX_ADDR"
#define AKI2CRTCMuxBusString                   "AKI2CRTC_MUX_BUS"
#define AKI2CRTCSecondsString                  "AKI2CRTC_SECONDS"
#define AKI2CRTCMinutesString                  "AKI2CRTC_MINUTES"
#define AKI2CRTCHoursString                    "AKI2CRTC_HOURS"
#define AKI2CRTCDaysString                     "AKI2CRTC_DAYS"
#define AKI2CRTCWeekDaysString                 "AKI2CRTC_WEEKDAYS"
#define AKI2CRTCMonthsString                   "AKI2CRTC_MONTHS"
#define AKI2CRTCYearsString                    "AKI2CRTC_YEARS"

/** Driver for AK-NORD XT-PICO-SXL I2C real time clock chip access over TCP/IP socket */
class AKI2CRTC: public AKI2C {
public:
	AKI2CRTC(const char *portName, const char *ipPort,
	        int numDevices, int priority, int stackSize);
	virtual ~AKI2CRTC();

    /* These are the methods that we override from AKI2C */
    virtual asynStatus writeInt32(asynUser *pasynUser, epicsInt32 value);
    void report(FILE *fp, int details);
    /* These are new methods */

protected:
    /* Our parameter list */
    int AKI2CRTCDevAddr;
#define FIRST_AKI2CRTC_PARAM AKI2CRTCDevAddr
    int AKI2CRTCMuxAddr;
    int AKI2CRTCMuxBus;
    int AKI2CRTCSeconds;
    int AKI2CRTCMinutes;
    int AKI2CRTCHours;
    int AKI2CRTCDays;
    int AKI2CRTCWeekdays;
    int AKI2CRTCMonths;
    int AKI2CRTCYears;
#define LAST_AKI2CRTC_PARAM AKI2CRTCYears

private:
    asynStatus setDateTime(int addr, unsigned char year, unsigned char month,
    		unsigned char weekday, unsigned char day, unsigned char hour, unsigned char minute,
			unsigned char second);
    asynStatus getDateTime(int addr, unsigned char *year, unsigned char *month,
    		unsigned char *weekday, unsigned char *day, unsigned char *hour, unsigned char *minute,
			unsigned char *second);
};

#define NUM_AKI2CRTC_PARAMS ((int)(&LAST_AKI2CRTC_PARAM - &FIRST_AKI2CRTC_PARAM + 1))

#endif /* _AKI2CRTC_H_ */
