# Saltwater

## Requirements:
Running the Editor requires an installed copy of [Qt] and the configured [Qt Visual Studio Add-in]. This generates MOC, UIC and RCC files who are needed building the application.

[Qt]: <https://www.qt.io/>
[Qt Visual Studio Add-in]: <http://doc.qt.io/vs-addin/>

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

## Assets:
Assets depends on the scenes you are creating. Currently an error occurs on runtime because of missing assets. Please keep in mind that these assets has to be loaded by a level file. This level file could be an archive with all asset dependencies inside.

Please contact the administrator and ask for the assets.