TOP=../..

include $(TOP)/configure/CONFIG
#----------------------------------------
#  ADD MACRO DEFINITIONS AFTER THIS LINE
#=============================

#==================================================
# build a support library

LIBRARY_IOC += bpmfe
PROD_NAME = bpmfe

# install bpmfe.dbd into <top>/dbd
DBD += bpmfe.dbd
$(PROD_NAME)_DBD += bpmfeSupport.dbd

# specify all source files to be compiled and added to the library
bpmfe_SRCS += AKBase.cpp
bpmfe_SRCS += AKI2C.cpp
bpmfe_SRCS += AKI2CTemp.cpp
bpmfe_SRCS += AKI2CEeprom.cpp
bpmfe_SRCS += AKI2CIdNum.cpp
bpmfe_SRCS += AKI2CRTC.cpp
bpmfe_SRCS += AKI2CIOExp.cpp
bpmfe_SRCS += AKI2C_LTC2991.cpp

PROD_LIBS += asyn

bpmfe_LIBS += $(EPICS_BASE_IOC_LIBS)

#===========================

include $(TOP)/configure/RULES
#----------------------------------------
#  ADD RULES AFTER THIS LINE

