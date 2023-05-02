import wave
import binascii

# 配置WAV文件
channels = 1  # 单声道
sample_width = 2  # 采样宽度为2字节
frame_rate = 5715.124 # 采样率为44100Hz
wave_file = wave.open('output.wav', 'wb')
wave_file.setnchannels(channels)
wave_file.setsampwidth(sample_width)
wave_file.setframerate(frame_rate)

with open('output.txt', 'r') as file:
    hex_str = file.read()

hex_str = hex_str[2:-2]

# 检查数据的长度，如果是奇数则补0或删除最后一个字符
if len(hex_str) % 2 != 0:
    hex_str = hex_str[:-1] # 或者 hex_data = hex_data[:-1]

data_resolve = ""
# 解密
for i in range(0, len(hex_str), 2):
    temp1 = hex_str[i]
    temp2 = hex_str[i+1]
    data_resolve = data_resolve + temp2 + temp1
    
byte_data = binascii.unhexlify(data_resolve)

# 将文件写入wav文件中
wave_file.writeframes(byte_data)

# 关闭WAV文件和串口
wave_file.close()
print('Finished')