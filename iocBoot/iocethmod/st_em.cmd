< envPaths
errlogInit(20000)

#cd "${TOP}"

## Register all support components
dbLoadDatabase("$(TOP)/dbd/ethmodDemoApp.dbd")
ethmodDemoApp_registerRecordDeviceDriver(pdbbase)
#epicsEnvSet("EPICS_CA_ADDR_LIST", "127.0.0.1")
epicsEnvSet("EPICS_CA_ADDR_LIST", "192.168.1.100 192.168.1.40")
epicsEnvSet("EPICS_CA_AUTO_ADDR_LIST", "NO")
epicsEnvSet("EPICS_CA_MAX_ARRAY_BYTES", "10000000")
# The search path for database files
epicsEnvSet("EPICS_DB_INCLUDE_PATH", "$(ETHMOD)/db")

# Prefix for all records
epicsEnvSet("PREFIX",               "ETHMOD:")
epicsEnvSet("I2C_TMP100_PORT",      "AK_I2C_TMP100")
epicsEnvSet("I2C_AT24LC64_PORT",    "AK_I2C_AT24LC64")
epicsEnvSet("I2C_DS28CM00_PORT",    "AK_I2C_DS28CM00")
epicsEnvSet("I2C_PCF85063TP_PORT",  "AK_I2C_PCF85063TP")
epicsEnvSet("I2C_TCA9555_PORT",     "AK_I2C_TCA9555")
epicsEnvSet("I2C_LTC2991_PORT",     "AK_I2C_LTC2991")
epicsEnvSet("I2C_IP_PORT",          "AK_I2C_COMM")

# Supported IP port types (see AKBase.h)
#AK_IP_PORT_INVALID       0
#AK_IP_PORT_RS232         1
#AK_IP_PORT_RS485         2
#AK_IP_PORT_LCD           3
#AK_IP_PORT_I2C           4
#AK_IP_PORT_SPI           5
#AK_IP_PORT_TTLIO         6
#AK_IP_PORT_SDCARD        7
#AK_IP_PORT_DFCARD        8

###
# I2C
###
# Create the asyn port to talk to the AK-NORD server on command port 1002.
drvAsynIPPortConfigure($(I2C_IP_PORT),"192.168.1.100:1002")
#asynSetTraceIOMask($(I2C_IP_PORT),0,255)
#asynSetTraceMask($(I2C_IP_PORT),0,255)
# Set the terminators
#asynOctetSetOutputEos($(I2C_IP_PORT), 0, "\003")
#asynOctetSetInputEos($(I2C_IP_PORT), 0,  "\003")

