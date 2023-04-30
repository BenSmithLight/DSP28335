# 导入需要的模块
import wave
import struct

# 定义WAV文件参数
channels = 1  # 单声道
sample_width = 2  # 采样宽度为2字节
frame_rate = 5715.124 # 采样率为44100Hz

# 使用with语句打开原始数据文件，并从中读取整数列表，使用空格分隔
with open('origin_data.txt', 'r') as file:
    # 使用split方法将字符串分割为列表，并使用map方法将字符串转换为整数
    data = list(map(int, file.read().split()))

# 使用with语句打开WAV文件，并设置参数和写入音频数据
with wave.open('test2.wav', 'wb') as wave_file:
    wave_file.setnchannels(channels)
    wave_file.setsampwidth(sample_width)
    wave_file.setframerate(frame_rate)
    # 将整数列表转换为二进制数据，每个整数占两个字节，使用小端字节序
    data = struct.pack('<' + 'h' * len(data), *data)
    # wave_file.writeframes(data[1:])
    wave_file.writeframes(data)

print('Finished')