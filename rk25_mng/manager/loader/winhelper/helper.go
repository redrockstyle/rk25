package winhelper

import (
	"syscall"
	"unsafe"

	"golang.org/x/sys/windows"
)

const (
	SERVICE_KERNEL_DRIVER     = 0x00000001
	SERVICE_DEMAND_START      = 0x00000003
	SERVICE_ERROR_NORMAL      = 0x00000001
	SC_MANAGER_CREATE_SERVICE = 0x0002
	SERVICE_START             = 0x0010
	SERVICE_STOP              = 0x0020
	SERVICE_DELETE            = 0x10000
)

var (
	advapi32               = windows.NewLazySystemDLL("advapi32.dll")
	procOpenSCManager      = advapi32.NewProc("OpenSCManagerW")
	procCreateService      = advapi32.NewProc("CreateServiceW")
	procOpenService        = advapi32.NewProc("OpenServiceW")
	procStartService       = advapi32.NewProc("StartServiceW")
	procControlService     = advapi32.NewProc("ControlService")
	procDeleteService      = advapi32.NewProc("DeleteService")
	procCloseServiceHandle = advapi32.NewProc("CloseServiceHandle")
)

// type DriverLoaderHelper interface {
// 	OpenSCManager() (windows.Handle, error)
// 	OpenService(scHandle windows.Handle, serviceName *uint16, desiredAccess uint32) (windows.Handle, error)
// 	CreateService(scHandle windows.Handle, serviceName, displayName, binPath string) (windows.Handle, error)
// 	StartService(svcHandle windows.Handle) error
// 	StopService(svcHandle windows.Handle) error
// 	DeleteService(svcHandle windows.Handle) error
// 	CloseServiceHandle(handle windows.Handle) error
// 	GetServiceHandle(service string) (windows.Handle, error)
// }

func OpenSCManager() (windows.Handle, error) {
	ret, _, err := procOpenSCManager.Call(0, 0, SC_MANAGER_CREATE_SERVICE)
	if ret == 0 {
		return 0, err
	}
	return windows.Handle(ret), nil
}

func OpenService(scHandle windows.Handle, serviceName *uint16, desiredAccess uint32) (windows.Handle, error) {
	ret, _, err := procOpenService.Call(
		uintptr(scHandle),
		uintptr(unsafe.Pointer(serviceName)),
		uintptr(desiredAccess),
	)
	if ret == 0 {
		return 0, err
	}
	return windows.Handle(ret), nil
}

func CreateService(scHandle windows.Handle, serviceName, displayName, binPath string) (windows.Handle, error) {
	sName, _ := syscall.UTF16PtrFromString(serviceName)
	dName, _ := syscall.UTF16PtrFromString(displayName)
	path, _ := syscall.UTF16PtrFromString(binPath)

	ret, _, err := procCreateService.Call(
		uintptr(scHandle),
		uintptr(unsafe.Pointer(sName)),
		uintptr(unsafe.Pointer(dName)),
		SERVICE_START|SERVICE_STOP|SERVICE_DELETE,
		SERVICE_KERNEL_DRIVER,
		SERVICE_DEMAND_START,
		SERVICE_ERROR_NORMAL,
		uintptr(unsafe.Pointer(path)),
		0, 0, 0, 0, 0)

	if ret == 0 {
		return 0, err
	}
	return windows.Handle(ret), nil
}

func StartService(svcHandle windows.Handle) error {
	ret, _, err := procStartService.Call(uintptr(svcHandle), 0, 0)
	if ret == 0 {
		return err
	}
	return nil
}

func StopService(svcHandle windows.Handle) error {
	var status windows.SERVICE_STATUS
	ret, _, err := procControlService.Call(
		uintptr(svcHandle),
		0x00000001, // SERVICE_CONTROL_STOP
		uintptr(unsafe.Pointer(&status)),
	)
	if ret == 0 {
		return err
	}
	return nil
}

func DeleteService(svcHandle windows.Handle) error {
	ret, _, err := procDeleteService.Call(uintptr(svcHandle))
	if ret == 0 {
		return err
	}
	return nil
}

func CloseServiceHandle(handle windows.Handle) error {
	ret, _, err := procCloseServiceHandle.Call(uintptr(handle))
	if ret == 0 {
		return err
	}
	return nil
}

func GetServiceHandle(service string) (windows.Handle, error) {
	scHandle, err := OpenSCManager()
	if err != nil {
		return 0, err
	}
	defer CloseServiceHandle(scHandle)

	sName, _ := syscall.UTF16PtrFromString(service)
	svcHandle, err := OpenService(scHandle, sName, SERVICE_START|SERVICE_STOP|SERVICE_DELETE)
	if err != nil {
		return 0, err
	}
	return svcHandle, nil
}