# AKI2CTMP100Configure(const char *portName, const char *ipPort,
#        int devCount, const char *devInfos, int priority, int stackSize);
#AKI2CTempConfigure($(I2C_TMP100_PORT), $(I2C_IP_PORT), 8, "0x48 0x49 0x4A 0x4B 0x4C 0x4D 0x4E 0x4F", 0x70, 0, 1, 0, 0)
#AKI2CTMP100Configure($(I2C_TMP100_PORT), $(I2C_IP_PORT), 1, "0x48", 0x73, 3, 1, 0, 0)
#AKI2CTMP100Configure($(I2C_TMP100_PORT), $(I2C_IP_PORT), 2, "0x48 0x49", 0x73, 3, 1, 0, 0)
#AKI2CTMP100Configure($(I2C_TMP100_PORT), $(I2C_IP_PORT), 1, "0x48", 0x73, 3, 1, 0, 0)
#AKI2CTMP100Configure($(I2C_TMP100_PORT), $(I2C_IP_PORT), 8, "0x48 0x49 0x4A 0x4B 0x4C 0x4D 0x4E 0x4F", 0x73, 3, 1, 0, 0)
#AKI2CTMP100Configure($(I2C_TMP100_PORT), $(I2C_IP_PORT), 8, "0x48, 0x73, 3; 0x49, 0x73, 3; 0x4A, 0x73, 3; 0x4B, 0x73, 3; 0x4C, 0x73, 3; 0x4D, 0x73, 3; 0x4E, 0x73, 3; 0x4F, 0x73, 3;", 1, 0, 0)
#AKI2CTMP100Configure($(I2C_TMP100_PORT), $(I2C_IP_PORT), 1, "0x48", 1, 0, 0)
AKI2CTMP100Configure($(I2C_TMP100_PORT), $(I2C_IP_PORT), 1, "0x49", 1, 0, 0)
dbLoadRecords("$(ETHMOD)/db/AKI2C_TMP100.db",       "P=$(PREFIX),R=I2C1:Temp1:,PORT=$(I2C_TMP100_PORT),IP_PORT=$(I2C_IP_PORT),ADDR=0,TIMEOUT=1")
#dbLoadRecords("$(ETHMOD)/db/AKI2C_TMP100.db",       "P=$(PREFIX),R=I2C1:Temp2:,PORT=$(I2C_TMP100_PORT),IP_PORT=$(I2C_IP_PORT),ADDR=1,TIMEOUT=1")
#dbLoadRecords("$(ETHMOD)/db/AKI2C_TMP100.db",       "P=$(PREFIX),R=I2C1:Temp3:,PORT=$(I2C_TMP100_PORT),IP_PORT=$(I2C_IP_PORT),ADDR=2,TIMEOUT=1")
#dbLoadRecords("$(ETHMOD)/db/AKI2C_TMP100.db",       "P=$(PREFIX),R=I2C1:Temp4:,PORT=$(I2C_TMP100_PORT),IP_PORT=$(I2C_IP_PORT),ADDR=3,TIMEOUT=1")
#dbLoadRecords("$(ETHMOD)/db/AKI2C_TMP100.db",       "P=$(PREFIX),R=I2C1:Temp5:,PORT=$(I2C_TMP100_PORT),IP_PORT=$(I2C_IP_PORT),ADDR=4,TIMEOUT=1")
#dbLoadRecords("$(ETHMOD)/db/AKI2C_TMP100.db",       "P=$(PREFIX),R=I2C1:Temp6:,PORT=$(I2C_TMP100_PORT),IP_PORT=$(I2C_IP_PORT),ADDR=5,TIMEOUT=1")
#dbLoadRecords("$(ETHMOD)/db/AKI2C_TMP100.db",       "P=$(PREFIX),R=I2C1:Temp7:,PORT=$(I2C_TMP100_PORT),IP_PORT=$(I2C_IP_PORT),ADDR=6,TIMEOUT=1")
#dbLoadRecords("$(ETHMOD)/db/AKI2C_TMP100.db",       "P=$(PREFIX),R=I2C1:Temp8:,PORT=$(I2C_TMP100_PORT),IP_PORT=$(I2C_IP_PORT),ADDR=7,TIMEOUT=1")
#asynSetTraceIOMask($(I2C_TMP100_PORT),0,255)
#asynSetTraceMask($(I2C_TMP100_PORT),0,255)

# AKI2CAT24LC64Configure(const char *portName, const char *ipPort,
#        int devCount, const char *devInfos, int priority, int stackSize);
#AKI2CAT24LC64Configure($(I2C_AT24LC64_PORT), $(I2C_IP_PORT), 1, "0x51", 0x70, 0, 1, 0, 0)
#dbLoadRecords("$(ETHMOD)/db/AKI2C_AT24LC64.template",     "P=$(PREFIX),R=I2C1:Eeprom1:,PORT=$(I2C_AT24LC64_PORT),IP_PORT=$(I2C_IP_PORT),ADDR=0,TIMEOUT=1,NELM=65536")
#asynSetTraceIOMask($(I2C_AT24LC64_PORT),0,255)
#asynSetTraceMask($(I2C_AT24LC64_PORT),0,255)

# AKI2CDS28CM00Configure(const char *portName, const char *ipPort,
#        int devCount, const char *devInfos, int priority, int stackSize);
#AKI2CDS28CM00Configure($(I2C_DS28CM00_PORT), $(I2C_IP_PORT), 1, "0x50", 0x73, 6, 1, 0, 0)
#AKI2CDS28CM00Configure($(I2C_DS28CM00_PORT), $(I2C_IP_PORT), 1, "0x50, 0x73, 6", 1, 0, 0)
#dbLoadRecords("$(ETHMOD)/db/AKI2C_DS28CM00.db",      "P=$(PREFIX),R=I2C1:IdNum1:,PORT=$(I2C_DS28CM00_PORT),IP_PORT=$(I2C_IP_PORT),ADDR=0,TIMEOUT=1")
#asynSetTraceIOMask($(I2C_DS28CM00_PORT),0,255)
#asynSetTraceMask($(I2C_DS28CM00_PORT),0,255)

