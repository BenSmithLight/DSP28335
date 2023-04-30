import re

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

string = "0a0a44535020526563656976653a20fffffffffffffffffe01caabfffe23efab0041c0efab0047d2efab0055acef"
string_bytes = bytes.fromhex(string)
matches = re.findall(r'ab.*?ef', string)
Data_bytes = b''
for dataa in matches:
    # data1 = bytes.fromhex(dataa[2:4])
    # data2 = bytes.fromhex(dataa[4:6])
    # data1 = hex(int(dataa[2:4], 16))
    # data2 = hex(int(dataa[4:6], 16))
    # # data = bytes.fromhex(data1)
    # data = bytes.fromhex(data1)
    data = bytes.fromhex(dataa[2:6])
    result = crc8(data)
    if(dataa[6:8] == '{:02X}'.format(result).lower()):
        print("CRC 校验成功")
        Data_bytes = Data_bytes + data
    else:
        print("CRC 校验失败")
