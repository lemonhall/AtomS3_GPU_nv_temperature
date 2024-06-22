import pynvml
import time
import paho.mqtt.client as mqtt

import TkEasyGUI as eg


# MQTT 相关设置
broker = "192.168.50.232"
port = 1883
topic = "lemon_switch"
temperature_topic = "lemon_gpu_temp"



# define layout
layout = [[eg.Text("服务器地址:")], [eg.Input(key="boroker_server_address",text=broker)],
          [eg.Text("服务器端口号:")], [eg.Input(key="boroker_port",text=port)],
          [eg.Text("控制主题:")], [eg.Input(key="controller_topic",text=topic)],
          [eg.Text("温度报送主题:")], [eg.Input(key="tem_topic",text=temperature_topic)],
          [eg.Button(button_text="OK",size=[19,1])]
          ]
# create a window
window = eg.Window("Hello App", layout)
# event loop
while True:
    event, values = window.read()
    if event in ["OK", eg.WINDOW_CLOSED]:
        eg.popup("开始执行")
        broker = window["boroker_server_address"].get()
        port = int(window["boroker_port"].get())
        topic = window["controller_topic"].get()
        temperature_topic = window["tem_topic"].get()
        print(broker)
        break
# close window
window.close()

client = mqtt.Client(mqtt.CallbackAPIVersion.VERSION2)

def get_nvidia_temperature():
    pynvml.nvmlInit()
    handle = pynvml.nvmlDeviceGetHandleByIndex(0)  # 假设获取第一个显卡
    above_50_count = 0
    below_50_count = 0
    while True:
        temperature = pynvml.nvmlDeviceGetTemperature(handle, pynvml.NVML_TEMPERATURE_GPU)
        client.publish(temperature_topic, temperature)
        if temperature > 50:
            above_50_count += 1
            below_50_count = 0  # 重置低于或等于 50 度的次数
            if above_50_count >= 20:
                client.connect(broker, port)
                client.publish(topic, "on")
                above_50_count = 0
                print(f"大于50度，启动风扇")
        elif temperature <= 50:
            below_50_count += 1
            above_50_count = 0  # 重置高于 50 度的次数
            if below_50_count >= 20:
                client.connect(broker, port)
                client.publish(topic, "off")
                below_50_count = 0
                print(f"小于等于50度，关闭风扇")
        print(f"当前 NVIDIA 显卡温度: {temperature} 度")
        time.sleep(3)

try:
    get_nvidia_temperature()
except KeyboardInterrupt:
    pynvml.nvmlShutdown()
    client.disconnect()


#别挂xxx的情况下：
# pip install pynvml
# pip install paho-mqtt
