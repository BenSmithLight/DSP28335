import serial
import time
import re
import binascii
from scipy.io.wavfile import write
import numpy as np

def crc8(buf):
    buf = bytes.fromhex(buf.hex()) # add this line
    crc = 0
    for b in buf:
        crc ^= b
        for _ in range(8):
            if crc & 0x80:
                crc = ((crc << 1) ^ 0x07) & 0xFF
            else:
                crc <<= 1
    return crc

ser = serial.Serial('COM14', 115200)  # 根据实际串口配置修改

Data = b''  # 用于存储串口读取的数据

st_signal = bytes([0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF])
ed_signal = bytes([0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE])

ser.write(st_signal)
time.sleep(0.1)
ser.write(st_signal)

start = time.time()
# 读取并写入数据
while True:
    end = time.time()
    if end - start > 5:
        break
    data = ser.read(ser.inWaiting())
    Data = Data + data

with open("output.txt", "w") as file:  # 打开文件
    print(Data.hex(), file=file)  # 读取文件

ser.write(ed_signal)
time.sleep(0.1)
ser.write(st_signal)
time.sleep(0.1)
ser.close()

#read txt method one
with open("output.txt", "r") as f:  # 打开文件
    Data_string = f.read()  # 读取文件

Data_match = re.findall(r'ab.*?ef', Data_string)
Data_bytes = b''
for data_ma in Data_match:
    if(len(data_ma) != 10):
        print("数据长度不正确")
        pass
    else:
        data = bytes.fromhex(data_ma[2:6])
        result = crc8(data)
        if(data_ma[6:8] == '{:02X}'.format(result).lower()):
            Data_bytes = Data_bytes + data
        else:
            print("CRC 校验失败")

file = open('data_byte.txt', 'w')
print(Data_bytes.hex(), file=file)
file.close()

# 从 txt 文件中读取数据并转为 bytes 类型
with open('data_byte.txt', 'r') as f:
    data_str = f.read()

# 检查数据的长度，如果是奇数则补0或删除最后一个字符
if len(data_str) % 2 != 0:
    data_str = data_str[:-1] # 或者 hex_data = hex_data[:-1]

data_str = data_str[2:-2]

data_bytes = binascii.unhexlify(data_str)
data_array = np.frombuffer(data_bytes, dtype=np.int16)

write('output.wav', 2280, data_array)
