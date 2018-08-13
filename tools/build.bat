@echo OFF
set DEFAULT_ARGS="-DDSLINK_BUILD_EXAMPLES=ON -DDSLINK_BUILD_BROKER=OFF -DCMAKE_BUILD_TYPE=Release"
set LOC="%cd%"
cd  %LOC%/..
echo "%cd%"
if exist ./build/CMakeCache.txt (
  echo "exist"
) else (
   del build
   mkdir build
)

cd build
cmake %DEFAULT_ARGS% ..
