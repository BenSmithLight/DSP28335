import serial
import wave
import time
import re
import binascii

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

# 配置WAV文件
channels = 1  # 单声道
sample_width = 2  # 采样宽度为2字节
frame_rate = 2298.623 # 采样率为44100Hz
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
    if end - start > 5:
        break
    data = ser.read(ser.inWaiting())
    Data = Data + data

print(Data)
file = open('output.txt', 'w')
print(Data.hex(), file=file)
file.close()

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
        # print("数据长度不正确")
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
# with open('data_byte.txt', 'r') as f:
#     data_str = f.read()

# for i in range(0, len(data_str), 2):
# for i in range(0, 5, 2):
#     data = bytes.fromhex(data_str[i:i+2])
#     print(data)
#     wave_file.writeframes(data)
# print(data_str[0: 10])

# data_bytes = data_str.encode('utf-8')

# # 将二进制数据写入wav文件中
# wave_file.writeframes(data_bytes)

# # 关闭文件
wave_file.close()

# 输出结果： <class 'bytes'> b'Hello World\n'


# txt_file = open('data_byte.txt', 'r')
# # 读取txt文件中的十六进制数据，并转换为二进制数据
# hex_data = txt_file.read()

# # 检查数据的长度，如果是奇数则补0或删除最后一个字符
# if len(hex_data) % 2 != 0:
#     hex_data = hex_data[:-1] # 或者 hex_data = hex_data[:-1]
# bin_data = binascii.unhexlify(hex_data)

# # 将二进制数据写入wav文件中
# wave_file.writeframes(bin_data)

# # 关闭文件
# txt_file.close()
# wave_file.close()

# # 读取txt文件中的十六进制数据
# with open("data_byte.txt", "r") as f:
#     hex_data = f.read()

# # 检查数据的长度，如果是奇数则补0或删除最后一个字符
# if len(hex_data) % 2 != 0:
#     hex_data = hex_data[:-1] # 或者 hex_data = hex_data[:-1]

# # 将十六进制数据转换为字节对象
# byte_data = binascii.unhexlify(hex_data)
# print(byte_data)

# 将列表转换为二进制数据
# data = Data_bytes
# print(type(data))
# data = b''.join(data)

# # 将二进制数据转换为整数列表，每个整数占两个字节，使用小端字节序
# data = struct.unpack('<' + 'h' * (len(data) // sample_width), data)

# # 使用with语句打开原始数据文件，并写入整数列表，使用空格分隔
# with open('origin_data.txt', 'w') as file:
#     for num in data:
#         file.write(str(num) + ' ')

# # 使用with语句打开原始数据文件，并从中读取整数列表，使用空格分隔
# with open('origin_data.txt', 'r') as file:
#     # 使用split方法将字符串分割为列表，并使用map方法将字符串转换为整数
#     data = list(map(int, file.read().split()))

# # 使用with语句打开WAV文件，并设置参数和写入音频数据
# with wave.open('test.wav', 'wb') as wave_file:
#     wave_file.setnchannels(channels)
#     wave_file.setsampwidth(sample_width)
#     wave_file.setframerate(frame_rate)
#     # 将整数列表转换为二进制数据，每个整数占两个字节，使用小端字节序
#     data = struct.pack('<' + 'h' * len(data), *data)
#     wave_file.writeframes(data[1:])

# # 关闭WAV文件和串口
# print('Finished')