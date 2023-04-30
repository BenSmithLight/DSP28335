import re
import wave

# 配置WAV文件
channels = 1  # 单声道
sample_width = 2  # 采样宽度为2字节
frame_rate = 5715.124 # 采样率为44100Hz
wave_file = wave.open('test.wav', 'wb')
wave_file.setnchannels(channels)
wave_file.setsampwidth(sample_width)
wave_file.setframerate(frame_rate)

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

#read txt method one
with open("output.txt", "r") as f:  # 打开文件
    Data_string = f.read()  # 读取文件

Data_match = re.findall(r'ab.*?ef', Data_string)
Data_bytes = b''
for data_ma in Data_match:
    if(len(data_ma) != 10):
        print("数据长度不正确")
    else:
        data = bytes.fromhex(data_ma[2:6])
        result = crc8(data)
        if(data_ma[6:8] == '{:02X}'.format(result).lower()):
            Data_bytes = Data_bytes + data
        else:
            print("CRC 校验失败")

wave_file.writeframes(Data_bytes)

# 关闭WAV文件和串口
wave_file.close()
print('Finished')

