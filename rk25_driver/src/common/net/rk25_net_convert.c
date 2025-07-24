#pragma once
#include "rk25_net_convert.h"

VOID RK25PrintTCPConnection(ULONG localIP, USHORT localPort, ULONG remoteIp, USHORT remotePort) {
    union {
        USHORT port;
        UCHAR portbytes[2];
    }localport, remoteport;

    ULONG localbytes[4] = { 0 };
    localbytes[0] = localIP & 0xFF;
    localbytes[1] = (localIP >> 8) & 0xFF;
    localbytes[2] = (localIP >> 16) & 0xFF;
    localbytes[3] = (localIP >> 24) & 0xFF;
    localport.port = 0;
    localport.portbytes[0] = (localPort >> 8) & 0xFF;
    localport.portbytes[1] = localPort & 0xFF;

    ULONG remotebytes[4] = { 0 };
    remotebytes[0] = remoteIp & 0xFF;
    remotebytes[1] = (remoteIp >> 8) & 0xFF;
    remotebytes[2] = (remoteIp >> 16) & 0xFF;
    remotebytes[3] = (remoteIp >> 24) & 0xFF;
    remoteport.port = 0;
    remoteport.portbytes[0] = (remotePort >> 8) & 0xFF;
    remoteport.portbytes[1] = remotePort & 0xFF;

    DbgPrint("%d.%d.%d.%d:%d\t - %d.%d.%d.%d:%d\n",
        localbytes[0], localbytes[1], localbytes[2], localbytes[3], localport.port,
        remotebytes[0], remotebytes[1], remotebytes[2], remotebytes[3], remoteport.port);
}

NTSTATUS RK25ConvertIPv4Dec2Str(IN ULONG ip, OUT PSTR buffer, IN ULONG bufferSize) {
    if (bufferSize < RK25_NET_IPV4_BUFLEN) return STATUS_BUFFER_TOO_SMALL;
    return RtlStringCbPrintfA(
        buffer,
        bufferSize,
        "%u.%u.%u.%u",
        (UCHAR)((ip >> 24) & 0xFF),
        (UCHAR)((ip >> 16) & 0xFF),
        (UCHAR)((ip >> 8) & 0xFF),
        (UCHAR)(ip & 0xFF)
    );
}

NTSTATUS RK25ConvertIPv4Str2Dec(IN PSTR ip, OUT PULONG buffer) {
    NTSTATUS status = STATUS_SUCCESS;
    IN_ADDR inAddr;
    PCSTR endPtr = NULL;

    status = RtlIpv4StringToAddressA(ip, TRUE, &endPtr, &inAddr);
    if (NT_SUCCESS(status)) {
        *buffer = inAddr.S_un.S_addr;
    }
    return status;
}

NTSTATUS RK25ConvertIPv6Str2Dec(IN PSTR ip, OUT PULONGLONG buffer) {
    NTSTATUS status = STATUS_SUCCESS;

    return status;
}