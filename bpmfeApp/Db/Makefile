TOP=../..
include $(TOP)/configure/CONFIG
#----------------------------------------
#  ADD MACRO DEFINITIONS AFTER THIS LINE

#----------------------------------------------------
#  Optimization of db files using dbst (DEFAULT: NO)
#DB_OPT = YES

#----------------------------------------------------
# Create and install (or just install) into <top>/db
# databases, templates, substitutions like this
DB += AKBase.template
DB += AKI2C.template
DB += AKI2C_TMP100.template
DB += AKI2CEeprom.template
DB += AKI2C_DS28CM00.template
DB += AKI2CRTC.template
DB += AKI2C_TCA9555.template
DB += AKI2C_LTC2991.template

#----------------------------------------------------
# If <anyname>.db template is not named <anyname>*.template add
# <anyname>_template = <templatename>

include $(TOP)/configure/RULES
#----------------------------------------
#  ADD RULES AFTER THIS LINE

