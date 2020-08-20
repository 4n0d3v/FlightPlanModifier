@echo off
cd "%~dp0"
mkdir FlightPlanModifier-build
for %%A in (FlightPlanModifier) DO (
	mkdir build-tmp
	cd build-tmp
	qmake ..\%%A\%%A.pro
	mingw32-make
	cd ..
	copy build-tmp\release\%%A.exe FlightPlanModifier-build
	rmdir /S /Q build-tmp
	cd FlightPlanModifier-build
	windeployqt %%A.exe --no-translations --no-opengl-sw
	cd ..
)