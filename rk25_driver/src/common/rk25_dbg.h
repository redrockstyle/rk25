#ifndef _RK25_DBG_H_
#define _RK25_DBG_H_

#include <wdm.h>

#if DBG
#define RK25_DBG_CODE(...)								{__VA_ARGS__}
#define RK25_DBG_PRINT(msg)								KdPrint((msg))
#define RK25_DBG_PRINTF(format, ...)					KdPrint((format, __VA_ARGS__))

#define RK25_DBG_ERR(prolog, msg)						RK25_DBG_PRINTF("[E] %s: %s\n", prolog, msg);
#define RK25_DBG_WARN(prolog, msg)						RK25_DBG_PRINTF("[W] %s: %s\n", prolog, msg);
#define RK25_DBG_INFO(prolog, msg)						RK25_DBG_PRINTF("[I] %s: %s\n", prolog, msg);

#define RK25_DBG_ERR_F(prolog, format, ...)				RK25_DBG_PRINTF("[E] %s: " format "\n", prolog, ##__VA_ARGS__)
#define RK25_DBG_WARN_F(prolog, format, ...)			RK25_DBG_PRINTF("[W] %s: " format "\n", prolog, ##__VA_ARGS__)
#define RK25_DBG_INFO_F(prolog, format, ...)			RK25_DBG_PRINTF("[I] %s: " format "\n", prolog, ##__VA_ARGS__)

#define RK25_DBG_EXCEPT_ERR_F(prolog, code, address)	RK25_DBG_ERR_F(prolog, "Thrown an exception code:%X address:0x%p", code, address)
#define RK25_DBG_EXCEPT(prolog, exceptionRecord)		RK25_DBG_EXCEPT_ERR_F(prolog, (exceptionRecord)->ExceptionCode, (exceptionRecord)->ExceptionAddress)
#else
#define RK25_DBG_CODE(...)
#define RK25_DBG_PRINT(msg)
#define RK25_DBG_PRINTF(format, ...)

#define RK25_DBG_ERR(prolog, msg)
#define RK25_DBG_WARN(prolog, msg)
#define RK25_DBG_INFO(prolog, msg)

#define RK25_DBG_ERR_F(prolog, format, ...)	
#define RK25_DBG_WARN_F(prolog, format, ...)
#define RK25_DBG_INFO_F(prolog, format, ...)

#define RK25_DBG_EXCEPT_ERR_F(prolog, code, address)
#define RK25_DBG_EXCEPT(prolog, exceptionRecord)
#endif

#define POSIX_CODE_WIN



#endif // !_RK25_DBG_H_
