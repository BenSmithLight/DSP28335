# 导入需要的模块
import serial
import wave
import struct
import time

# 定义串口参数
ser = serial.Serial('COM14', 115200)  # 根据实际串口配置修改

# 定义WAV文件参数
channels = 1  # 单声道
sample_width = 2  # 采样宽度为2字节
frame_rate = 5715.124 # 采样率为44100Hz

# 定义信号数据
st_signal = bytes([0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF])
ed_signal = bytes([0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE])

# 发送开始信号
ser.write(st_signal)
time.sleep(0.1)
ser.write(st_signal)

# 记录开始时间
start = time.time()

# 创建一个空列表用于存储串口读取的数据
data = []

# 循环读取并写入数据，直到超过2秒
while True:
    end = time.time()
    if end - start > 5:
        break
    # 如果有可读数据，就读取并追加到列表中
    if ser.inWaiting():
        data.append(ser.read())

# 发送结束信号
ser.write(ed_signal)
time.sleep(0.1)
ser.write(ed_signal)

# 关闭串口
ser.close()

# 将列表转换为二进制数据
data = b''.join(data)

# 将二进制数据转换为整数列表，每个整数占两个字节，使用小端字节序
# data = struct.unpack('<' + 'h' * (len(data) // sample_width), data)

# # 使用with语句打开原始数据文件，并写入整数列表，使用空格分隔
# with open('origin_data.txt', 'w') as file:
#     for num in data:
#         file.write(str(num) + ' ')

# # 使用with语句打开原始数据文件，并从中读取整数列表，使用空格分隔
# with open('origin_data.txt', 'r') as file:
#     # 使用split方法将字符串分割为列表，并使用map方法将字符串转换为整数
#     data = list(map(int, file.read().split()))

# 使用with语句打开WAV文件，并设置参数和写入音频数据
with wave.open('test.wav', 'wb') as wave_file:
    wave_file.setnchannels(channels)
    wave_file.setsampwidth(sample_width)
    wave_file.setframerate(frame_rate)
    # 将整数列表转换为二进制数据，每个整数占两个字节，使用小端字节序
    # data = struct.pack('<' + 'h' * len(data), *data)
    wave_file.writeframes(data[1:])

print('Finished')