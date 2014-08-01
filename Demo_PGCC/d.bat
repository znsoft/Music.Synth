\pgcc\bin\gcc.exe \temp\d.cpp -laygshell -target=windows -o \temp\d.exe -Wl,-s -fsigned-char -staticlibs 2>\temp\d.log
upx.exe \temp\d.exe
\temp\d.exe
pause
exit
