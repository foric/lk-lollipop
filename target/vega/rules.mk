LOCAL_DIR := $(GET_LOCAL_DIR)

DEFINES += BUILD_LK

PLATFORM := mt6582

MODULES += \
        dev/keys \
    lib/ptable \
    dev/lcm \

MEMBASE := 0x81E00000 # SDRAM
MEMSIZE := 0x00900000 # 896 KB

SCRATCH_ADDR     := 0x85000000
HAVE_CACHE_PL310 := no
LK_PROFILING := yes
MTK_FASTBOOT_SUPPORT := yes

MACH_TYPE := 6582

INCLUDES += -I$(LOCAL_DIR)/include
INCLUDES += -I$(LOCAL_DIR)/include/target
INCLUDES += -I$(LOCAL_DIR)/inc

DEFINES += \
        MEMBASE=$(MEMBASE)\
	SCRATCH_ADDR=$(SCRATCH_ADDR)\
	MACH_TYPE=$(MACH_TYPE)\
	ENABLE_L2_SHARING

ifeq ($(HAVE_CACHE_PL310), yes)
DEFINES += HAVE_CACHE_PL310
endif

ifeq ($(MTK_FASTBOOT_SUPPORT), yes)
DEFINES += MTK_FASTBOOT_SUPPORT
endif

ifeq ($(LK_PROFILING), yes)
DEFINES += LK_PROFILING
endif

ifeq ($(TARGET_BUILD_VARIANT),user)
DEFINES += USER_BUILD
endif


OBJS += \
        $(LOCAL_DIR)/init.o \
        $(LOCAL_DIR)/cust_msdc.o\
        $(LOCAL_DIR)/cust_display.o\
        $(LOCAL_DIR)/cust_leds.o\
        $(LOCAL_DIR)/power_off.o\
        $(LOCAL_DIR)/fastboot_oem_commands.o\
	
