import re

string = "0a0a44535020526563656976653a20fffffffffffffffffe01caabfff512efabfff936efab000309efab001848efab001b41ef"

matches = re.findall(r'ab.*?ef', string)
# 解释正则表达式：
# - ab 开头
# - (?:.(?!ab|ef))*? 匹配任意字符（非贪婪模式），但排除以 ab 或 ef 开头的字符
# - ef 结尾

print(matches)
