#
# Copyright (c) 2017, Feilong Wang
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# * Redistributions of source code must retain the above copyright notice, this
#   list of conditions and the following disclaimer.
#
# * Redistributions in binary form must reproduce the above copyright notice,
#   this list of conditions and the following disclaimer in the documentation
#   and/or other materials provided with the distribution.
#
# * Neither the name of the copyright holder nor the names of its
#   contributors may be used to endorse or promote products derived from
#   this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
# SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
# OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#

# Set up product
DEBUG ?= false
EXECUTABLE ?= false
CC := gcc
TARGET := cutils

# The default target
.PHONY: default
DEFAULT_GOAL := default
$(DEFAULT_GOAL): $(TARGET)

.PHONY: $(TARGET)
$(TARGET):

.PHONY: FORCE
FORCE:

OUT_DIR := out
TARGET_OUT_ROOT := $(OUT_DIR)
TARGET_OUT := $(TARGET_OUT_ROOT)
TARGET_OUT_INTERMEDIATES := $(TARGET_OUT)/obj

UNAME := $(shell uname -sm)
ifneq (, $(findstring MINGW, $(UNAME)))
	HOST_OS := MINGW
endif
ifneq (, $(findstring Linux, $(UNAME)))
	HOST_OS := Linux
endif
ifneq (, $(findstring Darwin, $(UNAME)))
	HOST_OS := Darwin
endif

ifeq ($(HOST_OS),)
$(error Unable to determine HOST_OS from uname -sm: $(UNAME)!)
endif

SRC_FILES := \
	circular_queue.c

C_INCLUES := \
	include

target_name := $(TARGET)
EX_CFLAGS :=
ifeq ($(EXECUTABLE), true)
	SRC_FILES += main.c
else
	target_name := lib$(TARGET).so
	EX_CFLAGS += -shared -fPIC
endif

ifeq ($(HOST_OS), MINGW)
	EX_CFLAGS += -D__MINGW
endif

ifeq ($(HOST_OS), Linux)
	EX_CFLAGS += -D__LINUX
endif

ifeq ($(HOST_OS), Darwin)
	EX_CFLAGS += -D__DARWIN
endif

ifeq ($(DEBUG), true)
	EX_CFLAGS += -D__DEBUG
endif
# TODO: This flag should only be added in debug mode.
EX_CFLAGS += -g

CFLAGS := -Wall -std=c99 -D_GNU_SOURCE

define transform-c-to-o
@echo "target C: $@ <= $<"
@mkdir -p $(dir $@) 
@$(CC) \
	$(addprefix -I, $(C_INCLUES)) \
	$(CFLAGS) \
	$(EX_CFLAGS) \
	-c \
	-o $@ \
	$<
endef

## Rule to compile a C source file with ../ in the path.
## Must be called with $(eval).
# $(1): the C source file
# $(2): the directory for output object files
# $(3): the variable name to collect the output object files.
define compile-dotdot-c-file
o := $(2)/$(patsubst %.c,%.o,$(subst ../,dotdot/,$(1)))
$$(o) : $(1)
	$$(transform-c-to-o)
$(3) += $$(o)
endef

intermediates := $(TARGET_OUT_INTERMEDIATES)
# For source files starting with ../, we remove all the ../ in the object file path,
# to avoid object file escaping the intermediate directory.
dotdot_sources := $(filter ../%.c, $(SRC_FILES))
dotdot_objects :=
$(foreach s, $(dotdot_sources), \
	$(eval $(call compile-dotdot-c-file, $(s), $(intermediates), dotdot_objects)) \
)
c_normal_sources := $(filter-out ../%,$(filter %.c,$(SRC_FILES)))
c_normal_objects := $(addprefix $(intermediates)/,$(c_normal_sources:.c=.o))

ifneq ($(strip $(c_normal_objects)),)
$(c_normal_objects): $(intermediates)/%.o: %.c
	$(transform-c-to-o)
endif

objs := $(dotdot_objects) $(c_normal_objects)
target_file := $(addprefix $(TARGET_OUT)/, $(target_name))
$(TARGET): $(target_file)
$(target_file): $(objs)
	@if [ $(EXECUTABLE) = "true" ]; then \
		echo "target Executable: $@ <= $^"; \
	else \
		echo "target SharedLib: $@ <= $^"; \
	fi
	@mkdir -p $(dir $@) 
	@$(CC) \
	$(CFLAGS) \
	$(EX_CFLAGS) \
	-o $@ \
	$^ \
	-lpthread -lm
	@echo "\nBuild Successfully!\n"

.PHONY: clean

clean:
	@rm -rf $(OUT_DIR)
	@echo "\nEntire build directory removed.\n"
