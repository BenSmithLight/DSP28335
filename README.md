# DSP28335
Audio capture and serial port transmission.

## 文件构成
C --- DSP中的主函数，分别为不使用CRC、使用CRC和不使用CRC加密三个文件。

Python --- 上位机程序。以audio开头的4个文件中，其中一个为CRC接收程序，另外三个为非CRC接收程序，其中direct表示不输出文件，另外两个输出文件的格式不同。
两个resolve文件表示对时域颠倒的解密和对循环移位的解密程序。

## 使用方法
1. 选择C文件夹下的一个程序，将例程MIC_IN中的main文件替换掉，编译运行
2. 选择对应的Python程序（注意修改串口的编号）。非CRC可以使用三个withoutCRC的Python程序，CRC只能使用一个CRC的Python程序。如果使用加密，则只能使用hex输出的Python程序，配合bit_turn解密程序使用。