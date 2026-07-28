import os, math
import psutil
import win32api, win32con, win32process

pid = 0
for i in filter(lambda p: p.name() == "DXMD.exe", psutil.process_iter()):
  pid = i.pid
  break
assert(pid != 0)
print("PID: ", pid)

logical_cores = os.cpu_count()
half_cores = math.floor(logical_cores / 2)-1
print("Number of execution threads:", logical_cores, "(",half_cores+1,"core CPU )")

affinity_mask = 1
for i in range(half_cores):
    affinity_mask = affinity_mask | (affinity_mask << 1)
print("Integer Affinity mask:", int(affinity_mask))
# Is there a more efficient way? Yes. Will I bother to discover it? no.

print("Obtaining handle...")
handle = win32api.OpenProcess(win32con.PROCESS_ALL_ACCESS, True, pid)

print("Changing affinity to half...")
win32process.SetProcessAffinityMask(handle, affinity_mask)
print("Done. DXMD.exe should be running on threads 0 to", half_cores)

print("Changing priority to \"Above Normal\"...")
win32process.SetPriorityClass(handle,win32process.ABOVE_NORMAL_PRIORITY_CLASS)
print("Done. DXMD.exe should be running at \"Above Normal\" priority.")
win32api.CloseHandle(handle)