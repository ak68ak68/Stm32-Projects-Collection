import serial
import socket

# 串口配置
ser = serial.Serial('COM5', 9600)  # 替换为Arduino Uno连接的串口号

# 服务器配置
server_ip = '192.168.0.14'  # 电脑的IP地址
server_port = 8888

# 创建套接字
server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
server_socket.bind((server_ip, server_port))
server_socket.listen(1)

print('Waiting for ESP32 connection...')
conn, addr = server_socket.accept()
print(f'Connected by {addr}')

try:
    while True:
        # 读取来自Arduino Uno的滑动变阻器值
        line = ser.readline().decode('utf-8').strip()
        if line:
            # 将滑动变阻器值发送到ESP32
            conn.sendall(line.encode('utf-8') + b'\n')
except KeyboardInterrupt:
    print('Closing connection...')
finally:
    ser.close()
    conn.close()
    server_socket.close()
