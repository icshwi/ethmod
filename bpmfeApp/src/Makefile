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
bpmfe_SRCS += bpmfe.cpp
bpmfe_SRCS += akNord.cpp

PROD_LIBS += asyn

bpmfe_LIBS += $(EPICS_BASE_IOC_LIBS)

#===========================

include $(TOP)/configure/RULES
#----------------------------------------
#  ADD RULES AFTER THIS LINE

