import serial
import wave
import time

ser = serial.Serial('COM14', 115200)  # 根据实际串口配置修改

# 配置WAV文件
channels = 1  # 单声道
sample_width = 2  # 采样宽度为2字节
frame_rate = 5715.124 # 采样率为44100Hz
wave_file = wave.open('output.wav', 'wb')
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
    if end - start > 3:
        break
    data = ser.read(ser.inWaiting())
    Data = Data + data

with open('output.txt', 'w') as file:
    print(Data.hex(), file=file)

ser.write(ed_signal)
time.sleep(0.1)
ser.write(st_signal)
time.sleep(0.1)
ser.close()

wave_file.writeframes(Data[1:])
# wave_file.writeframes(Data)

# 关闭WAV文件和串口
wave_file.close()
print('Finished')
