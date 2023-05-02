import wave
import binascii
from scipy.io.wavfile import write
import numpy as np

# 配置WAV文件
# channels = 1  # 单声道
# sample_width = 2  # 采样宽度为2字节
# frame_rate = 2298.623 # 采样率为44100Hz
# wave_file = wave.open('test.wav', 'wb')
# wave_file.setnchannels(channels)
# wave_file.setsampwidth(sample_width)
# wave_file.setframerate(frame_rate)

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
