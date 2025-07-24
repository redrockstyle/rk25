#pragma once
#include "rk25_switcher.h"

#define RK25_PROLOG_SWITCHER "OS Switcher"

ULONG glOsVersionCode = RK25_OS_VERSION_WIN_UNKNOWN;

NTSTATUS RK25GetOSVersion(POSVERSIONINFOEX pVersionInfo) {
	NTSTATUS(NTAPI * RtlGetVersion)(POSVERSIONINFOEX);
	UNICODE_STRING funcName;

	RtlInitUnicodeString(&funcName, L"RtlGetVersion");
	RtlGetVersion = (NTSTATUS(NTAPI*)(POSVERSIONINFOEX))MmGetSystemRoutineAddress(&funcName);

	if (!RtlGetVersion) {
		return STATUS_NOT_SUPPORTED;
	}

	pVersionInfo->dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);

	return RtlGetVersion(pVersionInfo);
}

ULONG RK25GetOSVersionCode(POSVERSIONINFOEX pVersionInfo) {
	ULONG code = RK25_OS_VERSION_WIN_UNKNOWN;

	if (glOsVersionCode != RK25_OS_VERSION_WIN_UNKNOWN) return glOsVersionCode;

	if (pVersionInfo) {
		ULONG major = pVersionInfo->dwMajorVersion;
		ULONG minor = pVersionInfo->dwMinorVersion;
		ULONG build = pVersionInfo->dwBuildNumber;
		USHORT spMajor = pVersionInfo->wServicePackMajor;
		UCHAR type = pVersionInfo->wProductType;

		if (major == 5 && minor == 1) {
			// Windows XP Starter
			// Windows XP Home
			// Windows XP Professional
			// Windows XP 64 - bit Edition (SP3 only for AI-32)
			// Windows XP Media Center Edition
			// Windows XP Media Center Edition 2004
			// Windows XP Media Center Edition 2005
			// Windows XP Media Center Edition 2005 Update Rollup 2
			if (spMajor == 0) code = RK25_OS_VERSION_WIN_XP_SP0;
			else if (spMajor == 1) code = RK25_OS_VERSION_WIN_XP_SP1;
			else if (spMajor == 2) code = RK25_OS_VERSION_WIN_XP_SP2;
			else if (spMajor == 3) code = RK25_OS_VERSION_WIN_XP_SP3;
		}
		else if (major == 5 && minor == 2) {
			// Windows Server 2003
			if (type == VER_NT_SERVER) {
				// Windows Server Web
				// Windows Server Standard
				// Windows Server Enterprise
				// Windows Server Datacenter
				// Windows Storage Server
				// Windows Unified Data Storage Server
				if (spMajor == 0) code = RK25_OS_VERSION_WIN_SERVER_2003_SP0;
				if (spMajor == 1) code = wcsstr((CONST wchar_t*) &pVersionInfo->szCSDVersion, L"R2") == NULL ? RK25_OS_VERSION_WIN_SERVER_2003_SP1 : RK25_OS_VERSION_WIN_SERVER_2003_R2_SP1;
				if (spMajor == 2) code = wcsstr((CONST wchar_t*)&pVersionInfo->szCSDVersion, L"R2") == NULL ? RK25_OS_VERSION_WIN_SERVER_2003_SP2 : RK25_OS_VERSION_WIN_SERVER_2003_R2_SP2;
			}
			else {
				// Windows XP Professional x64 Edition (SP3 only AI-32)
				if (spMajor == 0) code = RK25_OS_VERSION_WIN_XP_SP0;
				else if (spMajor == 1) code = RK25_OS_VERSION_WIN_XP_SP1;
				else if (spMajor == 2) code = RK25_OS_VERSION_WIN_XP_SP2;
				else if (spMajor == 3) code = RK25_OS_VERSION_WIN_XP_SP3;
			}
		}
		else if (major == 6 && minor == 0) {
			// Windows Server 2008
			if (type == VER_NT_SERVER) {
				// Windows Server Foundation
				// Windows Server Standard
				// Windows Server Enterprise
				// Windows Server Datacenter
				// Windows Server for Itanium - based Systems
				// Windows Storage Server
				// Windows Web Server
				if (spMajor == 0) code = RK25_OS_VERSION_WIN_SERVER_2008_SP0;
				else if (spMajor == 1) code = RK25_OS_VERSION_WIN_SERVER_2008_SP1;
				else if (spMajor == 2) code = RK25_OS_VERSION_WIN_SERVER_2008_SP2;
			}
			else {
				// Windows Vista Starter
				// Windows Vista Home Basic
				// Windows Vista Home Premium
				// Windows Vista Business
				// Windows Vista Enterprise
				// Windows Vista Ultimate
				if (spMajor == 0) code = RK25_OS_VERSION_WIN_VISTA_SP0;
				else if (spMajor == 1) code = RK25_OS_VERSION_WIN_VISTA_SP1;
				else if (spMajor == 2) code = RK25_OS_VERSION_WIN_VISTA_SP2;
			}
		}
		else if (major == 6 && minor == 1) {
			// Windows Server 2008 R2
			if (type == VER_NT_SERVER) {
				// Windows Server Foundation
				// Windows Server Standard
				// Windows Server Enterprise
				// Windows Server Datacenter
				// Windows Server for Itanium - based Systems
				// Windows Storage Server
				// Windows Web Server
				if (spMajor == 0 && wcsstr((CONST wchar_t*)&pVersionInfo->szCSDVersion, L"R2") != NULL) code = RK25_OS_VERSION_WIN_SERVER_2008_R2_SP0;
				else if (spMajor == 1 && wcsstr((CONST wchar_t*)&pVersionInfo->szCSDVersion, L"R2") != NULL) code = RK25_OS_VERSION_WIN_SERVER_2008_R2_SP1;
			}
			else {
				// Windows 7 Starter
				// Windows 7 Home Basic
				// Windows 7 Home Premium
				// Windows 7 Professional
				// Windows 7 Enterprise
				// Windows 7 Ultimate
				if (spMajor == 0) code = RK25_OS_VERSION_WIN_7_SP0;
				else if (spMajor == 1) code = RK25_OS_VERSION_WIN_7_SP1;
			}
		}
		else if (major == 6 && minor == 2) {
			// Windows Server 2012
			if (type == VER_NT_SERVER) {
				// Windows Server Foundation
				// Windows Server Essentials
				// Windows Server Standard
				// Windows Server Datacenter
				if (spMajor == 0) code = RK25_OS_VERSION_WIN_SERVER_2012_SP0;
				else if (spMajor == 1) code = RK25_OS_VERSION_WIN_SERVER_2012_SP1;
			}
			else {
				// Windows 8
				// Windows 8 Pro
				// Windows 8 Enterprise
				code = RK25_OS_VERSION_WIN_8_0;
			}
		}
		else if (major == 6 && minor == 3) {
			// Windows Server 2012 R2
			if (type == VER_NT_SERVER) {
				// Windows Server Foundation
				// Windows Server Essentials
				// Windows Server Standard
				// Windows Server Datacenter
				code = RK25_OS_VERSION_WIN_SERVER_2012_R2; // Windows Server 2012 SP1
			}
			else {
				// Windows 8.1
				// Windows 8.1 Pro
				// Windows 8.1 Enterprise
				// Windows 8.1 with Bing
				code = RK25_OS_VERSION_WIN_8_1;
			}
		}
		else if (major == 10 && minor == 0) {
			if (type == VER_NT_SERVER) {
				// Windows Server Essentials
				// Windows Server Standard
				// Windows Server Datacenter

				// Windows Server 2016
				if (build == 14393) code = RK25_OS_VERSION_WIN_SERVER_2016_LTSC_1607;
				else if (build == 16299) code = RK25_OS_VERSION_WIN_SERVER_2016_1709;
				else if (build == 16299) code = RK25_OS_VERSION_WIN_SERVER_2016_1803;
				// Windows Server 2019
				else if (build == 17763) code = RK25_OS_VERSION_WIN_SERVER_2019_LTSC_1809;
				else if (build == 18362) code = RK25_OS_VERSION_WIN_SERVER_2019_1903;
				else if (build == 18363) code = RK25_OS_VERSION_WIN_SERVER_2019_1909;
				else if (build == 19041) code = RK25_OS_VERSION_WIN_SERVER_2019_2004;
				else if (build == 19042) code = RK25_OS_VERSION_WIN_SERVER_2019_20H2;
				// Windows Server 2022
				else if (build == 20348) code = RK25_OS_VERSION_WIN_SERVER_2022_LTSC_21H2;
				else if (build == 25398) code = RK25_OS_VERSION_WIN_SERVER_2022_23H2;
				// Windows Server 2025
				else if (build == 26100) code = RK25_OS_VERSION_WIN_SERVER_2025_24H2;

			}
			// Windows 10
			else if (build >= 10240 && build <= 19045) {
				// Windows 10 Home
				// Windows 10 Pro
				// Windows 10 Education
				// Windows 10 Enterprise
				// Windows 10 Pro for Workstations
				// Windows 10 Pro Education
				// Windows 10 S
				// Windows 10 Enterprise LTSC
				if (build == 10240) code = RK25_OS_VERSION_WIN_10_1507;
				else if (build == 10586) code = RK25_OS_VERSION_WIN_10_1511;
				else if (build == 14393) code = RK25_OS_VERSION_WIN_10_1607;
				else if (build == 15063) code = RK25_OS_VERSION_WIN_10_1703;
				else if (build == 16299) code = RK25_OS_VERSION_WIN_10_1709;
				else if (build == 17134) code = RK25_OS_VERSION_WIN_10_1803;
				else if (build == 17763) code = RK25_OS_VERSION_WIN_10_1809;
				else if (build == 18362) code = RK25_OS_VERSION_WIN_10_1903;
				else if (build == 18363) code = RK25_OS_VERSION_WIN_10_1909;
				else if (build == 19041) code = RK25_OS_VERSION_WIN_10_2004;
				else if (build == 19042) code = RK25_OS_VERSION_WIN_10_20H2;
				else if (build == 19043) code = RK25_OS_VERSION_WIN_10_21H1;
				else if (build == 19044) code = RK25_OS_VERSION_WIN_10_21H2;
				else if (build == 19045) code = RK25_OS_VERSION_WIN_10_22H2;
			}
			// Windows 11
			else if (build >= 22000) {
				// Windows 11 Home
				// Windows 11 Pro
				// Windows 11 Pro for Workstations
				// Windows 11 Pro Education
				// Windows 11 Education
				// Windows 11 Enterprise
				// Windows 11 SE
				if (build == 22000) code = RK25_OS_VERSION_WIN_11_21H2;
				else if (build == 22621) code = RK25_OS_VERSION_WIN_11_22H2;
				else if (build == 22631) code = RK25_OS_VERSION_WIN_11_23H2;
				else if (build == 26100) code = RK25_OS_VERSION_WIN_11_24H2;
			}
		}
	}
	glOsVersionCode = code;
	return code;
}

