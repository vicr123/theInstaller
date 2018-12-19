if "%APPVEYOR_REPO_TAG_NAME%"=="continuous" (

    exit 1

)

echo Grabbing Qt 5.12 (Static)
curl -L http://downloads.sourceforge.net/project/theinstaller/Qt5.12.0-static.7z > Qt.7z
"/Program Files/7-zip/7z.exe" x Qt.7z -oQtStatic -r

set QTDIR=%cd%\QtStatic\Qt5.12.0-static
set PATH=%PATH%;%QTDIR%\bin
call "C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvars64.bat"

REM Set Qt Paths
echo [Paths] > %QTDIR%\bin\qt.conf
echo Prefix=%QTDIR% >> %QTDIR%\bin\qt.conf

GOTO buildversions

:build
echo *********************************************
echo  BUILDING %~1
echo *********************************************
if not exist "deploy" mkdir deploy
set DEPLOY=%cd%\deploy
echo %~2 > metadata.txt
mkdir %~1
pushd %~1
qmake ..\theInstaller.pro
nmake release
copy installer\release\installer.exe %DEPLOY%\%~1.exe
popd
EXIT /B

:buildversions
CALL :build theSlate "http://vicr123.com/theslate/theinstaller/installer.json"
CALL :build thePhoto "http://vicr123.com/thephoto/theinstaller/installer.json"