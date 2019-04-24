rem Usage: swfcombinedir.bat indir outfile.swf "[swfcombine params, **with quotes**]"
@echo off
SETLOCAL ENABLEDELAYEDEXPANSION
set "fnames="
for %%X in (%1\*.swf) do (
	set "fnames=!fnames! %%X"
)
echo swfcombine.exe -a %~3 %fnames% -o %2
swfcombine.exe -a %~3 %fnames% -o %2