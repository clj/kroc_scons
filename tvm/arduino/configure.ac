AC_INIT([tvm-arduino], [1.0.0], [kroc-bugs@kent.ac.uk])
AC_CANONICAL_SYSTEM

AM_INIT_AUTOMAKE

AC_PROG_CC
AC_CHECK_TOOL([OBJCOPY], [objcopy], [no])
AC_CHECK_PROG([AVRDUDE], [avrdude], [avrdude], [no])

AC_CHECK_HEADERS([stdio.h stdint.h])

BYTECODE_ADDR=0x5000
F_CPU=16000000
MCU=atmega328p
UPLOAD_PORT=/dev/ttyUSB0
UPLOAD_RATE=57600

AC_ARG_WITH([target-board],
            AC_HELP_STRING([--with-target-board=...],
                           [target board (arduino, sanguino)]),
            [target_board=$withval], [target_board=arduino])
if test "x$target_board" = "xsanguino"; then
	MCU=atmega644p
	UPLOAD_RATE=38400
else
	# Defaults above are suitable for Arduino.
	if test "x$target_board" != "xarduino"; then
		AC_MSG_WARN([unknown target board type; assuming arduino])
	fi
fi

AC_ARG_WITH([bytecode-addr],
            AC_HELP_STRING([--with-bytecode-addr=...],
                           [bytecode load address]),
            [BYTECODE_ADDR=$withval])
AC_ARG_WITH([fcpu],
            AC_HELP_STRING([--with-fcpu=...],
                           [AVR CPU frequency, in Hz]),
            [F_CPU=$withval])
AC_ARG_WITH([mcu],
            AC_HELP_STRING([--with-mcu=...],
                           [AVR MCU type]),
            [MCU=$withval])
AC_ARG_WITH([upload-port],
            AC_HELP_STRING([--with-upload-port=...],
                           [serial port to upload through]),
            [UPLOAD_PORT=$withval])
AC_ARG_WITH([upload-rate],
            AC_HELP_STRING([--with-upload-rate=...],
                           [serial rate to upload at]),
            [UPLOAD_RATE=$withval])

AC_SUBST(BYTECODE_ADDR)
AC_SUBST(F_CPU)
AC_SUBST(MCU)
AC_SUBST(UPLOAD_PORT)
AC_SUBST(UPLOAD_RATE)

CFLAGS="-g -Wall -Os -mmcu=$MCU"

AVRDUDE_FLAGS='-V -F -p $(MCU) -P $(UPLOAD_PORT) -b $(UPLOAD_RATE) -c stk500v1'
AC_SUBST(AVRDUDE_FLAGS)

AC_OUTPUT([Makefile config.mk])
