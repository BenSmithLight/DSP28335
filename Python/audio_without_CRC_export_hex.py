# 导入库
import serial  # 导入串口通信库
import wave  # 导入串口通信库
import time  # 导入时间库
import binascii  # 导入二进制转换库

# 打开串口，需要根据具体修改串口编号和波特率
ser = serial.Serial('COM14', 115200)  # 根据实际串口配置修改

# 配置WAV文件
channels = 1  # 单声道
sample_width = 2  # 采样宽度为2字节
frame_rate = 5715.124  # 采样率为44100Hz
wave_file = wave.open('output.wav',
                      'wb')  # 以二进制写入模式打开一个名为output.wav的音频文件，返回一个WAV文件对象
wave_file.setnchannels(channels)  # 设置音频文件的通道数为channels
wave_file.setsampwidth(sample_width)  # 设置每个采样点的宽度为sample_width
wave_file.setframerate(frame_rate)  # 设置音频文件的采样率为frame_rate

Data = b''  # 用于存储串口读取的数据

# 定义开始信号和结束信号的内容
st_signal = bytes([0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF])
ed_signal = bytes([0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE])

# 发送开始信号
ser.write(st_signal)  # 向串口写入开始信号
time.sleep(0.1)  # 延时0.1秒等待信号发送完成
ser.write(st_signal)  # 再次发送开始信号，以避免DSP的二级缓存未清空

start = time.time()  # 记录开始时间，用于控制录音时间长度
# 读取并写入数据
while True:  # 持续接收数据
    end = time.time()  # 刷新当前时间
    if end - start > 3:  # 控制录制时间为3秒
        break  # 超出录音时间则退出循环
    data = ser.read(ser.inWaiting())  # 读取串口缓存中的所有数据
    Data = Data + data  # 将读取到的数据添加到Data中

# 写入读取到的音频数据
with open("output.txt", "w") as file:  # 打开文件
    print(Data.hex(), file=file)  # 将Data中的数据以十六进制形式写入文件

with open('output.txt', 'r') as file:
    hex_str = file.read()

# 去掉文件的首个和末尾数据，为了避免时序对齐时出现的问题
# 同时去掉两个是因为进行字节转换时，每两个字符为一个字节
hex_str = hex_str[2:-2]  # 用于时域解密

# 检查数据的长度，如果是奇数则补0或删除最后一个字符
if len(hex_str) % 2 != 0:
    hex_str = hex_str[:-1]  # 或者 hex_data = hex_data[:-1]

byte_data = binascii.unhexlify(hex_str)

# 将文件写入wav文件中
wave_file.writeframes(byte_data)

# 发送结束信号
ser.write(ed_signal)
time.sleep(0.1)  # 延时0.1秒等待信号发送完成
ser.write(st_signal)  # 再次发送开始信号，以避免DSP的二级缓存未清空
time.sleep(0.1)
ser.close()  # 关闭串口

# 关闭WAV文件
wave_file.close()
print('Finished')
