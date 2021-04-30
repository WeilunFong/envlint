#!/bin/make

# ------------------------------------------------------------
# Author     : Weilun Fong
# E-mail     : wlf@zhishan-iot.tk
# Description: Makefile for envlint
# ------------------------------------------------------------

utility := envlint

# Define compile parameters
CC     ?= gcc
STRIP  ?= strip
CFLAGS += -Wall -MMD -O2
ifeq ($(ASAN), 1)
    CFLAGS += -fsanitize=address -fsanitize-recover=address
endif
ifeq ($(HAVE_CJSON), 1)
    CFLAGS += -lcjson -DHAVE_CJSON=1
endif

# Print config
ifneq ($(V), 0)
    Q :=
    VECHO := @true
else
    Q := @
    VECHO := @echo
endif

# Define targets
BIN := $(utility)
DEP := $(patsubst %.c, %.d, $(wildcard *.c))
OBJ := $(patsubst %.c, %.o, $(wildcard *.c))
ifeq ($(HAVE_CJSON), 1)
    DEP := $(filter-out cJSON.d, $(DEP))
    OBJ := $(filter-out cJSON.o, $(OBJ))
endif

# Define install path
ifeq ($(PREFIX),)
    PREFIX=/usr
endif

# Start to build
all: $(BIN)

$(BIN) : $(OBJ)
	$(VECHO) "CCLD    $@"
	$(Q)$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $@

%.o: %.c
	$(VECHO) "CC      $@"
	$(Q)$(CC) -c $(CFLAGS) $< -o $@

# Phony
.PHONY: clean help install mostlyclean rebuild strip uninstall

clean:
	rm -f $(BIN) $(OBJ)

help:
	@echo "Usage:"
	@echo "  all          - Build $(utility)"
	@echo "  clean        - Clean output files, except from .d files"
	@echo "  install      - Install $(utility)"
	@echo "  mostlyclean  - Clean all output files"
	@echo "  rebuild      - Rebuild"
	@echo "  uninstall    - Uninstall $(utility)"
	@echo ""
	@echo "Some influential variables:"
	@echo "  ASAN         - 1: enable Address Sanitizer; 0: disable(default: 0)"
	@echo "  HAVE_CJSON   - 1: for runtime with libcjson; 0: for runtime without libcjson(default: 1)"
	@echo ""
	@echo "Report bugs and patch to ytfang<ytfang@fiberhome.com>"

install: $(BIN)
	if [ ! -d $(PREFIX)/bin ]; then mkdir -p $(PREFIX)/bin; fi
	install -m 755 -s $(BIN) $(PREFIX)/bin/

mostlyclean:
	rm -f $(BIN) $(DEP) $(OBJ)

rebuild:
	@make clean && make

strip:
	$(VECHO) "STRIP   $(BIN)"
	$(Q)$(STRIP) $(BIN)

uninstall:
	rm -f $(PREFIX)/bin/$(utility)

-include $(DEP)