VOID RK25PrintDebugOSVersion(POSVERSIONINFOEX pVersionInfo) {
	if (pVersionInfo->dwMajorVersion == 0) {
		RK25_DBG_ERR(RK25_PROLOG_SWITCHER, "Unknown Version OS");
	}
	else {
		RK25_DBG_INFO_F(RK25_PROLOG_SWITCHER, "OS Windows Version:\n\tMajor: %d\n\tMinor: %d\n\tBuild: %d\n\tServicePackMajor: %d\n\tProductType: %d\n\tCSDVersion: %wZ",
			pVersionInfo->dwMajorVersion, pVersionInfo->dwMinorVersion, pVersionInfo->dwBuildNumber, pVersionInfo->wServicePackMajor, pVersionInfo->wProductType, pVersionInfo->szCSDVersion);
	}
}

VOID RK25PrintDebugOSVersionByCode(ULONG osVersionCode) {
	switch (osVersionCode) {

	// Windows XP
	case RK25_OS_VERSION_WIN_XP_SP0:
		RK25_DBG_INFO(RK25_PROLOG_SWITCHER, "OS Version: Windows XP SP0");
		break;
	case RK25_OS_VERSION_WIN_XP_SP1:
		RK25_DBG_INFO(RK25_PROLOG_SWITCHER, "OS Version: Windows XP SP1");
		break;
	case RK25_OS_VERSION_WIN_XP_SP2:
		RK25_DBG_INFO(RK25_PROLOG_SWITCHER, "OS Version: Windows XP SP2");
		break;
	case RK25_OS_VERSION_WIN_XP_SP3:
		RK25_DBG_INFO(RK25_PROLOG_SWITCHER, "OS Version: Windows XP SP3");
		break;

	// Windows Server 2003
	case RK25_OS_VERSION_WIN_SERVER_2003_SP0:
		RK25_DBG_INFO(RK25_PROLOG_SWITCHER, "OS Version: Windows Server 2003 SP0");
		break;
	case RK25_OS_VERSION_WIN_SERVER_2003_SP1:
		RK25_DBG_INFO(RK25_PROLOG_SWITCHER, "OS Version: Windows Server 2003 SP1");
		break;
	case RK25_OS_VERSION_WIN_SERVER_2003_SP2:
		RK25_DBG_INFO(RK25_PROLOG_SWITCHER, "OS Version: Windows Server 2003 SP2");
		break;
	case RK25_OS_VERSION_WIN_SERVER_2003_R2_SP1:
		RK25_DBG_INFO(RK25_PROLOG_SWITCHER, "OS Version: Windows Server 2003 R2 SP1");
		break;
	case RK25_OS_VERSION_WIN_SERVER_2003_R2_SP2:
		RK25_DBG_INFO(RK25_PROLOG_SWITCHER, "OS Version: Windows Server 2003 R2 SP1");
		break;

	// Windows Vista
	case RK25_OS_VERSION_WIN_VISTA_SP0:
		RK25_DBG_INFO(RK25_PROLOG_SWITCHER, "OS Version: Windows Vista SP0");
		break;
	case RK25_OS_VERSION_WIN_VISTA_SP1:
		RK25_DBG_INFO(RK25_PROLOG_SWITCHER, "OS Version: Windows Vista SP1");
		break;
	case RK25_OS_VERSION_WIN_VISTA_SP2:
		RK25_DBG_INFO(RK25_PROLOG_SWITCHER, "OS Version: Windows Vista SP2");
		break;

	// Windows Serer 2008
	case RK25_OS_VERSION_WIN_SERVER_2008_SP0:
		RK25_DBG_INFO(RK25_PROLOG_SWITCHER, "OS Version: Windows Server 2008 SP0");
		break;
	case RK25_OS_VERSION_WIN_SERVER_2008_SP2:
		RK25_DBG_INFO(RK25_PROLOG_SWITCHER, "OS Version: Windows Server 2008 SP2");
		break;
	case RK25_OS_VERSION_WIN_SERVER_2008_R2_SP0:
		RK25_DBG_INFO(RK25_PROLOG_SWITCHER, "OS Version: Windows Server 2008 R2 SP0");
		break;
	case RK25_OS_VERSION_WIN_SERVER_2008_R2_SP1:
		RK25_DBG_INFO(RK25_PROLOG_SWITCHER, "OS Version: Windows Server 2008 R2 SP1");
		break;

	// Windows 7
	case RK25_OS_VERSION_WIN_7_SP0:
		RK25_DBG_INFO(RK25_PROLOG_SWITCHER, "OS Version: Windows 7 SP0");
		break;
	case RK25_OS_VERSION_WIN_7_SP1:
		RK25_DBG_INFO(RK25_PROLOG_SWITCHER, "OS Version: Windows 7 SP1");
		break;

	// Windows 8
	case RK25_OS_VERSION_WIN_8_0:
		RK25_DBG_INFO(RK25_PROLOG_SWITCHER, "OS Version: Windows 8.0");
		break;
	case RK25_OS_VERSION_WIN_8_1:
		RK25_DBG_INFO(RK25_PROLOG_SWITCHER, "OS Version: Windows 8.1");
		break;

	// Windows Server 2012
	case RK25_OS_VERSION_WIN_SERVER_2012_SP0:
		RK25_DBG_INFO(RK25_PROLOG_SWITCHER, "OS Version: Windows Server 2012 SP0");
		break;
	case RK25_OS_VERSION_WIN_SERVER_2012_SP1:
		RK25_DBG_INFO(RK25_PROLOG_SWITCHER, "OS Version: Windows Server 2012 SP1");
		break;
	case RK25_OS_VERSION_WIN_SERVER_2012_R2:
		RK25_DBG_INFO(RK25_PROLOG_SWITCHER, "OS Version: Windows Server 2012 R2");
		break;

	// Windows 10
	case RK25_OS_VERSION_WIN_10_1507:
		RK25_DBG_INFO(RK25_PROLOG_SWITCHER, "OS Version: Windows 10 1507");
		break;
	case RK25_OS_VERSION_WIN_10_1511:
		RK25_DBG_INFO(RK25_PROLOG_SWITCHER, "OS Version: Windows 10 1511");
		break;
	case RK25_OS_VERSION_WIN_10_1607:
		RK25_DBG_INFO(RK25_PROLOG_SWITCHER, "OS Version: Windows 10 1607");
		break;
	case RK25_OS_VERSION_WIN_10_1703:
		RK25_DBG_INFO(RK25_PROLOG_SWITCHER, "OS Version: Windows 10 1703");
		break;
	case RK25_OS_VERSION_WIN_10_1709:
		RK25_DBG_INFO(RK25_PROLOG_SWITCHER, "OS Version: Windows 10 1709");
		break;
	case RK25_OS_VERSION_WIN_10_1803:
		RK25_DBG_INFO(RK25_PROLOG_SWITCHER, "OS Version: Windows 10 1803");
		break;
	case RK25_OS_VERSION_WIN_10_1809:
		RK25_DBG_INFO(RK25_PROLOG_SWITCHER, "OS Version: Windows 10 1809");
		break;
	case RK25_OS_VERSION_WIN_10_1903:
		RK25_DBG_INFO(RK25_PROLOG_SWITCHER, "OS Version: Windows 10 1903");
		break;
	case RK25_OS_VERSION_WIN_10_1909:
		RK25_DBG_INFO(RK25_PROLOG_SWITCHER, "OS Version: Windows 10 1909");
		break;
	case RK25_OS_VERSION_WIN_10_2004:
		RK25_DBG_INFO(RK25_PROLOG_SWITCHER, "OS Version: Windows 10 2004");
		break;
	case RK25_OS_VERSION_WIN_10_20H2:
		RK25_DBG_INFO(RK25_PROLOG_SWITCHER, "OS Version: Windows 10 20H2");
		break;
	case RK25_OS_VERSION_WIN_10_21H1:
		RK25_DBG_INFO(RK25_PROLOG_SWITCHER, "OS Version: Windows 10 21H1");
		break;
	case RK25_OS_VERSION_WIN_10_21H2:
		RK25_DBG_INFO(RK25_PROLOG_SWITCHER, "OS Version: Windows 10 21H2");
		break;
	case RK25_OS_VERSION_WIN_10_22H2:
		RK25_DBG_INFO(RK25_PROLOG_SWITCHER, "OS Version: Windows 10 22H2");
		break;

	// Windows Server 2016
	case RK25_OS_VERSION_WIN_SERVER_2016_LTSC_1607:
		RK25_DBG_INFO(RK25_PROLOG_SWITCHER, "OS Version: Windows Server 2016 LTSC 1607");
		break;
	case RK25_OS_VERSION_WIN_SERVER_2016_1709:
		RK25_DBG_INFO(RK25_PROLOG_SWITCHER, "OS Version: Windows Server 2016 1709");
		break;
	case RK25_OS_VERSION_WIN_SERVER_2016_1803:
		RK25_DBG_INFO(RK25_PROLOG_SWITCHER, "OS Version: Windows Server 2016 1803");
		break;

	// Windows Server 2019
	case RK25_OS_VERSION_WIN_SERVER_2019_LTSC_1809:
		RK25_DBG_INFO(RK25_PROLOG_SWITCHER, "OS Version: Windows Server 2019 LTSC 1809");
		break;
	case RK25_OS_VERSION_WIN_SERVER_2019_1903:
		RK25_DBG_INFO(RK25_PROLOG_SWITCHER, "OS Version: Windows Server 2019 LTSC 1903");
		break;
	case RK25_OS_VERSION_WIN_SERVER_2019_1909:
		RK25_DBG_INFO(RK25_PROLOG_SWITCHER, "OS Version: Windows Server 2019 LTSC 1909");
		break;
	case RK25_OS_VERSION_WIN_SERVER_2019_2004:
		RK25_DBG_INFO(RK25_PROLOG_SWITCHER, "OS Version: Windows Server 2019 LTSC 2004");
		break;
	case RK25_OS_VERSION_WIN_SERVER_2019_20H2:
		RK25_DBG_INFO(RK25_PROLOG_SWITCHER, "OS Version: Windows Server 2019 LTSC 20H2");
		break;

	// Windows 11
	case RK25_OS_VERSION_WIN_11_21H2:
		RK25_DBG_INFO(RK25_PROLOG_SWITCHER, "OS Version: Windows 11 21H2");
		break;
	case RK25_OS_VERSION_WIN_11_22H2:
		RK25_DBG_INFO(RK25_PROLOG_SWITCHER, "OS Version: Windows 11 22H2");
		break;
	case RK25_OS_VERSION_WIN_11_23H2:
		RK25_DBG_INFO(RK25_PROLOG_SWITCHER, "OS Version: Windows 11 23H2");
		break;
	case RK25_OS_VERSION_WIN_11_24H2:
		RK25_DBG_INFO(RK25_PROLOG_SWITCHER, "OS Version: Windows 11 24H2");
		break;

	// Windows Server 2022
	case RK25_OS_VERSION_WIN_SERVER_2022_LTSC_21H2:
		RK25_DBG_INFO(RK25_PROLOG_SWITCHER, "OS Version: Windows Server 2022 21H2");
		break;
	case RK25_OS_VERSION_WIN_SERVER_2022_23H2:
		RK25_DBG_INFO(RK25_PROLOG_SWITCHER, "OS Version: Windows Server 2022 23H2");
		break;

	// Windows Server 2025
	case RK25_OS_VERSION_WIN_SERVER_2025_24H2:
		RK25_DBG_INFO(RK25_PROLOG_SWITCHER, "OS Version: Windows Server 2025 24H2");
		break;

	// Windows Unknown
	case RK25_OS_VERSION_WIN_UNKNOWN:
	default:
		RK25_DBG_INFO(RK25_PROLOG_SWITCHER, "OS Version: Windows Unknown");
		break;
	}
}

