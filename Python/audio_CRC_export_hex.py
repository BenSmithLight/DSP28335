# 导入库
import serial  # 导入串口通信库
import time  # 导入时间库
import re  # 导入正则表达式库
import binascii  # 导入二进制转换库
import wave  # 导入WAV文件处理库


# CRC校验函数（多项式为0x07）
# 输入一个字节串（长度为2），返回一个字节（CRC校验结构）
def crc8(buf):
    # 进一步转换，避免ASCII编码导致输出结果不一致
    buf = bytes.fromhex(buf.hex())

    crc = 0  # 赋初始值
    for b in buf:  # 对每一个字节进行CRC计算
        crc ^= b  # 与字节进行异或运算
        for _ in range(8):  # 对每一位进行CRC计算
            if crc & 0x80:  # 如果最高位为1
                crc = ((crc << 1) ^ 0x07) & 0xFF  # 左移一位，与多项式0x07异或，再与0xFF按位与
            else:  # 如果最高位为0
                crc <<= 1  # 左移一位
    return crc  # 返回CRC校验结果


# 配置录音时间
record_time = 5

# 配置WAV文件
channels = 1  # 单声道，即只有一个音频通道
sample_width = 2  # 采样宽度为2字节，即每个采样点占用2个字节的空间
frame_rate = 2280  # 采样率为2280Hz（通过音频总长度计算得到）
wave_file = wave.open('output.wav', 'wb')  # 以写入模式打开output.wav音频文件
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
with open("output.txt", "w") as file:  # 打开文件
    print(Data.hex(), file=file)  # 将Data中的数据以十六进制形式写入文件

# 发送结束信号
ser.write(ed_signal)
time.sleep(0.1)  # 延时0.1秒等待信号发送完成
ser.write(st_signal)  # 再次发送开始信号，以避免DSP的二级缓存未清空
time.sleep(0.1)
ser.close()  # 关闭串口

# 读取十六进制的音频数据
with open("output.txt", "r") as f:  # 打开文件
    Data_string = f.read()  # 字符串形式读取文件

# 使用正则表达式，通过帧头和帧尾匹配音频数据
Data_match = re.findall(r'ab.*?ef', Data_string)  # 匹配音频数据，返回一个列表
Data_bytes = b''  # 用于存储音频数据的字节串，准备写入WAV文件
for data_ma in Data_match:  # 遍历列表
    if (len(data_ma) != 10):  # 检查数据长度
        # print("数据长度不正确")
        pass
    else:
        data = bytes.fromhex(data_ma[2:6])  # 将字符串转换为字节串
        result = crc8(data)  # 计算CRC校验结果
        if (data_ma[6:8] == '{:02X}'.format(result).lower()):  # 检查CRC校验结果
            Data_bytes = Data_bytes + data  # 将数据添加到Data_bytes中
        else:
            print("CRC 校验失败")

# 将校验后的音频数据Data_bytes以十六进制形式写入文件
with open("data_byte.txt", "w") as file:  # 打开文件
    print(Data_bytes.hex(), file=file)  # 将Data_bytes中的数据以十六进制形式写入文件

# 读取校验后的十六进制音频数据
with open('data_byte.txt', 'r') as file:  # 打开文件
    hex_str = file.read()  # 字符串形式读取文件

# 去掉文件的首个和末尾数据，为了避免时序对齐时出现的问题
# 同时去掉两个是因为进行字节转换时，每两个字符为一个字节
hex_str = hex_str[2:-2]

# 检查数据的长度，如果是奇数则删除最后一个字符
if len(hex_str) % 2 != 0:  # 如果数据长度为奇数
    hex_str = hex_str[:-1]  # 删除最后一个字符

# 将十六进制数据转换为字节串
byte_data = binascii.unhexlify(hex_str)

# 将文件写入wav文件中
wave_file.writeframes(byte_data)

# 关闭WAV文件
wave_file.close()
print('Finished')
