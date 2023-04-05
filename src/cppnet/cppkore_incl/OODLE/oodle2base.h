
//===================================================
// Oodle2 Base header
// (C) Copyright 1994-2021 Epic Games Tools LLC
//===================================================

#ifndef __OODLE2BASE_H_INCLUDED__
#define __OODLE2BASE_H_INCLUDED__

#ifndef OODLE2BASE_PUBLIC_HEADER
#define OODLE2BASE_PUBLIC_HEADER 1
#endif

#ifdef _MSC_VER
#pragma pack(push, Oodle, 8)

#pragma warning(push)
#pragma warning(disable : 4127) // conditional is constant
#endif

#ifndef OODLE_BASE_TYPES_H
#define OODLE_BASE_TYPES_H

#include <stdint.h>

#define OOCOPYRIGHT "Copyright (C) 1994-2021, Epic Games Tools LLC"

// Typedefs
typedef int8_t OO_S8;
typedef uint8_t OO_U8;
typedef int16_t OO_S16;
typedef uint16_t OO_U16;
typedef int32_t OO_S32;
typedef uint32_t OO_U32;
typedef int64_t OO_S64;
typedef uint64_t OO_U64;
typedef float OO_F32;
typedef double OO_F64;
typedef intptr_t OO_SINTa;
typedef uintptr_t OO_UINTa;
typedef int32_t OO_BOOL;

// Struct packing handling and inlining
#if defined(__GNUC__) || defined(__clang__)
    #define OOSTRUCT struct __attribute__((__packed__))
    #define OOINLINEFUNC inline
#elif defined(_MSC_VER)
    // on VC++, we use pragmas for the struct packing
    #define OOSTRUCT struct
    #define OOINLINEFUNC __inline
#endif

// Linkage stuff
#if defined(_WIN32)
    #define OOLINK __stdcall
    #define OOEXPLINK __stdcall
#else
    #define OOLINK
    #define OOEXPLINK
#endif

// C++ name demangaling
#ifdef __cplusplus
    #define OODEFFUNC extern "C"
    #define OODEFSTART extern "C" {
    #define OODEFEND }
    #define OODEFAULT( val ) =val
#else
    #define OODEFFUNC
    #define OODEFSTART
    #define OODEFEND
    #define OODEFAULT( val )
#endif

// ========================================================
// Exported function declarations
#define OOEXPFUNC OODEFFUNC

//===========================================================================
// OO_STRING_JOIN joins strings in the preprocessor and works with LINESTRING
#define OO_STRING_JOIN(arg1, arg2)              OO_STRING_JOIN_DELAY(arg1, arg2)
#define OO_STRING_JOIN_DELAY(arg1, arg2)        OO_STRING_JOIN_IMMEDIATE(arg1, arg2)
#define OO_STRING_JOIN_IMMEDIATE(arg1, arg2)    arg1 ## arg2

//===========================================================================
// OO_NUMBERNAME is a macro to make a name unique, so that you can use it to declare
//    variable names and they won't conflict with each other
// using __LINE__ is broken in MSVC with /ZI , but __COUNTER__ is an MSVC extension that works

#ifdef _MSC_VER
  #define OO_NUMBERNAME(name) OO_STRING_JOIN(name,__COUNTER__)
#else
  #define OO_NUMBERNAME(name) OO_STRING_JOIN(name,__LINE__)
#endif

//===================================================================
// simple compiler assert
// this happens at declaration time, so if it's inside a function in a C file, drop {} around it
#ifndef OO_COMPILER_ASSERT
  #if defined(__clang__)
    #define OO_COMPILER_ASSERT_UNUSED __attribute__((unused))  // hides warnings when compiler_asserts are in a local scope
  #else
    #define OO_COMPILER_ASSERT_UNUSED
  #endif

  #define OO_COMPILER_ASSERT(exp)   typedef char OO_NUMBERNAME(_dummy_array) [ (exp) ? 1 : -1 ] OO_COMPILER_ASSERT_UNUSED
#endif


#endif



// Oodle2 base header

#ifndef OODLE2_PUBLIC_CORE_DEFINES
#define OODLE2_PUBLIC_CORE_DEFINES 1

#define OOFUNC1 OOEXPFUNC
#define OOFUNC2 OOEXPLINK
#define OOFUNCSTART
#define OODLE_CALLBACK  OOLINK

// Check build flags
    #if defined(OODLE_BUILDING_LIB) || defined(OODLE_BUILDING_DLL)
        #error Should not see OODLE_BUILDING set for users of oodle.h
    #endif

#ifndef NULL
#define NULL    (0)
#endif

// OODLE_MALLOC_MINIMUM_ALIGNMENT is 8 in 32-bit, 16 in 64-bit
#define OODLE_MALLOC_MINIMUM_ALIGNMENT  (2*sizeof(void *))

typedef void (OODLE_CALLBACK t_OodleFPVoidVoid)(void);
/* void-void callback func pointer
    takes void, returns void
*/

typedef void (OODLE_CALLBACK t_OodleFPVoidVoidStar)(void *);
/* void-void-star callback func pointer
    takes void pointer, returns void
*/

#define OODLE_JOB_MAX_DEPENDENCIES (4) /* Maximum number of dependencies Oodle will ever pass to a RunJob callback
*/

#define t_fp_Oodle_Job  t_OodleFPVoidVoidStar /* Job function pointer for Plugin Jobify system

    takes void pointer returns void
*/

#endif // OODLE2_PUBLIC_CORE_DEFINES

#ifdef _MSC_VER
#pragma warning(pop)
#pragma pack(pop, Oodle)
#endif

#endif // __OODLE2BASE_H_INCLUDED__
