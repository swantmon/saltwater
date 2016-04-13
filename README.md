# Saltwater

## Installation notes:
1. Clone respository
2. Download dependencies from download section
3. Unzip package into the root folder
4. Create a 'bin' folder in the root folder
5. Execute preparation file depending on your system
6. Open Visual Studio project file
7. Change project properties->debugging:
```
Command = "$(SolutionDir)..\..\..\bin\$(TargetFileName)"
```
```
Working Directory = "$(SolutionDir)..\..\..\bin\"
```
Last but not least: Build and Run

## Assets:
Assets depends on the scenes you are creating. Currently an error occurs on runtime because of missing assets. Please keep in mind that these assets has to be loaded by a level file. This level file could be an archive with all asset dependencies inside.

However, the current version has no support for loading assets and level description from file. Please contact the administrator and ask for the assets.