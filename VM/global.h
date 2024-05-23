//////////////////////////////////////////////////////////////////////
//2024 Copyright David Martin. All Rights Reserved. 
//
/////////////////////////////////////////////////////////////////////
#pragma once
#include <stdlib.h>
#include <stdio.h>
//#include <string.h>
//#include <math.h>
//#include <stdarg.h>
#include <conio.h>
#include <iostream>



// The following Windows-specific includes are only here to implement GetCurrTime (); these
// can be replaced when implementing the XVM on non-Windows platforms.

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

using namespace std;


#define MAX_COERCION_STRING_SIZE    64          // The maximum allocated space for a
// string coercion
#define XS_GLOBAL_FUNC              -1          // Flags a host API function as being
// global

#define DEF_STACK_SIZE			    1024	    // The default stack size

#define EXEC_FILE_EXT			    ".VSE"	    // Executable file extension

#define XSE_ID_STRING               "XSE0"      // Used to validate an .XSE executable

#define MAX_THREAD_COUNT		    1024        // The maximum number of scripts that
// can be loaded at once. Change this
// to support more or less.

#define MAX_FUNC_NAME_SIZE          256         // Maximum size of a function's name

#define MAX_HOST_API_SIZE           1024        // Maximum number of functions in the

#define THREAD_MODE_MULTI           0           // Multithreaded execution
#define THREAD_MODE_SINGLE          1           // Single-threaded execution

#define THREAD_PRIORITY_DUR_LOW     20          // Low-priority thread timeslice
#define THREAD_PRIORITY_DUR_MED     40          // Medium-priority thread timeslice
#define THREAD_PRIORITY_DUR_HIGH    80          // High-priority thread timeslice

#define XS_LOAD_OK					0			// Load successful
#define XS_LOAD_ERROR_FILE_IO  	    1			// File I/O error (most likely a file
												// not found error
#define XS_LOAD_ERROR_INVALID_XSE	    2		// Invalid .XSE structure
#define XS_LOAD_ERROR_UNSUPPORTED_VERS	3		// The format version is unsupported
#define XS_LOAD_ERROR_OUT_OF_MEMORY	    4		// Out of memory
#define XS_LOAD_ERROR_OUT_OF_THREADS	5		// Out of threads

#define XS_THREAD_PRIORITY_USER     0           // User-defined priority
#define XS_THREAD_PRIORITY_LOW      1           // Low priority
#define XS_THREAD_PRIORITY_MED      2           // Medium priority
#define XS_THREAD_PRIORITY_HIGH     3           // High priority

#define XS_INFINITE_TIMESLICE       -1          // Allows a thread to run indefinitely

#define OP_TYPE_NULL                -1          // Uninitialized/Null data
#define OP_TYPE_INT                 0           // Integer literal value
#define OP_TYPE_FLOAT               1           // Floating-point literal value
#define OP_TYPE_STRING		        2           // String literal value
#define OP_TYPE_ABS_STACK_INDEX     3           // Absolute array index
#define OP_TYPE_REL_STACK_INDEX     4           // Relative array index
#define OP_TYPE_INSTR_INDEX         5           // Instruction index
#define OP_TYPE_FUNC_INDEX          6           // Function index
#define OP_TYPE_HOST_API_CALL_INDEX 7           // Host API call index
#define OP_TYPE_REG                 8           // Register

#define OP_TYPE_STACK_BASE_MARKER   9           // Marks a stack base

#define INSTR_MOV                   0

#define INSTR_ADD                   1
#define INSTR_SUB                   2
#define INSTR_MUL                   3
#define INSTR_DIV                   4
#define INSTR_MOD                   5
#define INSTR_EXP                   6
#define INSTR_NEG                   7
#define INSTR_INC                   8
#define INSTR_DEC                   9

#define INSTR_AND                   10
#define INSTR_OR                    11
#define INSTR_XOR                   12
#define INSTR_NOT                   13
#define INSTR_SHL                   14
#define INSTR_SHR                   15

#define INSTR_CONCAT                16
#define INSTR_GETCHAR               17
#define INSTR_SETCHAR               18

#define INSTR_JMP                   19
#define INSTR_JE                    20
#define INSTR_JNE                   21
#define INSTR_JG                    22
#define INSTR_JL                    23
#define INSTR_JGE                   24
#define INSTR_JLE                   25

