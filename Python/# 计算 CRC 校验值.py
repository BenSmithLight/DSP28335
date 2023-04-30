# 计算 CRC 校验值
def calc_crc(data):
    crc = 0
    for byte in data:
        crc ^= byte
        for _ in range(8):
            if crc & 0x01:
                crc = (crc >> 1) ^ 0x07
            else:
                crc >>= 1
    return crc

# 将数据转换为十六进制
data = "0a0a44535020526563656976653a20fffffffffffffffffe01caabfff512efabfff936efab000309efab001848ef"
hex_data = bytes.fromhex(data)

data = "fff5"
hex_data = bytes.fromhex(data)
print(hex_data)
crc = calc_crc(hex_data)
print(crc)

# 提取有效数据
start = hex_data.find(b"\xab")
end = hex_data.find(b"\xef", start)
valid_data = hex_data[start+4:end-2]
print("有效数据：", valid_data.hex())

# 分帧处理有效数据
while True:
    start = valid_data.find(b"\xab")
    if start == -1:
        break
    end = valid_data.find(b"\xef", start)
    frame = valid_data[start:end+2]
    crc = frame[-2:]
    frame_data = frame[4:-2]
    print(crc)
    if calc_crc(frame_data) == int.from_bytes(crc, byteorder="big"):
        print("有效数据：", frame_data.hex())
    else:
        print("CRC 校验失败")
    valid_data = valid_data[end+2:]
