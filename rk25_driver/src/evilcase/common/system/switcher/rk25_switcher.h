#ifndef _RK25_SWITCHER_H_
#define _RK25_SWITCHER_H_

#include <ntddk.h>
#include "../../../../common/rk25_dbg.h"

/** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*								 Windows OS Versions								*
*																					*
* More information about Windows OS Version:										*
* https://en.wikipedia.org/wiki/List_of_Microsoft_Windows_versions					*
* More information about syscalls:													*
* https://github.com/j00ru/windows-syscalls											*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
// Windows XP
#define RK25_OS_VERSION_WIN_XP_SP0					0x00201020		// AI-32
#define RK25_OS_VERSION_WIN_XP_SP1					0x00201021		// AI-32, x86-64
#define RK25_OS_VERSION_WIN_XP_SP2					0x00201022		// AI-32, x86-64
#define RK25_OS_VERSION_WIN_XP_SP3					0x00201023		// AI-32
// Windows Server 2003
#define RK25_OS_VERSION_WIN_SERVER_2003_SP0			0x00201030		// AI-32, x86-64
#define RK25_OS_VERSION_WIN_SERVER_2003_SP1			0x00201031		// AI-32, x86-64
#define RK25_OS_VERSION_WIN_SERVER_2003_SP2			0x00201032		// AI-32, x86-64
#define RK25_OS_VERSION_WIN_SERVER_2003_R2_SP1		0x00201033		// AI-32, x86-64
#define RK25_OS_VERSION_WIN_SERVER_2003_R2_SP2		0x00201034		// AI-32, x86-64
// Windows Vista
#define RK25_OS_VERSION_WIN_VISTA_SP0				0x00201040		// AI-32, x86-64
#define RK25_OS_VERSION_WIN_VISTA_SP1				0x00201041		// AI-32, x86-64
#define RK25_OS_VERSION_WIN_VISTA_SP2				0x00201042		// AI-32, x86-64
// Windows Server 2008
#define RK25_OS_VERSION_WIN_SERVER_2008_SP0			0x00201050		// AI-32, x86-64
#define RK25_OS_VERSION_WIN_SERVER_2008_SP1			0x00201051		// AI-32, x86-64
#define RK25_OS_VERSION_WIN_SERVER_2008_SP2			0x00201052		// AI-32, x86-64
#define RK25_OS_VERSION_WIN_SERVER_2008_R2_SP0		0x00201053		// x86-64
#define RK25_OS_VERSION_WIN_SERVER_2008_R2_SP1		0x00201054		// x86-64
// Windows 7
#define RK25_OS_VERSION_WIN_7_SP0					0x00201060		// AI-32, x86-64
#define RK25_OS_VERSION_WIN_7_SP1					0x00201061		// AI-32, x86-64
// Windows 8
#define RK25_OS_VERSION_WIN_8_0						0x00201070		// AI-32, x86-64
#define RK25_OS_VERSION_WIN_8_1						0x00201071		// AI-32, x86-64
// Windows Server 2012
#define RK25_OS_VERSION_WIN_SERVER_2012_SP0			0x00201080		// x86-64
#define RK25_OS_VERSION_WIN_SERVER_2012_SP1			0x00201081		// x86-64
#define RK25_OS_VERSION_WIN_SERVER_2012_R2			0x00201082		// x86-64
// Windows 10
#define RK25_OS_VERSION_WIN_10_1507					0x00201090		// AI-32, x86-64, ARM64
#define RK25_OS_VERSION_WIN_10_1511					0x00201091		// AI-32, x86-64, ARM64
#define RK25_OS_VERSION_WIN_10_1607					0x00201092		// AI-32, x86-64, ARM64
#define RK25_OS_VERSION_WIN_10_1703					0x00201093		// AI-32, x86-64, ARM64
#define RK25_OS_VERSION_WIN_10_1709					0x00201094		// AI-32, x86-64, ARM64
#define RK25_OS_VERSION_WIN_10_1803					0x00201095		// AI-32, x86-64, ARM64
#define RK25_OS_VERSION_WIN_10_1809					0x00201096		// AI-32, x86-64, ARM64
#define RK25_OS_VERSION_WIN_10_1903					0x00201097		// AI-32, x86-64, ARM64
#define RK25_OS_VERSION_WIN_10_1909					0x00201098		// AI-32, x86-64, ARM64
#define RK25_OS_VERSION_WIN_10_2004					0x00201099		// AI-32, x86-64, ARM64
#define RK25_OS_VERSION_WIN_10_20H2					0x0020109A		// AI-32, x86-64, ARM64
#define RK25_OS_VERSION_WIN_10_21H1					0x0020109B		// AI-32, x86-64, ARM64
#define RK25_OS_VERSION_WIN_10_21H2					0x0020109C		// AI-32, x86-64, ARM64
#define RK25_OS_VERSION_WIN_10_22H2					0x0020109D		// AI-32, x86-64, ARM64
// Windows Server 2016
#define RK25_OS_VERSION_WIN_SERVER_2016_LTSC_1607	0x002010A0		// x86-64
#define RK25_OS_VERSION_WIN_SERVER_2016_1709		0x002010A1		// x86-64
#define RK25_OS_VERSION_WIN_SERVER_2016_1803		0x002010A2		// x86-64
// Windows Server 2019
#define RK25_OS_VERSION_WIN_SERVER_2019_LTSC_1809	0x002010B0		// x86-64
#define RK25_OS_VERSION_WIN_SERVER_2019_1903		0x002010B1		// x86-64
#define RK25_OS_VERSION_WIN_SERVER_2019_1909		0x002010B2		// x86-64
#define RK25_OS_VERSION_WIN_SERVER_2019_2004		0x002010B3		// x86-64
#define RK25_OS_VERSION_WIN_SERVER_2019_20H2		0x002010B4		// x86-64
// Windows 11
#define RK25_OS_VERSION_WIN_11_21H2					0x002010C0		// x86-64, ARM64
#define RK25_OS_VERSION_WIN_11_22H2					0x002010C1		// x86-64, ARM64
#define RK25_OS_VERSION_WIN_11_23H2					0x002010C2		// x86-64, ARM64
#define RK25_OS_VERSION_WIN_11_24H2					0x002010C3		// x86-64, ARM64
// Windows Server 2022
#define RK25_OS_VERSION_WIN_SERVER_2022_LTSC_21H2	0x002010D0		// x86-64
#define RK25_OS_VERSION_WIN_SERVER_2022_23H2		0x002010D1		// AMD64
// Windows Server 2025
#define RK25_OS_VERSION_WIN_SERVER_2025_24H2		0x002010E0		// AMD64, ARM64
// Windows Unknow
#define RK25_OS_VERSION_WIN_UNKNOWN					0x00201000
/** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */


/** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*				Unicode name of syscall for NtQuerySystemInformation				*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
#define RK25_SYSCALL_NAME_NTQSI		L"NtQuerySystemInformation"
#define RK25_SYSCALL_NAME_NTTP		L"NtTerminateProcess"
/** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

// Unsupported Windows Version
#define RK25_UNSUPPORTED_NUMBER			0x0000

/** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*				Number of syscall for NtQuerySystemInformation						*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
// Windows XP
#define RK25_WIN_XP_X86_NTQSI			0x00AD
#define RK25_WIN_XP_X64_NTQSI			0x0033
// Windows Server 2003
#define RK25_WIN_SERVER_2003_X86_NTQSI	0x00B5
#define RK25_WIN_SERVER_2003_X64_NTQSI	RK25_WIN_XP_X64_NTQSI
// Windows Vista
#define RK25_WIN_VISTA_X86_NTQSI		0x00F8
#define RK25_WIN_VISTA_X64_NTQSI		RK25_WIN_XP_X64_NTQSI
// Windows 7
#define RK25_WIN_7_X86_NTQSI			0x0105
#define RK25_WIN_7_X64_NTQSI			RK25_WIN_XP_X64_NTQSI
// Windows 8
#define RK25_WIN_8_0_X86_NTQSI			0x0095
#define RK25_WIN_8_1_X86_NTQSI			0x0098
#define RK25_WIN_8_0_X64_NTQSI			0x0034
#define RK25_WIN_8_1_X64_NTQSI			0x0035
// Windows 10
#define RK25_WIN_10_X86_NTQSI_1			0x009A
#define RK25_WIN_10_X86_NTQSI_2			0x009B
#define RK25_WIN_10_X86_NTQSI_3			0x009C
#define RK25_WIN_10_X86_NTQSI_4			0x009D
#define RK25_WIN_10_X64_NTQSI			0x0036
// Windows 11 and Windows Server
#define RK25_WIN_11_NTQSI				0x0036
/** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */


/** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*					Number of syscall for NtTerminateProcess						*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
// Windows XP
#define RK25_WIN_XP_X86_NTTP			0x0101
#define RK25_WIN_XP_X64_NTTP			0x0029
// Windows Server 2003
#define RK25_WIN_SERVER_2003_X86_NTTP	0x010a
#define RK25_WIN_SERVER_2003_X64_NTTP	RK25_WIN_XP_X64_NTTP
// Windows Vista
#define RK25_WIN_VISTA_SP0_X86_NTTP		0x0152
#define RK25_WIN_VISTA_SP12_X86_NTTP	0x014e
#define RK25_WIN_VISTA_X64_NTTP			RK25_WIN_XP_X64_NTTP
// Windows 7
#define RK25_WIN_7_X86_NTTP				0x0172
#define RK25_WIN_7_X64_NTTP				RK25_WIN_XP_X64_NTTP
// Windows 8
#define RK25_WIN_8_X86_NTTP				0x0023
#define RK25_WIN_8_0_X64_NTTP			0x002a
#define RK25_WIN_8_1_X64_NTTP			0x002b
// Windows 10
#define RK25_WIN_10_X86_NTTP			0x0024
#define RK25_WIN_10_X64_NTTP			0x002c
// Windows 11 and Windows Server
#define RK25_WIN_11_NTTP				0x002c
/** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */


/** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*									Net Driver										*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
// Windows XP
#define RK25_NET_DRIVER_XP			L"\\Device\\Tcp"
// Windows Vista and upper
#define RK25_NET_DRIVER_VISTA_UP	L"\\Driver\\nsiproxy"
/** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */


NTSTATUS RK25GetOSVersion(POSVERSIONINFOEX pVersionInfo);
ULONG RK25GetOSVersionCode(POSVERSIONINFOEX pVersionInfo);
VOID RK25PrintDebugOSVersion(POSVERSIONINFOEX pVersionInfo);
VOID RK25PrintDebugOSVersionByCode(ULONG osVersionCode);

// Return TRUE if actual build newest and equal
BOOLEAN RK25WindowsVersionOnlyUp(ULONG osVersionCode);

// Get System Service Number
USHORT RK25GetSysallSSNByCode(ULONG osVersionCode, PWCHAR sysallName);

#endif // !__RK25_SWITCHER_H_
