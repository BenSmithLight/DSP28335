# 导入库
import serial
import wave
import time

# 配置录音时间
record_time = 5

# 配置WAV文件
channels = 1  # 单声道，即只有一个音频通道
sample_width = 2  # 采样宽度为2字节，即每个采样点占用2个字节的空间
frame_rate = 5715.124  # 采样率为5715.124Hz（通过音频总长度计算得到）
wave_file = wave.open('output.wav',
                      'wb')  # 以二进制写入模式打开一个名为output.wav的音频文件，返回一个WAV文件对象
wave_file.setnchannels(channels)  # 设置音频文件的通道数为channels
wave_file.setsampwidth(sample_width)  # 设置每个采样点的宽度为sample_width
wave_file.setframerate(frame_rate)  # 设置音频文件的采样率为frame_rate

# 定义开始信号和结束信号的内容
st_signal = bytes([0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF])
ed_signal = bytes([0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE])

# 打开串口，需要根据具体修改串口编号和波特率
ser = serial.Serial('COM14', 115200)  # 配置串口编号为COM14，波特率为115200

# 发送开始信号
ser.write(st_signal)  # 向串口写入开始信号
time.sleep(0.1)  # 延时0.1秒等待信号发送完成
ser.write(st_signal)  # 再次发送开始信号，以避免DSP的二级缓存未清空

# 读取数据
Data = b''  # 用于存储串口读取的数据
start = time.time()  # 记录开始时间，用于控制录音时间长度
while True:  # 持续接收数据
    end = time.time()  # 刷新当前时间
    if end - start > record_time:  # 控制录制时间
        break  # 超出录音时间则退出循环
    data = ser.read(ser.inWaiting())  # 读取串口缓存中的所有数据
    Data = Data + data  # 将读取到的数据添加到Data中

# 写入读取到的音频数据
with open('output.txt', 'w') as file:  # 打开文件
    print(Data.hex(), file=file)  # 将Data中的数据以十六进制形式写入文件

# 发送结束信号
ser.write(ed_signal)
time.sleep(0.1)  # 延时0.1秒等待信号发送完成
ser.write(st_signal)  # 再次发送开始信号，以避免DSP的二级缓存未清空
time.sleep(0.1)
ser.close()  # 关闭串口

# 将文件写入wav文件中
wave_file.writeframes(Data[1:])
# wave_file.writeframes(Data)  # 此句表示没有时域调转

# 关闭WAV文件
wave_file.close()
print('Finished')
