@echo off
rem compile-ow.cmd -- Build simple.exe with OpenWatcom 2.0 for OS/2
rem
rem Requires:
rem   OpenWatcom 2.0 (wcc386, wlink) on the PATH
rem
rem Output: bin-ow\simple.exe
rem Build log: compile-ow.log

if not exist bin-ow mkdir bin-ow

wmake -f makefile-ow clean 2>&1 | tee compile-ow.log
wmake -f makefile-ow 2>&1 | tee compile-ow.log

echo.
echo Done. See compile-ow.log for full output.
