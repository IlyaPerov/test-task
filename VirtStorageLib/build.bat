md build
cd .\build

cmake ..

cmake --build .

REM Run tests
cd .\tests
ctest