#导入库
import wave
import binascii

# 配置WAV文件
channels = 1    # 单声道，即只有一个音频通道
sample_width = 2    # 采样宽度为2字节，即每个采样点占用2个字节的空间
frame_rate = 5715.124   # 采样率为5715.124Hz（通过音频总长度计算得到）
wave_file = wave.open('output.wav', 'wb')   # 以写入模式打开output.wav音频文件
wave_file.setnchannels(channels)    # 设置音频文件的通道数为channels
wave_file.setsampwidth(sample_width)    # 设置每个采样点的宽度为sample_width
wave_file.setframerate(frame_rate)  # 设置音频文件的采样率为frame_rate

# 读取十六进制的音频数据
with open('output.txt', 'r') as file: # 打开文件
    hex_str = file.read()# 字符串形式读取文件

# 去掉文件的首个和末尾数据，为了避免时序对齐时出现的问题
# 同时去掉两个是因为进行字节转换时，每两个字符为一个字节
hex_str = hex_str[2:-2]

# 检查数据的长度，如果是奇数则补0或删除最后一个字符
if len(hex_str) % 2 != 0: # 如果数据长度为奇数
    hex_str = hex_str[:-1] # 删除最后一个字符

# 将解密后的数据转换为二进制数据
byte_data = binascii.unhexlify(hex_str)

# 将文件写入wav文件中
wave_file.writeframes(byte_data)

# 关闭WAV文件
wave_file.close()
print('Finished')