# AKI2CPCF85063TPConfigure(const char *portName, const char *ipPort,
#        int devCount, const char *devAddrs,
#        int muxAddr, int muxBus,
#        int priority, int stackSize);
#AKI2CPCF85063TPConfigure($(I2C_PCF85063TP_PORT), $(I2C_IP_PORT), 1, "0x51", 0x70, 0, 1, 0, 0)
#AKI2CPCF85063TPConfigure($(I2C_PCF85063TP_PORT), $(I2C_IP_PORT), 1, "0x51, 0x73, 5", 1, 0, 0)
#dbLoadRecords("$(ETHMOD)/db/AKI2C_PCF85063TP.db",        "P=$(PREFIX),R=I2C1:RTC1:,PORT=$(I2C_PCF85063TP_PORT),IP_PORT=$(I2C_IP_PORT),ADDR=0,TIMEOUT=1")
#asynSetTraceIOMask($(I2C_PCF85063TP_PORT),0,255)
#asynSetTraceMask($(I2C_PCF85063TP_PORT),0,255)

# AKI2CTCA9555Configure(const char *portName, const char *ipPort,
#        int devCount, const char *devInfos, int priority, int stackSize);
#AKI2CTCA9555Configure($(I2C_TCA9555_PORT), $(I2C_IP_PORT), 1, "0x23, 0x73, 1", 1, 0, 0)
#AKI2CTCA9555Configure($(I2C_TCA9555_PORT), $(I2C_IP_PORT), 3, "0x21, 0x73, 7; 0x23, 0x73, 1; 0x25", 1, 0, 0)
#dbLoadRecords("$(ETHMOD)/db/AKI2C_TCA9555.db",        "P=$(PREFIX),R=I2C1:IOExp1:,PORT=$(I2C_TCA9555_PORT),IP_PORT=$(I2C_IP_PORT),ADDR=0,TIMEOUT=1")
#dbLoadRecords("$(ETHMOD)/db/AKI2C_TCA9555.db",        "P=$(PREFIX),R=I2C1:IOExp2:,PORT=$(I2C_TCA9555_PORT),IP_PORT=$(I2C_IP_PORT),ADDR=1,TIMEOUT=1")
#dbLoadRecords("$(ETHMOD)/db/AKI2C_TCA9555.db",        "P=$(PREFIX),R=I2C1:IOExp3:,PORT=$(I2C_TCA9555_PORT),IP_PORT=$(I2C_IP_PORT),ADDR=2,TIMEOUT=1")
#asynSetTraceIOMask($(I2C_TCA9555_PORT),0,255)
#asynSetTraceMask($(I2C_TCA9555_PORT),0,255)

# AKI2CLTC2991Configure(const char *portName, const char *ipPort,
#        int devCount, const char *devInfos, int priority, int stackSize);
AKI2CLTC2991Configure($(I2C_LTC2991_PORT), $(I2C_IP_PORT), 1, "0x48", 0, 0, 1, 0, 0)
#AKI2CLTC2991Configure($(I2C_LTC2991_PORT), $(I2C_IP_PORT), 1, "0x48", 0x73, 0, 1, 0, 0)
#AKI2CLTC2991Configure($(I2C_LTC2991_PORT), $(I2C_IP_PORT), 1, "0x48, 0x73, 0", 1, 0, 0)
dbLoadRecords("$(ETHMOD)/db/AKI2C_LTC2991.db",        "P=$(PREFIX),R=I2C1:VMon1:,PORT=$(I2C_LTC2991_PORT),IP_PORT=$(I2C_IP_PORT),ADDR=0,TIMEOUT=1")
#asynSetTraceIOMask($(I2C_LTC2991_PORT),0,255)
#asynSetTraceMask($(I2C_LTC2991_PORT),0,255)


#set_requestfile_path("./")
#set_requestfile_path("$(ETHMOD)/ethmodApp/Db")
#set_savefile_path("./autosave")
#set_pass0_restoreFile("auto_settings.sav")
#set_pass1_restoreFile("auto_settings.sav")
#save_restoreSet_status_prefix("$(PREFIX)")
#dbLoadRecords("$(AUTOSAVE)/asApp/Db/save_restoreStatus.db", "P=$(PREFIX)")

#cd "${TOP}/iocBoot/${IOC}"
iocInit()

# save things every thirty seconds
#create_monitor_set("${TOP}/iocBoot/${IOC}/auto_settings.req", 30,"P=$(PREFIX)")
