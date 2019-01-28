# Saltwater

## Installation notes:
1. Clone respository
2. Execute setup batch script to download and install dependencies
3. Open Visual Studio project file
4. Change project properties->debugging:
```
Command = "$(SolutionDir)..\..\..\bin\$(TargetFileName)"
```
```
Working Directory = "$(SolutionDir)..\..\..\bin\"
```
Last but not least: Build and Run