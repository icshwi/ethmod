< envPaths
errlogInit(20000)

#cd "${TOP}"

## Register all support components
dbLoadDatabase("$(TOP)/dbd/bpmfeDemoApp.dbd")
bpmfeDemoApp_registerRecordDeviceDriver(pdbbase)

# Prefix for all records
epicsEnvSet("PREFIX", "BPMFE:")
# The port name for the detector
epicsEnvSet("PORT",      "BPMFE")
epicsEnvSet("CMD_PORT",  "BPMFE_CMD")

###
# Create the asyn port to talk to the AK-NORD server on command port 1002.
#drvAsynIPPortConfigure($(CMD_PORT),"192.168.100.100:1002")
# Dummy IP:PORT
drvAsynIPPortConfigure($(CMD_PORT),"localhost:23")
asynSetTraceIOMask($(CMD_PORT),0,255)
asynSetTraceMask($(CMD_PORT),0,255)

# Set the terminators
asynOctetSetOutputEos($(CMD_PORT), 0, "\n")
asynOctetSetInputEos($(CMD_PORT), 0, "\n")

BPMFEConfigure($(PORT), $(CMD_PORT))
dbLoadRecords("$(BPMFE)/db/bpmfe.template",    "P=$(PREFIX),PORT=$(PORT),ADDR=0,TIMEOUT=1,BPMFE_PORT=$(CMD_PORT)")
asynSetTraceIOMask($(PORT),0,255)
asynSetTraceMask($(PORT),0,255)

#set_requestfile_path("./")
#set_requestfile_path("$(BPMFE)/bpmfeApp/Db")
#set_savefile_path("./autosave")
#set_pass0_restoreFile("auto_settings.sav")
#set_pass1_restoreFile("auto_settings.sav")
#save_restoreSet_status_prefix("$(PREFIX)")
#dbLoadRecords("$(AUTOSAVE)/asApp/Db/save_restoreStatus.db", "P=$(PREFIX)")

#cd "${TOP}/iocBoot/${IOC}"
iocInit()

# save things every thirty seconds
#create_monitor_set("${TOP}/iocBoot/${IOC}/auto_settings.req", 30,"P=$(PREFIX)")
