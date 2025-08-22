import mss
import numpy as np
import serial
import time
import cv2
import time

def main():
    # 初始化串口，串口要调整为自己电脑上的串口
    ser = serial.Serial('COM5', baudrate=972200, timeout=0.1)
    time.sleep(5)
    # 获取屏幕尺寸
    with mss.mss() as sct:
        monitor = sct.monitors[1]  # 主显示器
        target_size = (86, 48)
        
        while True:
            start_time = time.perf_counter()
            
            # 1. 快速截图
            img = np.array(sct.grab(monitor))
            
            # 2. 高效调整尺寸 (使用OpenCV)
            resized = cv2.resize(
                img, target_size, 
                interpolation=cv2.INTER_LINEAR
            )
            
            # 3. 直接转换为RGB565 (向量化操作)
            # 提取RGB通道并移位
            r = (resized[..., 2] >> 3).astype(np.uint16) << 11
            g = (resized[..., 1] >> 2).astype(np.uint16) << 5
            b = (resized[..., 0] >> 3).astype(np.uint16)
            
            # 合并为RGB565
            rgb565 = r | g | b
            
            # 4. 序列化并发送
            ser.write(rgb565.byteswap().tobytes())
            
            # 5. 维持15fps帧率
            elapsed = time.perf_counter() - start_time
            delay = max(0, 1/10 - elapsed)
            time.sleep(delay)

if __name__ == "__main__":
    try:
        main()
    except KeyboardInterrupt:

        print("程序终止")
