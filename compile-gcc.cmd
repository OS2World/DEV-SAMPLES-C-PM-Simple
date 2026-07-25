@echo off
rem compile-gcc.cmd -- Build simple.exe with GCC 9.2 (bitwiseworks) for OS/2
rem
rem Requires:
rem   GCC 9.2 (bitwiseworks) on the PATH
rem   GNU make (make.exe or gm.exe)
rem
rem Output: bin-gcc\simple.exe
rem Build log: compile-gcc.log

set EMXOMFLD_TYPE=WLINK
set EMXOMFLD_LINKER=wl.exe
set EMXOMFLD_PRELINK=0

if not exist bin-gcc mkdir bin-gcc

make -f makefile-gcc clean 2>&1 | tee compile-gcc.log
make -f makefile-gcc 2>&1 | tee compile-gcc.log

echo.
echo Done. See compile-gcc.log for full output.
