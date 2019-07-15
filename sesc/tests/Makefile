
#

# big-endian
#ARCH=mips
# little-endian
ARCH=mipsel

CC=$(ARCH)-linux-gcc
CPPFLAGS=
CFLAGS=-O
LDFLAGS=-Tmint.x

#BINTYPE=
BINTYPE=.$(ARCH)

BINSUFFIX=.bin$(BINTYPE)

SRCDIRS:=$(shell find . -type d)

SOURCES:=$(foreach dir, $(SRCDIRS), $(wildcard $(dir)/*.c))
TARGETS:=$(SOURCES:.c=$(BINSUFFIX))

all: $(TARGETS)

%.bin.mipsel: %.c
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) -static $< -o $@
%.bin.mips: %.c
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) -static $< -o $@
%.bin: %.c
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) -static $< -o $@

clean:
	rm -f $(TARGETS)
	rm -f $(foreach dir, $(SRCDIRS), $(wildcard $(dir)/a.out))

#
