#ifndef OS_DATATYPE_H
#define OS_DATATYPE_H
//把OS_CPU.h的数据类型改写为mini2440的数据类型；把OS_STACK改为32位...
typedef unsigned char  uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int   uint32_t;
typedef signed char    int8_t;
typedef signed short   int16_t;
typedef signed int     int32_t;

//U:unsigned S:signed
typedef uint8_t     BOOLEAN;
typedef uint8_t     INT8U;
typedef int8_t      INT8S;
typedef uint16_t    INT16U;
typedef int16_t     INT16S;
typedef uint32_t    INT32U;
typedef int32_t     INT32S;

typedef float       FP32;
typedef double      FP64;


//D:double
#define BYTE        INT8S
#define UBYTE       INT8U
#define WORD        INT16S
#define UWORD       INT16U
#define DWORD       INT32S
#define UDWORD      INT32U
#define LONG        INT32S
#define ULONG       INT32U

typedef INT32U      OS_STACK;   /* 定义任务栈数据类型 */
typedef INT32U      OS_CPU_SR;  /* 定义 CPU 状态寄存器的大小 */
typedef INT32U      OS_FLAGS;   /* 定义事件标志的数据类型 */

#endif // OS_DATATYPE_H
