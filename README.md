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