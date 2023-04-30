import serial
import wave
import time
import re

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

ser = serial.Serial('COM12', 115200)  # 根据实际串口配置修改

# 配置WAV文件
channels = 1  # 单声道
sample_width = 2  # 采样宽度为2字节
frame_rate = 5715.124 # 采样率为44100Hz
wave_file = wave.open('test.wav', 'wb')
wave_file.setnchannels(channels)
wave_file.setsampwidth(sample_width)
wave_file.setframerate(frame_rate)

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
    if end - start > 2:
        break
    data = ser.read(ser.inWaiting())
    Data = Data + data

ser.write(ed_signal)
time.sleep(0.1)
ser.write(st_signal)
time.sleep(0.1)
ser.close()

wave_file.writeframes(Data)
# 关闭WAV文件和串口
wave_file.close()
print('Finished')

# file = open('output.txt', 'w')
# print(Data.hex(), file=file)
# file.close()

# ser.write(ed_signal)
# time.sleep(0.1)
# ser.write(st_signal)
# time.sleep(0.1)
# ser.close()

# #read txt method one
# with open("output.txt", "r") as f:  # 打开文件
#     Data_string = f.read()  # 读取文件

# Data_match = re.findall(r'ab.*?ef', Data_string)
# Data_bytes = b''
# for data_ma in Data_match:
#     if(len(data_ma) != 10):
#         print("数据长度不正确")
#     else:
#         data = bytes.fromhex(data_ma[2:6])
#         result = crc8(data)
#         if(data_ma[6:8] == '{:02X}'.format(result).lower()):
#             Data_bytes = Data_bytes + data
#         else:
#             print("CRC 校验失败")

# wave_file.writeframes(Data_bytes)

# file = open('data_byte.txt', 'w')
# print(Data_bytes.hex(), file=file)
# file.close()

# # 关闭WAV文件和串口
# wave_file.close()
# print('Finished')