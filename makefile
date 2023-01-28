# 
# Created on Thu Dec 22 2022 10:23:34 PM
# 
# The MIT License (MIT)
# Copyright (c) 2022 Aananth C N
# 
# Permission is hereby granted, free of charge, to any person obtaining a copy of this software
# and associated documentation files (the "Software"), to deal in the Software without restriction,
# including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
# and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so,
# subject to the following conditions:
# 
# The above copyright notice and this permission notice shall be included in all copies or substantial
# portions of the Software.
# 
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED
# TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
# THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
# TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
# 
CC=${COMPILER}gcc
LD=${COMPILER}ld
AS=${COMPILER}as
AR=${COMPILER}ar
RANLIB=${COMPILER}ranlib
OBJCOPY=${COMPILER}objcopy


include ${ROOT_DIR}/app_paths.mk
include ${ROOT_DIR}/path_defs.mk


INCDIRS  += -I ${NammaTestApp_path}/src \
	    -I ${OS_PATH}/include \
	    -I ${MCU_PATH}/src \
	    -I ${MCU_PATH}/src/common \
	    -I ${MCU_PATH}/src/common/api \
	    -I ${MCU_PATH}/src/common/src \
	    -I ${MCU_STARTUP_PATH} \
	    -I ${OS_BUILDER_PATH}/src \
	    -I ${DIO_PATH}/api \
	    -I ${SPI_PATH}/api \
	    -I ${SPI_PATH}/cfg \
	    -I ${ETH_PATH}/api \
	    -I ${ETH_PATH}/cfg \
	    -I ${ETH_PATH}/src/macphy \
	    -I ${TCPIP_PATH}/api \
	    -I ${TCPIP_PATH}/cfg


$(info  )
$(info compiling NammaTestApp source files)


APP_OBJS := \
	${NammaTestApp_path}/src/namma_test_app.o \
	${NammaTestApp_path}/src/ethernet_test_app.o


LDFLAGS := -g -relocatable
CFLAGS  := -Werror ${INCDIRS} -g
ASFLAGS := ${INCDIRS} -g
TARGET 	:= libNammaTestApp.la
# include c_l_flags.mk to add more definitions specific to micro-controller
include ${ROOT_DIR}/c_l_flags.mk

all: $(TARGET)

LIB_OBJS := $(APP_OBJS)

$(TARGET): $(LIB_OBJS)
	$(LD) ${LDFLAGS} -o $@ $^

clean:
	$(RM) $(LIB_OBJS) $(TARGET)