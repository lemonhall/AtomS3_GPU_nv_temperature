import os, sys, io
import M5
from M5 import *
from umqtt import *
from hardware import *
from base import ATOMSocketBase



mqtt_client = None
atomsocket = None
rgb = None


onoff = None


def mqtt_lemon_switch_event(data):
  global mqtt_client, atomsocket, rgb, onoff
  onoff = (str(((data[1]).decode()))).strip()
  print(onoff)
  if onoff == 'on':
    atomsocket.set_relay(True)
  if onoff == 'off':
    atomsocket.set_relay(False)


def setup():
  global mqtt_client, atomsocket, rgb, onoff

  M5.begin()
  mqtt_client = MQTTClient('M5_switcher', '192.168.50.232', port=1883, user='', password='', keepalive=65535)
  mqtt_client.connect(clean_session=True)
  mqtt_client.subscribe('lemon_switch', mqtt_lemon_switch_event, qos=0)
  rgb = RGB()
  atomsocket = ATOMSocketBase(1, port=(22, 33), relay=23)
  atomsocket.set_relay(False)
  onoff = ''
  rgb.set_brightness(10)
  rgb.fill_color(0xff0000)
  print('hello M5')


def loop():
  global mqtt_client, atomsocket, rgb, onoff
  M5.update()
  mqtt_client.wait_msg()


if __name__ == '__main__':
  try:
    setup()
    while True:
      loop()
  except (Exception, KeyboardInterrupt) as e:
    try:
      from utility import print_error_msg
      print("sososososososo")
      print_error_msg(e)
    except ImportError:
      print("please update to latest firmware")
# main.py