/*
 * AKI2CRTC.h
 *
 *  Created on: May 17, 2016
 *      Author: hinxx
 */

#ifndef _AKI2CRTC_H_
#define _AKI2CRTC_H_

#include "AKI2C.h"

#define AKI2CRTCReadString                     "AKI2CRTC_READ"
#define AKI2CRTCWriteString                    "AKI2CRTC_WRITE"
#define AKI2CRTCSecondsString                  "AKI2CRTC_SECONDS"
#define AKI2CRTCMinutesString                  "AKI2CRTC_MINUTES"
#define AKI2CRTCHoursString                    "AKI2CRTC_HOURS"
#define AKI2CRTCDaysString                     "AKI2CRTC_DAYS"
#define AKI2CRTCWeekdaysString                 "AKI2CRTC_WEEKDAYS"
#define AKI2CRTCMonthsString                   "AKI2CRTC_MONTHS"
#define AKI2CRTCYearsString                    "AKI2CRTC_YEARS"
#define AKI2CRTCDateString                     "AKI2CRTC_DATE"
#define AKI2CRTCTimeString                     "AKI2CRTC_TIME"

/** Driver for AK-NORD XT-PICO-SXL I2C real time clock chip access over TCP/IP socket */
class AKI2CRTC: public AKI2C {
public:
	AKI2CRTC(const char *portName, const char *ipPort,
	        int devCount, const char *devAddrs,
			int muxAddr, int muxBus,
			int priority, int stackSize);
	virtual ~AKI2CRTC();

    /* These are the methods that we override from AKI2C */
    virtual asynStatus writeInt32(asynUser *pasynUser, epicsInt32 value);
    void report(FILE *fp, int details);
    /* These are new methods */

protected:
    /* Our parameter list */
    int AKI2CRTCRead;
#define FIRST_AKI2CRTC_PARAM AKI2CRTCRead
    int AKI2CRTCWrite;
    int AKI2CRTCSeconds;
    int AKI2CRTCMinutes;
    int AKI2CRTCHours;
    int AKI2CRTCDays;
    int AKI2CRTCWeekdays;
    int AKI2CRTCMonths;
    int AKI2CRTCYears;
    int AKI2CRTCDate;
    int AKI2CRTCTime;
#define LAST_AKI2CRTC_PARAM AKI2CRTCTime

private:
    asynStatus setDateTime(int addr);
    asynStatus getDateTime(int addr);
};

#define NUM_AKI2CRTC_PARAMS ((int)(&LAST_AKI2CRTC_PARAM - &FIRST_AKI2CRTC_PARAM + 1))

#endif /* _AKI2CRTC_H_ */
