import serial
import time
import re
import binascii
import wave

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

# 配置WAV文件
channels = 1  # 单声道
sample_width = 2  # 采样宽度为2字节
frame_rate = 2280 # 采样率为44100Hz
wave_file = wave.open('output.wav', 'wb')
wave_file.setnchannels(channels)
wave_file.setsampwidth(sample_width)
wave_file.setframerate(frame_rate)

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

with open('data_byte.txt', 'r') as file:
    hex_str = file.read()

hex_str = hex_str[2:-2]

# 检查数据的长度，如果是奇数则补0或删除最后一个字符
if len(hex_str) % 2 != 0:
    hex_str = hex_str[:-1] # 或者 hex_data = hex_data[:-1]

byte_data = binascii.unhexlify(hex_str)

# 将文件写入wav文件中
wave_file.writeframes(byte_data)

# 关闭WAV文件和串口
wave_file.close()
print('Finished')
