@echo off
set commonCompilerFlags=-nologo -FC -Zi -Gm- -GR- -EHa- -Zo -Oi -Zi -Od

cl %commonCompilerFlags% main.c /link shlwapi.lib

REM.\main.exe ../md_files/ ../