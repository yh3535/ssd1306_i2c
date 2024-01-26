# ssd1306_i2c
This is a library used by Raspberry Pi Pico based on the official example "ssd1306_i2c".  
同时增加了基于GT21L16S2Y SPI接口芯片的汉字驱动程序。  
# How to build
To build this program, you will need pico sdk and the toolchain described in the official documentation. 
After configuring the toolchain and pico sdk, you can run:  
```shell
git clone https://github.com/yh3535/ssd1306_i2c.git
cd ssd1306_i2c
cmake --no-warn-unused-cli -DCMAKE_BUILD_TYPE:STRING=Release -DCMAKE_EXPORT_COMPILE_COMMANDS:BOOL=TRUE -DCMAKE_C_COMPILER:FILEPATH=/usr/bin/arm-none-eabi-gcc -DCMAKE_CXX_COMPILER:FILEPATH=/usr/bin/arm-none-eabi-g++ -S./ -B./build -G Ninja
cmake --build ./build/ --target all
```
Then you'll get the uf2 file in ./build/ .
# License
BSD License