import time
from tkinter import Tk
from pynput.keyboard import Controller

def countdown(t):
    while t > 0:
        print("\t", t)
        t -= 1
        time.sleep(1)
    print("Typing...")

"""
def type():
    output = Tk().clipboard_get()
    keyboard = Controller()
    for c in output:
        #keyboard.type(c)
        print(c)
        time.sleep(0.01)
"""

countdown(5)

Controller().type(Tk().clipboard_get())
