@echo off

set BuildDir=build
set RootPath=%cd%
set CodePath=%RootPath%/code

set Options=/nologo /Zi /MTd /EHa-

pushd %BuildDir%
cl %CodePath%/main.cpp %Options% /Fe"main.exe"
popd

