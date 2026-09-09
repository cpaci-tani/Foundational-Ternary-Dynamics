"""Shared stdlib-only Windows process limits and parent-owned cleanup Jobs.

Create the cleanup ancestor before the process-memory Job. Keep both handles
owned only by that metadata process, and retain the returned objects until its
normal exit. Closing a cleanup handle can terminate its owner; flush all
terminal evidence first. This module performs no action merely on import.
"""
import os

OUTER_MONITOR_CAP = 134217728
SUPERVISOR_PARENT_CAP = 1073741824
DESCENDANT_ALLOWANCE = 7381975040
AGGREGATE_MEMORY_CAP = 8589934592


def integer(value, name, lo=0, hi=None):
    if type(value) is not int or value < lo or (hi is not None and value > hi):
        raise ValueError("integer outside domain: " + name)
    return value


def _pid_alive(pid):
    import ctypes
    from ctypes import wintypes as W
    kernel = ctypes.WinDLL("kernel32", use_last_error=True)
    kernel.OpenProcess.argtypes = (W.DWORD, W.BOOL, W.DWORD)
    kernel.OpenProcess.restype = W.HANDLE
    kernel.GetExitCodeProcess.argtypes = (W.HANDLE, ctypes.POINTER(W.DWORD))
    kernel.CloseHandle.argtypes = (W.HANDLE,)
    handle = kernel.OpenProcess(0x1000, False, pid)
    if not handle:
        if ctypes.get_last_error() == 87:
            return False
        raise ctypes.WinError(ctypes.get_last_error())
    try:
        code = W.DWORD()
        if not kernel.GetExitCodeProcess(handle, ctypes.byref(code)):
            raise ctypes.WinError(ctypes.get_last_error())
        return code.value == 259
    finally:
        kernel.CloseHandle(handle)


def _private_memory():
    import ctypes
    from ctypes import wintypes as W
    class Memory(ctypes.Structure):
        _fields_ = [("cb", W.DWORD), ("PageFaultCount", W.DWORD)] + [(name, ctypes.c_size_t) for name in (
            "PeakWorkingSetSize", "WorkingSetSize", "QuotaPeakPagedPoolUsage", "QuotaPagedPoolUsage", "QuotaPeakNonPagedPoolUsage",
            "QuotaNonPagedPoolUsage", "PagefileUsage", "PeakPagefileUsage", "PrivateUsage")]
    kernel = ctypes.WinDLL("kernel32", use_last_error=True)
    kernel.GetCurrentProcess.restype = W.HANDLE
    psapi = ctypes.WinDLL("psapi", use_last_error=True)
    psapi.GetProcessMemoryInfo.argtypes = (W.HANDLE, ctypes.c_void_p, W.DWORD)
    row = Memory()
    row.cb = ctypes.sizeof(row)
    if not psapi.GetProcessMemoryInfo(kernel.GetCurrentProcess(), ctypes.byref(row), ctypes.sizeof(row)):
        raise ctypes.WinError(ctypes.get_last_error())
    return int(row.PrivateUsage)


