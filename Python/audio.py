import serial
import wave
import time

ser = serial.Serial('COM12', 115200)  # 根据实际串口配置修改

# 配置WAV文件
channels = 1  # 单声道
sample_width = 2  # 采样宽度为2字节
frame_rate = 5715.124 # 采样率为44100Hz
wave_file = wave.open('test.wav', 'wb')
wave_file.setnchannels(channels)
wave_file.setsampwidth(sample_width)
wave_file.setframerate(frame_rate)

# 100字节数据
# 采样宽度2字节   -->  50个音节
# 采样率50Hz  --> 1秒钟采样50个音节,100个字节
# 采样率100Hz  --> 1秒钟采样100个音节,200个字节

# 采样率越高，时间越短
# 10    10.564
# 5410
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

wave_file.writeframes(Data)

# 关闭WAV文件和串口
wave_file.close()
print('Finished')

ser.write(ed_signal)
time.sleep(0.1)
ser.write(st_signal)
time.sleep(0.1)
ser.close()

ser.close()