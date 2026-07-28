import win32gui
import win32con
import win32api
import pynput

modifiers = "<ctrl>+<alt>"
hotkeys = {
	"retarget": f"{modifiers}+t",
	"increase transparency": f"{modifiers}+w",
	"decrease transparency": f"{modifiers}+s",
	"quit": f"{modifiers}+x"
}

def as_percentage(first, second):
   return f"{(first/100) / (second/100) : .2%}"

def set_window_transparency(hwnd, value):
	win32gui.SetWindowLong(
		hwnd,
		win32con.GWL_EXSTYLE,
		win32gui.GetWindowLong(hwnd, win32con.GWL_EXSTYLE ) | win32con.WS_EX_LAYERED
	)
	win32gui.SetLayeredWindowAttributes(
		hwnd,
		win32api.RGB(0,0,0),
		value,
		win32con.LWA_ALPHA
	)

class Target:
	def __init__(self):
		self.hwnd = None
		self.alpha = 255
		self.text = str()

	def has_target(self):
		return self.hwnd != None

	def reset_transparency(self):
		self.alpha = 255
		if (self.has_target()):
			set_window_transparency(self.hwnd, 255)
			print(f"Opaqueness of \"{self.text}\" reset to {as_percentage(255, 255)}")
		else:
			print("Set a target before resetting transparency")

	def apply_transparency(self):
		if (self.has_target()):
			set_window_transparency(self.hwnd, self.alpha)
			print(f"Opaqueness of \"{self.text}\" set to {as_percentage(self.alpha, 255)}")
		else:
			print("Set a target before setting transparency")

	def retarget(self):
		if (self.has_target()):
			self.reset_transparency()
		self.hwnd = win32gui.GetForegroundWindow()
		self.text = win32gui.GetWindowText(self.hwnd)
		print(f"Targetting \"{self.text}\"")

	def increase_transparency(self):
		if self.alpha > 0:
			if self.alpha - 15 >= 0:
				self.alpha -= 15
			else:
				self.alpha = 0
		self.apply_transparency()

	def decrease_transparency(self):
		if self.alpha < 255:
			if self.alpha + 15 <= 255:
				self.alpha += 15
			else:
				self.alpha = 255
		self.apply_transparency()

global_state = Target()

def retarget_hotkey():
	global_state.retarget()

def transparency_up_hotkey():
	global_state.increase_transparency()

def transparency_down_hotkey():
	global_state.decrease_transparency()

def quit_hotkey():
	print("Quitting...")
	if (global_state.has_target()):
		global_state.reset_transparency()
	print("Good bye.")
	exit()

print("Hotkeys:")
for k in hotkeys:
	print(f"Press \"{hotkeys[k]}\" to {k}")

mappings = {
	hotkeys["retarget"]: retarget_hotkey,
	hotkeys["increase transparency"]: transparency_up_hotkey,
	hotkeys["decrease transparency"]: transparency_down_hotkey,
	hotkeys["quit"]: quit_hotkey
}

with pynput.keyboard.GlobalHotKeys(mappings) as h:
	h.join()
