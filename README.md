# Saltwater

## Installation notes:
1. Clone respository  
2. Fetch large files with Git LFS  
2. Copy DLL files from external to bin folder  
3. Open Visual Studio project file  
4. Change project properties->debugging:  
```
Command = "$(SolutionDir)..\..\..\bin\$(TargetFileName)"
```
```
Working Directory = "$(SolutionDir)..\..\..\bin\"
```
Last but not least: Build and Run  


## Special Android:
These commands are needed for putting data to mobile phone (Android):  
```
adb push data/ /storage/emulated/0/Android/data/de.tuilmenau.saltwater/files
```
```
adb push assets/ /storage/emulated/0/Android/data/de.tuilmenau.saltwater/files
```