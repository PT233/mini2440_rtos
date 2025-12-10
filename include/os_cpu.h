#ifndef __OS_CPU_H__
#define __OS_CPU_H__

/*
*********************************************************************************************************
*                                              DATA TYPES
*                                         (Compiler Specific)
*********************************************************************************************************
*/

typedef unsigned char  BOOLEAN;
typedef unsigned char  INT8U;                    /* Unsigned  8 bit quantity                           */
typedef signed   char  INT8S;                    /* Signed    8 bit quantity                           */
typedef unsigned short INT16U;                   /* Unsigned 16 bit quantity                           */
typedef signed   short INT16S;                   /* Signed   16 bit quantity                           */
typedef unsigned int   INT32U;                   /* Unsigned 32 bit quantity                           */
typedef signed   int   INT32S;                   /* Signed   32 bit quantity                           */
typedef float          FP32;                     /* Single precision floating point                    */
typedef double         FP64;                     /* Double precision floating point                    */

typedef unsigned int   OS_STK;                   /* Each stack entry is 32-bit wide                    */
typedef unsigned int   OS_CPU_SR;                /* Define size of CPU status register (PSR = 32 bits) */

/* 
*********************************************************************************************************
*                                              Mini2440 Specific Types
*********************************************************************************************************
*/
typedef unsigned char  uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int   uint32_t;
typedef signed char    int8_t;
typedef signed short   int16_t;
typedef signed int     int32_t;

#define BYTE           INT8S
#define UBYTE          INT8U
#define WORD           INT16S
#define UWORD          INT16U
#define DWORD          INT32S
#define UDWORD         INT32U
#define LONG           INT32S
#define ULONG          INT32U

typedef INT32U         OS_STK;   /* Alias for OS_STK for compatibility */
typedef INT32U         OS_FLAGS;   /* Date type for event flag bits      */

/*
*********************************************************************************************************
*                                              PROTOTYPES
*********************************************************************************************************
*/

OS_CPU_SR  OS_CPU_SR_Save(void);
void       OS_CPU_SR_Restore(OS_CPU_SR cpu_sr);

void       OSCtxSw(void);
void       OSIntCtxSw(void);
void       OSStartHighRdy(void);

OS_STK     *OSTaskStkInit(void (*task)(void *p_arg), void *p_arg, OS_STK *ptos, INT16U opt);

/*
*********************************************************************************************************
*                                          CRITICAL SECTIONS
*********************************************************************************************************
*/

#define     OS_ENTER_CRITICAL()   (cpu_sr = OS_CPU_SR_Save())    /* Disable interrupts                        */
#define     OS_EXIT_CRITICAL()    (OS_CPU_SR_Restore(cpu_sr))    /* Restore  interrupts                        */

#endif // __OS_CPU_H__
