@echo OFF
set DEFAULT_ARGS="-DDSLINK_BUILD_EXAMPLES=ON -DDSLINK_BUILD_BROKER=OFF -DCMAKE_BUILD_TYPE=Release"
set LOC="%~p0"
echo "Applying Windows fix pathces"

cd %LOC%\..\deps\wslay\
echo "Applying patch at %cd% location"
git apply %LOC%\..\patch\wslay_patch.txt

cd ..\libuv\
echo "Applying patch at %cd% location"
git apply %LOC%\..\patch\libuv_poll_patch.txt

cd  %LOC%\..
echo "%cd%"
if exist build\CMakeCache.txt (
  echo "exist"
) else (
   rmdir /s /q build
   mkdir build
)

cd build
cmake %DEFAULT_ARGS% ..
echo "Now going to build project solution, Make sure msbuild path is set in system enviornment"
msbuild sdk_dslink_c.sln
echo ".exe file now available on Debug folder"