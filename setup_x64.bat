@echo "= Welcome to Saltwater"
@echo "This script download and installs all dependencies for running the application"
@echo "Mode: x64"

@echo off
set BuildMode=x64
@echo on

@echo "Start downloading dependencies..."

@echo off
if not exist "tmp_dependencies_x64_arm64.zip" curl http://data.tobias-schwandt.de/dependencies_x64_arm64.zip --output tmp_dependencies_x64_arm64.zip
@echo on

@echo "Downloading dependencies finished"



@echo "Start unzipping dependencies..."

@echo off
Call :UnZipFile "%cd%\extern\" "%cd%\tmp_dependencies_x64_arm64.zip"

del tmp_dependencies_x64_arm64.zip
@echo on

@echo "Unzipping dependencies finished"



@echo "Copy dynamic dependencies..."

@echo off
if not exist "bin" mkdir bin

copy extern\assimp\lib\%BuildMode%\*.dll bin\
copy extern\devil\lib\%BuildMode%\*.dll bin\
copy extern\easyar\lib\%BuildMode%\*.dll bin\
copy extern\glew\lib\%BuildMode%\*.dll bin\
copy extern\opencv\lib\%BuildMode%\*.dll bin\
copy extern\qt\lib\%BuildMode%\*.dll bin\
copy extern\sdl2\lib\%BuildMode%\*.dll bin\
copy extern\tinyxml2\lib\%BuildMode%\*.dll bin\
copy extern\zlib\lib\%BuildMode%\*.dll bin\
@echo on

@echo "Copying finished"



@echo "All dependencies are installed. Please follow the instructions in README.md"


:UnZipFile <ExtractTo> <newzipfile>
set vbs="%temp%\_.vbs"
if exist %vbs% del /f /q %vbs%
>%vbs%  echo Set fso = CreateObject("Scripting.FileSystemObject")
>>%vbs% echo If NOT fso.FolderExists(%1) Then
>>%vbs% echo fso.CreateFolder(%1)
>>%vbs% echo End If
>>%vbs% echo set objShell = CreateObject("Shell.Application")
>>%vbs% echo set FilesInZip=objShell.NameSpace(%2).items
>>%vbs% echo objShell.NameSpace(%1).CopyHere(FilesInZip)
>>%vbs% echo Set fso = Nothing
>>%vbs% echo Set objShell = Nothing
cscript //nologo %vbs%
if exist %vbs% del /f /q %vbs%