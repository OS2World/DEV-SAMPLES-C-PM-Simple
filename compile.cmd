REM SET C_INCLUDE_PATH=%UNIXROOT%/usr/include;%UNIXROOT%\usr\include\os2tk45\inc;%UNIXROOT%\usr\include\os2tk45\gl;%UNIXROOT%\usr\include\os2tk45;%C_INCLUDE_PATH%
REM SET INCLUDE=%UNIXROOT%/usr/include;%UNIXROOT%\usr\include\os2tk45\inc;%UNIXROOT%\usr\include\os2tk45\gl;%UNIXROOT%\usr\include\os2tk45;
REM SET EMXOMFLD_TYPE=WLINK
REM SET EMXOMFLD_LINKER=wl.exe
make 2>&1 |tee make.out
