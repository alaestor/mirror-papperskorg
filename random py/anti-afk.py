import time
from pynput.keyboard import Key, Controller

keyboard = Controller()
key = "w"

while (1):
    time.sleep(59)
    keyboard.press(key)
    time.sleep(0.3)
    keyboard.release(key)
    print("moved?");