USHORT RK25GetSysallSSNByCode(ULONG osVersionCode, PWCHAR syscallName) {
	if (wcsstr(syscallName, RK25_SYSCALL_NAME_NTTP)) {
		switch (osVersionCode)
		{
		// Windows XP
		case RK25_OS_VERSION_WIN_XP_SP1:
		case RK25_OS_VERSION_WIN_XP_SP2:
#if defined(_M_X64)
			return RK25_WIN_XP_X64_NTTP;
#endif
		case RK25_OS_VERSION_WIN_XP_SP0:
		case RK25_OS_VERSION_WIN_XP_SP3:
#if defined(_M_X64)
			return RK25_UNSUPPORTED_NUMBER;
#else
			return RK25_WIN_XP_X86_NTTP;
#endif

		// Windows Server 2003
		case RK25_OS_VERSION_WIN_SERVER_2003_SP0:
		case RK25_OS_VERSION_WIN_SERVER_2003_SP1:
		case RK25_OS_VERSION_WIN_SERVER_2003_SP2:
		case RK25_OS_VERSION_WIN_SERVER_2003_R2_SP1:
		case RK25_OS_VERSION_WIN_SERVER_2003_R2_SP2:
#if defined(_M_X64)
			return RK25_WIN_SERVER_2003_X64_NTTP;
#else
			return RK25_WIN_SERVER_2003_X86_NTTP;
#endif

		// Windows Server 2008
		// Windows Vista
		case RK25_OS_VERSION_WIN_SERVER_2008_SP0:
		case RK25_OS_VERSION_WIN_VISTA_SP0:
#if defined(_M_IX86)
			return RK25_WIN_VISTA_SP0_X86_NTTP;
#endif
		case RK25_OS_VERSION_WIN_SERVER_2008_SP1:
		case RK25_OS_VERSION_WIN_SERVER_2008_SP2:
		case RK25_OS_VERSION_WIN_VISTA_SP1:
		case RK25_OS_VERSION_WIN_VISTA_SP2:
#if defined(_M_X64)
			return RK25_WIN_VISTA_X64_NTTP;
#else
			return RK25_WIN_VISTA_SP12_X86_NTTP;
#endif

		// Windows Server 2008 R2
		case RK25_OS_VERSION_WIN_SERVER_2008_R2_SP0:
		case RK25_OS_VERSION_WIN_SERVER_2008_R2_SP1: 
		// Windows 7
		case RK25_OS_VERSION_WIN_7_SP0:
		case RK25_OS_VERSION_WIN_7_SP1:
#if defined(_M_X64)
			return RK25_WIN_7_X64_NTTP;
#else
			return RK25_WIN_7_X86_NTTP;
#endif

		// Windows Server 2012
		case RK25_OS_VERSION_WIN_SERVER_2012_SP0:
		case RK25_OS_VERSION_WIN_SERVER_2012_SP1:
		// Windows 8
		case RK25_OS_VERSION_WIN_8_0:
#if defined(_M_X64)
			return RK25_WIN_8_0_X64_NTTP;
#else
			return RK25_WIN_8_X86_NTTP;
#endif

		// Windows Server 2012
		case RK25_OS_VERSION_WIN_SERVER_2012_R2:
		// Windows 8.1
		case RK25_OS_VERSION_WIN_8_1:
#if defined(_M_X64)
			return RK25_WIN_8_1_X64_NTTP;
#else
			return RK25_WIN_8_X86_NTTP;
#endif

		// Windows 10
		case RK25_OS_VERSION_WIN_10_1507:
		case RK25_OS_VERSION_WIN_10_1511:
		case RK25_OS_VERSION_WIN_SERVER_2016_LTSC_1607: // Windows Server 2016
		case RK25_OS_VERSION_WIN_10_1607:
		case RK25_OS_VERSION_WIN_10_1703:
		// Windows Server 2016
		case RK25_OS_VERSION_WIN_SERVER_2016_1709:
		case RK25_OS_VERSION_WIN_SERVER_2016_1803:
		case RK25_OS_VERSION_WIN_SERVER_2019_LTSC_1809:
		case RK25_OS_VERSION_WIN_SERVER_2019_1903:
		case RK25_OS_VERSION_WIN_SERVER_2019_1909:
		case RK25_OS_VERSION_WIN_SERVER_2019_2004:
		case RK25_OS_VERSION_WIN_SERVER_2019_20H2:
		// Windows 10
		case RK25_OS_VERSION_WIN_10_1709:
		case RK25_OS_VERSION_WIN_10_1803:
		case RK25_OS_VERSION_WIN_10_1809:
		case RK25_OS_VERSION_WIN_10_1903:
		case RK25_OS_VERSION_WIN_10_1909:
		case RK25_OS_VERSION_WIN_10_2004:
		case RK25_OS_VERSION_WIN_10_20H2:
		case RK25_OS_VERSION_WIN_10_21H1:
		case RK25_OS_VERSION_WIN_10_21H2:
		case RK25_OS_VERSION_WIN_10_22H2:
#if defined(_M_X64)
			return RK25_WIN_10_X64_NTTP;
#endif
#if defined(_M_IX86)
			return RK25_WIN_10_X86_NTTP;
#endif

#if defined(_M_X64)
		// Windows 11
		case RK25_OS_VERSION_WIN_11_21H2:
		case RK25_OS_VERSION_WIN_11_22H2:
		case RK25_OS_VERSION_WIN_11_23H2:
		case RK25_OS_VERSION_WIN_11_24H2:
		// Windows Server 2022
		case RK25_OS_VERSION_WIN_SERVER_2022_LTSC_21H2:
		case RK25_OS_VERSION_WIN_SERVER_2022_23H2:
		// Windows Server 2025
		case RK25_OS_VERSION_WIN_SERVER_2025_24H2:
			return RK25_WIN_11_NTTP;
#endif
		default:
			return RK25_UNSUPPORTED_NUMBER;
		}
	}
	else if (wcsstr(syscallName, RK25_SYSCALL_NAME_NTQSI)) {
		switch (osVersionCode)
		{
			// Windows XP
		case RK25_OS_VERSION_WIN_XP_SP1:
		case RK25_OS_VERSION_WIN_XP_SP2:
#if defined(_M_X64)
			return RK25_WIN_XP_X64_NTQSI;
#endif
		case RK25_OS_VERSION_WIN_XP_SP0:
		case RK25_OS_VERSION_WIN_XP_SP3:
#if defined(_M_X64)
			return RK25_UNSUPPORTED_NUMBER;
#else
			return RK25_WIN_XP_X86_NTQSI;
#endif

			// Windows Server 2003
		case RK25_OS_VERSION_WIN_SERVER_2003_SP0:
		case RK25_OS_VERSION_WIN_SERVER_2003_SP1:
		case RK25_OS_VERSION_WIN_SERVER_2003_SP2:
		case RK25_OS_VERSION_WIN_SERVER_2003_R2_SP1:
		case RK25_OS_VERSION_WIN_SERVER_2003_R2_SP2:
#if defined(_M_X64)
			return RK25_WIN_SERVER_2003_X64_NTQSI;
#else
			return RK25_WIN_SERVER_2003_X86_NTQSI;
#endif

			// Windows Server 2008
		case RK25_OS_VERSION_WIN_SERVER_2008_SP0:
		case RK25_OS_VERSION_WIN_SERVER_2008_SP1:
		case RK25_OS_VERSION_WIN_SERVER_2008_SP2:
			// Windows Vista
		case RK25_OS_VERSION_WIN_VISTA_SP0:
		case RK25_OS_VERSION_WIN_VISTA_SP1:
		case RK25_OS_VERSION_WIN_VISTA_SP2:
#if defined(_M_X64)
			return RK25_WIN_VISTA_X64_NTQSI;
#else
			return RK25_WIN_VISTA_X86_NTQSI;
#endif

			// Windows Server 2008 R2
		case RK25_OS_VERSION_WIN_SERVER_2008_R2_SP0:
		case RK25_OS_VERSION_WIN_SERVER_2008_R2_SP1:
			// Windows 7
		case RK25_OS_VERSION_WIN_7_SP0:
		case RK25_OS_VERSION_WIN_7_SP1:
#if defined(_M_X64)
			return RK25_WIN_7_X64_NTQSI;
#else
			return RK25_WIN_7_X86_NTQSI;
#endif

			// Windows Server 2012
		case RK25_OS_VERSION_WIN_SERVER_2012_SP0:
		case RK25_OS_VERSION_WIN_SERVER_2012_SP1:
			// Windows 8
		case RK25_OS_VERSION_WIN_8_0:
#if defined(_M_X64)
			return RK25_WIN_8_0_X64_NTQSI;
#else
			return RK25_WIN_8_0_X86_NTQSI;
#endif

			// Windows Server 2012
		case RK25_OS_VERSION_WIN_SERVER_2012_R2:
			// Windows 8.1
		case RK25_OS_VERSION_WIN_8_1:
#if defined(_M_X64)
			return RK25_WIN_8_1_X64_NTQSI;
#else
			return RK25_WIN_8_1_X86_NTQSI;
#endif

			// Windows 10
		case RK25_OS_VERSION_WIN_10_1507:
		case RK25_OS_VERSION_WIN_10_1511:
#if defined(_M_IX86)
			return RK25_WIN_10_X86_NTQSI_1;
#endif
		case RK25_OS_VERSION_WIN_SERVER_2016_LTSC_1607: // Windows Server 2016
		case RK25_OS_VERSION_WIN_10_1607:
#if defined(_M_IX86)
			return RK25_WIN_10_X86_NTQSI_2;
#endif
		case RK25_OS_VERSION_WIN_10_1703:
#if defined(_M_IX86)
			return RK25_WIN_10_X86_NTQSI_3;
#endif
			// Windows Server 2016
		case RK25_OS_VERSION_WIN_SERVER_2016_1709:
		case RK25_OS_VERSION_WIN_SERVER_2016_1803:
		case RK25_OS_VERSION_WIN_SERVER_2019_LTSC_1809:
		case RK25_OS_VERSION_WIN_SERVER_2019_1903:
		case RK25_OS_VERSION_WIN_SERVER_2019_1909:
		case RK25_OS_VERSION_WIN_SERVER_2019_2004:
		case RK25_OS_VERSION_WIN_SERVER_2019_20H2:
			// Windows 10
		case RK25_OS_VERSION_WIN_10_1709:
		case RK25_OS_VERSION_WIN_10_1803:
		case RK25_OS_VERSION_WIN_10_1809:
		case RK25_OS_VERSION_WIN_10_1903:
		case RK25_OS_VERSION_WIN_10_1909:
		case RK25_OS_VERSION_WIN_10_2004:
		case RK25_OS_VERSION_WIN_10_20H2:
		case RK25_OS_VERSION_WIN_10_21H1:
		case RK25_OS_VERSION_WIN_10_21H2:
		case RK25_OS_VERSION_WIN_10_22H2:
#if defined(_M_X64)
			return RK25_WIN_10_X64_NTQSI;
#endif
#if defined(_M_IX86)
			return RK25_WIN_10_X86_NTQSI_4;
#endif

#if defined(_M_X64)
			// Windows 11
		case RK25_OS_VERSION_WIN_11_21H2:
		case RK25_OS_VERSION_WIN_11_22H2:
		case RK25_OS_VERSION_WIN_11_23H2:
		case RK25_OS_VERSION_WIN_11_24H2:
			// Windows Server 2022
		case RK25_OS_VERSION_WIN_SERVER_2022_LTSC_21H2:
		case RK25_OS_VERSION_WIN_SERVER_2022_23H2:
			// Windows Server 2025
		case RK25_OS_VERSION_WIN_SERVER_2025_24H2:
			return RK25_WIN_11_NTQSI;
#endif
		default:
			return RK25_UNSUPPORTED_NUMBER;
		}
	}
	else return RK25_UNSUPPORTED_NUMBER;
}

BOOLEAN RK25WindowsVersionOnlyUp(ULONG osVersionCode) {
	if (glOsVersionCode == RK25_OS_VERSION_WIN_UNKNOWN)
		return FALSE;
	return glOsVersionCode >= osVersionCode ? TRUE : FALSE;
}