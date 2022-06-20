#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "native_client/src/trusted/service_runtime/nacl_config.h"
#include "native_client/src/trusted/service_runtime/include/bits/nacl_syscalls.h"
#include "native_client/src/trusted/service_runtime/sel_rt.h"
#include "native_client/src/trusted/dyn_ldr/dyn_ldr_test_structs.h"

#define EXIT_FROM_MAIN 0
#define EXIT_FROM_CALL 1

typedef int32_t (*SandboxExitType)(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);

typedef int32_t (*SandboxCallbackType)(uint32_t, nacl_reg_t*, uintptr_t);

void MakeNaClSysCall_exit_sandbox(uint32_t exitLocation,
  uint32_t register_ret_bottom, uint32_t register_ret_top,
  uint32_t register_float_ret_bottom, uint32_t register_float_ret_top
)
{
	((SandboxExitType)NACL_SYSCALL_ADDR(NACL_sys_exit_sandbox))(exitLocation, register_ret_bottom, register_ret_top, register_float_ret_bottom, register_float_ret_top);
}

//Specifically not making this a new function as this may add a new stack frame
#define MakeNaClSysCall_callback(slotNumber, parameterRegisters, retPtr) ((SandboxCallbackType)NACL_SYSCALL_ADDR(NACL_sys_callback))(slotNumber, parameterRegisters, retPtr)


//Add this to make sure the final application includes the asm file also
void exitFunctionWrapper(void);

void exitFunctionWrapperRef(void) {
	exitFunctionWrapper();
}


#if defined(_M_IX86) || defined(__i386__)
	//for 32 bit the parameters are on the stack
	#define generateCallbackFunc(num)                           \
	uint64_t callbackFunctionWrapper##num(void) {               \
		uint64_t retBuff = 0;                                   \
		MakeNaClSysCall_callback(num, 0, (uintptr_t) &retBuff); \
		return retBuff;                                         \
	}

	#define generateCallbackFuncFloat(num)                      \
	float callbackFunctionWrapper##num(void) {                  \
		uint64_t retBuff = 0;                                   \
		MakeNaClSysCall_callback(num, 0, (uintptr_t) &retBuff); \
		float ret = 0;                                          \
		memcpy(&ret, &retBuff, sizeof(float));                  \
		return ret;                                             \
	}
#elif defined(_M_X64) || defined(__x86_64__)
	//for 64 bit the parameters are in registers, which will get overwritten, so we need to save it
	//nacl does not allow 64 bit parameters to trusted code calls, so we just save the values in an array and pass it out as a 64 bit pointer
	#define generateCallbackFunc(num)                                   \
	uint64_t callbackFunctionWrapper##num(unsigned long long p0, unsigned long long p1, unsigned long long p2, unsigned long long p3, unsigned long long p4, unsigned long long p5) \
	{                                                                   \
			uint64_t retBuff = 0;                                       \
			nacl_reg_t parameterRegisters[6];                           \
			parameterRegisters[0] = p0;                                 \
			parameterRegisters[1] = p1;                                 \
			parameterRegisters[2] = p2;                                 \
			parameterRegisters[3] = p3;                                 \
			parameterRegisters[4] = p4;                                 \
			parameterRegisters[5] = p5;                                 \
			MakeNaClSysCall_callback(num, parameterRegisters, &retBuff);\
			return retBuff;                                             \
	}

	#define generateCallbackFuncFloat(num)                              \
	uint64_t callbackFunctionWrapper##num(unsigned long long p0, unsigned long long p1, unsigned long long p2, unsigned long long p3, unsigned long long p4, unsigned long long p5) \
	{                                                                   \
			uint64_t retBuff = 0;                                       \
			nacl_reg_t parameterRegisters[6];                           \
			parameterRegisters[0] = p0;                                 \
			parameterRegisters[1] = p1;                                 \
			parameterRegisters[2] = p2;                                 \
			parameterRegisters[3] = p3;                                 \
			parameterRegisters[4] = p4;                                 \
			parameterRegisters[5] = p5;                                 \
			MakeNaClSysCall_callback(num, parameterRegisters, &retBuff);\
			float ret = 0;                                              \
			memcpy(&ret, &retBuff, sizeof(float));                      \
			return ret;                                                 \
	}

#elif defined(__ARMEL__) || defined(__MIPSEL__)
	#error Unsupported platform!
#else
	#error Unknown platform!
#endif

generateCallbackFunc(0)
generateCallbackFunc(1)
generateCallbackFunc(2)
generateCallbackFunc(3)
generateCallbackFunc(4)
generateCallbackFunc(5)
generateCallbackFunc(6)
generateCallbackFuncFloat(7)

unsigned test_localMath(unsigned a, unsigned  b, unsigned c)
{
	unsigned ret;
	ret = (a*100) + (b * 10) + (c);
	return ret;
}

size_t test_localString(char* test)
{
	return strlen(test);
}

int test_checkStructSizes
(
	int size_DoubleAlign,
	int size_PointerSize,
	int size_IntSize,
	int size_LongSize,
	int size_LongLongSize
)
{
	int isEqual =
		size_DoubleAlign == sizeof(struct TestStructDoubleAlign) &&
		size_PointerSize == sizeof(struct TestStructPointerSize) &&
		size_IntSize == sizeof(struct TestStructIntSize) &&
		size_LongSize == sizeof(struct TestStructLongSize) &&
		size_LongLongSize == sizeof(struct TestStructLongLongSize);
	return isEqual;
}

typedef void (*IdentifyCallbackHelperType) (uint32_t, uint32_t, uint32_t,
		                                    uint32_t, uint32_t, uint32_t,
		                                    uint32_t, uint32_t, uint32_t, uint32_t);
void identifyCallbackOffsetHelper(IdentifyCallbackHelperType callback)
{
	callback(1, 2, 3, 4, 5, 6, 7, 8, 9, 10);
}

int threadMain(void)
{
	MakeNaClSysCall_exit_sandbox(EXIT_FROM_MAIN,
		0, 0, 0, 0 /* return values not used here */);
	return 0;
}

//fopen and fclose aren't used in this file
//so they are not included in the symbol table
//Small hack to ensure fopen and fclose remain in the symbol table
void* fopenCopy = (void *) fopen;
void* fcloseCopy = (void *) fclose;

int main(int argc, char** argv)
{
	MakeNaClSysCall_exit_sandbox(EXIT_FROM_MAIN,
		0, 0, 0, 0 /* return values not used here */);
	return 0;
}
