import serial
import wave
import time

ser = serial.Serial('COM14', 115200)  # 根据实际串口配置修改
over_time = 10  # 设置的时间

# 配置WAV文件
channels = 1  # 单声道
sample_width = 2  # 采样宽度为2字节
frame_rate = 44100  # 采样率为44100 Hz
wave_file = wave.open('test.wav', 'wb')
wave_file.setnchannels(channels)
wave_file.setsampwidth(sample_width)
wave_file.setframerate(frame_rate)

start_time = time.time()
# 读取并写入数据
while True:
    end_time = time.time()
    data = ser.read(ser.inWaiting())
    

    # if end_time - start_time < over_time:
    #     # 从串口读取数据
    #     data = ser.read(ser.inWaiting())

    #     # 将数据写入WAV文件
    #     wave_file.writeframes(data)
    # else:
    #     break

# 关闭WAV文件和串口
wave_file.close()
ser.close()
print('Finished')
