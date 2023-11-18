/***
*cruntime.h - definitions specific to the target operating system and hardware
*
*       Copyright (c) Microsoft Corporation. All rights reserved.
*
*Purpose:
*       This header file contains widely used definitions specific to the
*       host operating system and hardware. It is included by every C source
*       and most every other header file.
*
*       [Internal]
*
****/

#pragma once

#ifndef _CRTBLD
/*
 * This is an internal C runtime header file. It is used when building
 * the C runtimes only. It is not to be used as a public header file.
 */
#error ERROR: Use of C runtime library internal header file.
#endif  /* _CRTBLD */

#if defined (_SYSCRT) && defined (_WIN64)
#define _USE_OLD_STDCPP 1
#endif  /* defined (_SYSCRT) && defined (_WIN64) */

#if defined (_M_X64) || defined (_M_ARM)
#define _UNALIGNED __unaligned
#elif defined (_M_IX86)
#define _UNALIGNED
#else  /* defined (_M_IX86) */
#error Unknown target architecture.
#endif  /* defined (_M_IX86) */

/*
 * Are the macro definitions below still needed in this file?
 */