#define INSTR_PUSH                  26
#define INSTR_POP                   27
////next OP to add
#define INSTR_CALL                  28
#define INSTR_RET                   29
#define INSTR_CALLHOST              30

#define INSTR_PAUSE                 31
#define INSTR_EXIT                  32
			

// host API

typedef void(*HostAPIFuncPntr) (int iThreadIndex);  // Host API function pointer
																  // alias

typedef struct _HostAPIFunc                     // Host API function
{
	int iIsActive;                              // Is this slot in use?

	int iThreadIndex;                           // The thread to which this function
												// is visible
	char * pstrName;                            // The function name
	HostAPIFuncPntr fnFunc;                     // Pointer to the function definition
}
HostAPIFunc;

typedef struct _HostAPICallTable				// A host API call table
{
	char ** ppstrCalls;							// Pointer to the call array
	int iSize;									// The number of calls in the array
}
HostAPICallTable;

typedef struct _Func							// A function
{
	int iEntryPoint;							// The entry point
	int iParamCount;							// The parameter count
	int iLocalDataSize;							// Total size of all local data
	int iStackFrameSize;						// Total size of the stack frame
	char pstrName[MAX_FUNC_NAME_SIZE + 1];   // The function's name
}
Func;

typedef struct _FuncTable                       // A function table
{
	Func * pFuncs;                              // Pointer to the function array
	int iSize;                                  // The number of functions in the array
}
FuncTable;


typedef struct _Value							// A runtime value
{
	int iType;                                  // Type
	int iIntLiteral;                        // Integer literal
	float fFloatLiteral;                    // Float literal
	union                                       // The value
	{

		char * pstrStringLiteral;				// String literal
		int iStackIndex;                        // Stack Index
		int iInstrIndex;                        // Instruction index
		int iFuncIndex;                         // Function index
		int iHostAPICallIndex;                  // Host API Call index
		int iReg;                               // Register code
	};
	int iOffsetIndex;                           // Index of the offset
}
Value;

typedef struct _RuntimeStack					// A runtime stack
{
	Value * pElmnts;							// The stack elements
	int iSize;									// The number of elements in the stack

	int iTopIndex;								// The top index
	int iFrameIndex;                            // Index of the top of the current
												// stack frame.
}
RuntimeStack;

typedef struct _Instr                           // An instruction
{
	int iOpcode;                                // The opcode
	int iOpCount;                               // The number of operands
	Value * pOpList;                            // The operand list
}
Instr;

typedef struct _InstrStream                     // An instruction stream
{
	Instr * pInstrs;							// The instructions themselves
	int iSize;                                  // The number of instructions in the
												// stream
	int iCurrInstr;                             // The instruction pointer
}
InstrStream;




typedef struct _Script							// Encapsulates a full script
{
	int iIsActive;								// Is this script structure in use?

	// Header data

	int iGlobalDataSize;						// The size of the script's global data
	int iIsMainFuncPresent;                     // Is _Main () present?
	int iMainFuncIndex;							// _Main ()'s function index

	// Runtime tracking

	int iIsRunning;								// Is the script running?
	int iIsPaused;								// Is the script currently paused?
	int iPauseEndTime;			                // If so, when should it resume?

	// Threading

	int iTimesliceDur;                          // The thread's timeslice duration

	// Register file

	Value _RetVal;								// The _RetVal register

	// Script data

	InstrStream InstrStream;                    // The instruction stream
	RuntimeStack Stack;                         // The runtime stack
	FuncTable FuncTable;                        // The function table
	HostAPICallTable HostAPICallTable;			// The host API call table
}
Script;


Script g_Scripts[MAX_THREAD_COUNT];
HostAPIFunc g_HostAPI[MAX_HOST_API_SIZE];    // The host API

int g_iCurrThreadMode;                          // The current threading mode
int g_iCurrThread;								// The currently running thread
int g_iCurrThreadActiveTime;					// The time at which the current thread
												// was activated

void CopyValue(Value * pDest, Value Source);
Value ResolveOpValue(int iOpIndex);
int CoerceValueToInt(Value Val);
float CoerceValueToFloat(Value Val);
int ResolveStackIndex(int iIndex);
void SetStackValue(int iThreadIndex, int iIndex, Value Val);
void Push(int iThreadIndex, Value Val);
void PushFrame(int iThreadIndex, int iSize);
int globalExit = false;