class MetadataProcessJob:
    """OS process-memory cap; child breaks away into the frozen scientific Job."""
    def __init__(self, cap=None, cleanup=False):
        import ctypes
        from ctypes import wintypes as W
        class Basic(ctypes.Structure):
            _fields_ = [("PerProcessUserTimeLimit", ctypes.c_int64), ("PerJobUserTimeLimit", ctypes.c_int64),
                        ("LimitFlags", W.DWORD), ("MinimumWorkingSetSize", ctypes.c_size_t), ("MaximumWorkingSetSize", ctypes.c_size_t),
                        ("ActiveProcessLimit", W.DWORD), ("Affinity", ctypes.c_size_t), ("PriorityClass", W.DWORD), ("SchedulingClass", W.DWORD)]
        class Extended(ctypes.Structure):
            _fields_ = [("BasicLimitInformation", Basic), ("IoInfo", ctypes.c_uint64 * 6), ("ProcessMemoryLimit", ctypes.c_size_t),
                        ("JobMemoryLimit", ctypes.c_size_t), ("PeakProcessMemoryUsed", ctypes.c_size_t), ("PeakJobMemoryUsed", ctypes.c_size_t)]
        self.ctypes, self.W, self.Extended = ctypes, W, Extended
        self.cap = 0 if cleanup else integer(cap, "metadata OS process-memory cap", 1, SUPERVISOR_PARENT_CAP)
        self.flags = 0x2000 if cleanup else 0x100 | 0x1000
        self.kernel = ctypes.WinDLL("kernel32", use_last_error=True)
        self.kernel.CreateJobObjectW.argtypes = (ctypes.c_void_p, W.LPCWSTR)
        self.kernel.CreateJobObjectW.restype = W.HANDLE
        self.kernel.SetInformationJobObject.argtypes = (W.HANDLE, ctypes.c_int, ctypes.c_void_p, W.DWORD)
        self.kernel.QueryInformationJobObject.argtypes = (W.HANDLE, ctypes.c_int, ctypes.c_void_p, W.DWORD, ctypes.c_void_p)
        self.kernel.GetCurrentProcess.restype = W.HANDLE
        self.kernel.AssignProcessToJobObject.argtypes = (W.HANDLE, W.HANDLE)
        self.kernel.IsProcessInJob.argtypes = (W.HANDLE, W.HANDLE, ctypes.POINTER(W.BOOL))
        self.kernel.OpenProcess.argtypes = (W.DWORD, W.BOOL, W.DWORD)
        self.kernel.OpenProcess.restype = W.HANDLE
        self.kernel.CloseHandle.argtypes = (W.HANDLE,)
        self.handle = self.kernel.CreateJobObjectW(None, None)
        if not self.handle:
            raise ctypes.WinError(ctypes.get_last_error())
        info = Extended()
        info.BasicLimitInformation.LimitFlags = self.flags
        info.ProcessMemoryLimit = self.cap
        if not self.kernel.SetInformationJobObject(self.handle, 9, ctypes.byref(info), ctypes.sizeof(info)):
            raise ctypes.WinError(ctypes.get_last_error())
        if not self.kernel.AssignProcessToJobObject(self.handle, self.kernel.GetCurrentProcess()):
            raise ctypes.WinError(ctypes.get_last_error())
        queried = Extended()
        if not self.kernel.QueryInformationJobObject(self.handle, 9, ctypes.byref(queried), ctypes.sizeof(queried), None):
            raise ctypes.WinError(ctypes.get_last_error())
        if queried.ProcessMemoryLimit != self.cap or queried.BasicLimitInformation.LimitFlags != self.flags or not self.contains(os.getpid()):
            raise ValueError("outer monitor Job cap/assignment not installed")

    def contains(self, pid):
        process = self.kernel.OpenProcess(0x1000, False, pid)
        if not process:
            raise self.ctypes.WinError(self.ctypes.get_last_error())
        try:
            inside = self.W.BOOL()
            if not self.kernel.IsProcessInJob(process, self.handle, self.ctypes.byref(inside)):
                raise self.ctypes.WinError(self.ctypes.get_last_error())
            return bool(inside.value)
        finally:
            self.kernel.CloseHandle(process)

    def peak_memory(self):
        row = self.Extended()
        if not self.kernel.QueryInformationJobObject(self.handle, 9, self.ctypes.byref(row), self.ctypes.sizeof(row), None):
            raise self.ctypes.WinError(self.ctypes.get_last_error())
        return int(row.PeakProcessMemoryUsed)


pid_alive = _pid_alive
private_memory = _private_memory


def install_metadata_boundary(process_cap):
    """Return (cleanup_ancestor, process_memory_job), owned by this process.

    Children escape the immediate memory Job but remain in the cleanup Job.
    Both handles are noninherited. Neither handle is duplicated into children.
    The scientific child still needs assignment to the frozen supervisor Job.
    """
    cleanup = MetadataProcessJob(cleanup=True)
    memory = MetadataProcessJob(process_cap)
    return cleanup, memory
