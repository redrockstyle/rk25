#ifndef _RK25_DISPATCHER_H_
#define _RK25_DISPATHCER_H_

#include <ntddk.h>
#include "../../common/rk25_dbg.h"
#include "../../evilcase/process/rk25_proc.h"
#include "../../evilcase/net/rk25_net.h"

#if _RK25_IOCTL
#include "../../common/rk25_ioctl.h"
#endif

//for IRP_MJ_DEVICE_CONTROL
NTSTATUS DriverDispatch(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp);

#endif // _RK25_DISPATCHER_H_