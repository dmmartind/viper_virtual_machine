//////////////////////////////////////////////////////////////////////
//2024 Copyright David Martin. All Rights Reserved. 
//
/////////////////////////////////////////////////////////////////////
#pragma once

#include "global.h"

void PopFrame(int iSize)
{
	//decrement the top index by the size of the frame
	//move the frame index to the new top of the stack
	g_Scripts[g_iCurrThread].Stack.iTopIndex -= iSize;
}

int ResolveOpAsFuncIndex(int iOpIndex)
{
	//resolve the operand's value
	Value OpValue = ResolveOpValue(iOpIndex);

	//return the function index
	return OpValue.iFuncIndex;
}

Func GetFunc(int iThreadIndex, int iIndex)
{
	return g_Scripts[iThreadIndex].FuncTable.pFuncs[iIndex];
}



void CallFunc(int iThreadIndex, int iIndex)
{
	Func DestFunc = GetFunc(iThreadIndex, iIndex);

	//save the current stack frame index

	int iFrameIndex = g_Scripts[iThreadIndex].Stack.iFrameIndex;

	//push the return address

	Value ReturnAddr;
	ReturnAddr.iInstrIndex = g_Scripts[iThreadIndex].InstrStream.iCurrInstr;
	Push(iThreadIndex, ReturnAddr);

	//push the stack frme + 1 (the extra space is for the functiuon index

	PushFrame(iThreadIndex, DestFunc.iLocalDataSize + 1);

	//Write the function index and old stack frame to the top of the stack

	Value FuncIndex;
	FuncIndex.iFuncIndex = iIndex;
	FuncIndex.iOffsetIndex = iFrameIndex;
	SetStackValue(iThreadIndex, g_Scripts[iThreadIndex].Stack.iTopIndex - 1, FuncIndex);

	//let the caller mke the jump to the entry point
	g_Scripts[iThreadIndex].InstrStream.iCurrInstr = DestFunc.iEntryPoint;

}


float CoerceValueToFloat(Value Val)
{
	//Determine which type the Value currently is
	switch (Val.iType)
	{
		//its an integer, so cast it to a float
	case OP_TYPE_INT:
	{
		return (float)Val.iIntLiteral;
		break;
	}
	case OP_TYPE_FLOAT:
	{
		return Val.fFloatLiteral;
		break;
	}
	case OP_TYPE_STRING:
	{
		return (float)atof(Val.pstrStringLiteral);
		break;
	}
	default:
		return 0;
		break;
	}
}

int ResolveOpAsInt(int iOpIndex)
{
	//Resolve the operand's value
	Value OpValue = ResolveOpValue(iOpIndex);
	//Coerce it to an int and return it

	int iInt = CoerceValueToInt(OpValue);
	return iInt;

}

float ResolveOpAsFloat(int iOpIndex)
{
	//resolve the ops value
	Value OpValue = ResolveOpValue(iOpIndex);

	//coerce it to a float and return it
	float fFloat = CoerceValueToFloat(OpValue);
	return fFloat;
}


//-------------------------------------------------------------------------------------
void XS_UnloadScript(int iThreadIndex)
{
	// Exit if the script isn't active

	if (!g_Scripts[iThreadIndex].iIsActive)
		return;

	// ---- Free The instruction stream

	// First check to see if any instructions have string operands, and free them if they
	// do

	for (int iCurrInstrIndex = 0; iCurrInstrIndex < g_Scripts[iThreadIndex].InstrStream.iSize; ++iCurrInstrIndex)
	{
		// Make a local copy of the operand count and operand list

		int iOpCount = g_Scripts[iThreadIndex].InstrStream.pInstrs[iCurrInstrIndex].iOpCount;
		Value * pOpList = g_Scripts[iThreadIndex].InstrStream.pInstrs[iCurrInstrIndex].pOpList;

		// Loop through each operand and free its string pointer

		for (int iCurrOpIndex = 0; iCurrOpIndex < iOpCount; ++iCurrOpIndex)
			if (pOpList[iCurrOpIndex].pstrStringLiteral)
				pOpList[iCurrOpIndex].pstrStringLiteral;
	}

	// Now free the stream itself

	if (g_Scripts[iThreadIndex].InstrStream.pInstrs)
		free(g_Scripts[iThreadIndex].InstrStream.pInstrs);

	// ---- Free the runtime stack

	// Free any strings that are still on the stack

	for (int iCurrElmtnIndex = 0; iCurrElmtnIndex < g_Scripts[iThreadIndex].Stack.iSize; ++iCurrElmtnIndex)
		if (g_Scripts[iThreadIndex].Stack.pElmnts[iCurrElmtnIndex].iType == OP_TYPE_STRING)
			free(g_Scripts[iThreadIndex].Stack.pElmnts[iCurrElmtnIndex].pstrStringLiteral);

	// Now free the stack itself

	if (g_Scripts[iThreadIndex].Stack.pElmnts)
		free(g_Scripts[iThreadIndex].Stack.pElmnts);

	// ---- Free the function table

	if (g_Scripts[iThreadIndex].FuncTable.pFuncs)
		free(g_Scripts[iThreadIndex].FuncTable.pFuncs);

	// --- Free the host API call table

	// First free each string in the table individually

	for (int iCurrCallIndex = 0; iCurrCallIndex < g_Scripts[iThreadIndex].HostAPICallTable.iSize; ++iCurrCallIndex)
		if (g_Scripts[iThreadIndex].HostAPICallTable.ppstrCalls[iCurrCallIndex])
			free(g_Scripts[iThreadIndex].HostAPICallTable.ppstrCalls[iCurrCallIndex]);

	// Now free the table itself

	if (g_Scripts[iThreadIndex].HostAPICallTable.ppstrCalls)
		free(g_Scripts[iThreadIndex].HostAPICallTable.ppstrCalls);
}






void XS_ShutDown()
{
	// ---- Unload any scripts that may still be in memory

	for (int iCurrScriptIndex = 0; iCurrScriptIndex < MAX_THREAD_COUNT; ++iCurrScriptIndex)
		XS_UnloadScript(iCurrScriptIndex);

	// ---- Free the host API's function name strings

	for (int iCurrHostAPIFunc = 0; iCurrHostAPIFunc < MAX_HOST_API_SIZE; ++iCurrHostAPIFunc)
		if (g_HostAPI[iCurrHostAPIFunc].pstrName)
			free(g_HostAPI[iCurrHostAPIFunc].pstrName);
}




//---------------------------------------------------------------------------------

char* GetHostAPICall(int iIndex)
{
	return g_Scripts[g_iCurrThread].HostAPICallTable.ppstrCalls[iIndex];
}

int ResolveOpAsInstrIndex(int iOpIndex)
{
	//resolve the operands value
	Value OpValue = ResolveOpValue(iOpIndex);

	//return it's instruction index
	return OpValue.iInstrIndex;
}

int ResolveStackIndex(int iIndex)
{
	int test = (iIndex < 0 ? iIndex += g_Scripts[g_iCurrThread].Stack.iFrameIndex : iIndex);
	return test;
}

void SetStackValue(int iThreadIndex, int iIndex, Value Val)
{
	//use ResolveStackIndex to set the element at the specified index
	g_Scripts[iThreadIndex].Stack.pElmnts[ResolveStackIndex(iIndex)] = Val;
}

Value GetStackValue(int iThreadIndex, int iIndex)
{
	//ResolveStackIndex return the element at the specified index
	return g_Scripts[iThreadIndex].Stack.pElmnts[ResolveStackIndex(iIndex)];
}

int GetOpType(int iOpIndex)
{
	//get the curr instr
	int iCurrInstr = g_Scripts[g_iCurrThread].InstrStream.iCurrInstr;

	//return the type

	return g_Scripts[g_iCurrThread].InstrStream.pInstrs[iCurrInstr].pOpList[iOpIndex].iType;
}

int ResolveOpStackIndex(int iOpIndex)
{
	//get the current Instruction
	int iCurrInstr = g_Scripts[g_iCurrThread].InstrStream.iCurrInstr;

	//get the current op type
	Value OpValue = g_Scripts[g_iCurrThread].InstrStream.pInstrs[iCurrInstr].pOpList[iOpIndex];

	//resolve the stack index on its type
	switch (OpValue.iType)
	{
		case OP_TYPE_ABS_STACK_INDEX:
		{
			return OpValue.iStackIndex;
			break;
		}
		case OP_TYPE_REL_STACK_INDEX:
		{
			//first get the base index
			int iBaseIndex = OpValue.iStackIndex;

			//now get the index of the var
			int iOffsetIndex = OpValue.iOffsetIndex;

			//get the variable's value
			Value StackValue = GetStackValue(g_iCurrThread, iOffsetIndex);

			//add the var integer to the base index to get the absolute index
			return iBaseIndex + StackValue.iIntLiteral;
			break;
		}
		default:
			return 0;
			break;
	}


}


Value* ResolveOpPntr(int iOpIndex)
{
	//get the method of indirect
	int iIndirMethod = GetOpType(iOpIndex);

	//returns a pointer to wherever the operand lies

	switch (iIndirMethod)
	{
	case OP_TYPE_ABS_STACK_INDEX:
	case OP_TYPE_REL_STACK_INDEX:
	{
		int iStackIndex = ResolveOpStackIndex(iOpIndex);
		return &g_Scripts[g_iCurrThread].Stack.pElmnts[ResolveStackIndex(iStackIndex)];
	}
	}


}


void CopyValue(Value* pDest, Value Source)
{
	//if the dest is of type string, free
	if (pDest->iType == OP_TYPE_STRING)
		free(pDest->pstrStringLiteral);
	//cp object to dest
	*pDest = Source;

	//make a physical copy
	if (Source.iType == OP_TYPE_STRING)
	{
		pDest->pstrStringLiteral = (char*)malloc(strlen(Source.pstrStringLiteral) + 1);
		strcpy(pDest->pstrStringLiteral, Source.pstrStringLiteral);
	}
}


void Push(int iThreadIndex, Value Val)
{
	//get the current top element
	int iTopIndex = g_Scripts[iThreadIndex].Stack.iTopIndex;
	//put the value into the current top index
	CopyValue(&g_Scripts[iThreadIndex].Stack.pElmnts[iTopIndex], Val);
	++g_Scripts[iThreadIndex].Stack.iTopIndex;
}

Value Pop(int iThreadIndex)
{
	//decrement the top index to clear the old element
	--g_Scripts[iThreadIndex].Stack.iTopIndex;

	//get the current top element
	int iTopIndex = g_Scripts[iThreadIndex].Stack.iTopIndex;

	//use this index to read the top element
	Value Val;
	CopyValue(&Val, g_Scripts[iThreadIndex].Stack.pElmnts[iTopIndex]);

	//return the value to the caller
	return Val;
}

Value ResolveOpValue(int iOpIndex)
{
	//get inst
	int iCurrInstr = g_Scripts[g_iCurrThread].InstrStream.iCurrInstr;
	//get op type
	Value OpValue = g_Scripts[g_iCurrThread].InstrStream.pInstrs[iCurrInstr].pOpList[iOpIndex];

	//return based on type
	switch (OpValue.iType)
	{
	case OP_TYPE_ABS_STACK_INDEX:
	case OP_TYPE_REL_STACK_INDEX:
	{
		int iAbsIndex = ResolveOpStackIndex(iOpIndex);
		return GetStackValue(g_iCurrThread, iAbsIndex);
		break;
	}
	// It's in _RetVal

	case OP_TYPE_REG:
		return g_Scripts[g_iCurrThread]._RetVal;
	default:
		return OpValue;

	}

}

bool IsValidThreadIndex(int iIndex) {
	return (iIndex < 0 || iIndex > MAX_THREAD_COUNT ? false :true);
}
                                          
        


bool IsThreadActive(int iIndex) {
	return (IsValidThreadIndex(iIndex) && g_Scripts[iIndex].iIsActive ? true : false);
}
        

/******************************************************************************************
	*
	*   CoereceValueToString ()
	*
	*   Coerces a Value structure from it's current type to a string value.
	*/

char * CoerceValueToString(Value Val)
{

	char * pstrCoercion;
	if (Val.iType != OP_TYPE_STRING)
		pstrCoercion = (char *)malloc(MAX_COERCION_STRING_SIZE + 1);

	// Determine which type the Value currently is

	switch (Val.iType)
	{
		// It's an integer, so convert it to a string

	case OP_TYPE_INT:
		itoa(Val.iIntLiteral, pstrCoercion, 10);
		return pstrCoercion;

		// It's a float, so use sprintf () to convert it since there's no built-in function
		// for converting floats to strings

	case OP_TYPE_FLOAT:
		sprintf(pstrCoercion, "%f", Val.fFloatLiteral);
		return pstrCoercion;

		// It's a string, so return it as-is

	case OP_TYPE_STRING:
		return Val.pstrStringLiteral;

		// Anything else is invalid

	default:
		return NULL;
	}
}


/******************************************************************************************
	*
	*   XS_GetParamAsString ()
	*
	*   Returns the specified string parameter to a host API function.
	*/

char * XS_GetParamAsString(int iThreadIndex, int iParamIndex)
{
	// Get the current top element

	int iTopIndex = g_Scripts[g_iCurrThread].Stack.iTopIndex;
	Value Param = g_Scripts[iThreadIndex].Stack.pElmnts[iTopIndex - (iParamIndex + 1)];

	// Coerce the top element of the stack to a string

	char * pstrString = CoerceValueToString(Param);

	// Return the value

	return pstrString;
}

/******************************************************************************************
	*
	*   XS_RegisterHostAPIFunc ()
	*
	*   Registers a function with the host API.
	*/

void XS_RegisterHostAPIFunc(int iThreadIndex, char * pstrName, HostAPIFuncPntr fnFunc)
{
	// Loop through each function in the host API until a free index is found

	for (int iCurrHostAPIFunc = 0; iCurrHostAPIFunc < MAX_HOST_API_SIZE; ++iCurrHostAPIFunc)
	{
		// If the current index is free, use it

		if (!g_HostAPI[iCurrHostAPIFunc].iIsActive)
		{
			// Set the function's parameters

			g_HostAPI[iCurrHostAPIFunc].iThreadIndex = iThreadIndex;
			g_HostAPI[iCurrHostAPIFunc].pstrName = (char *)malloc(strlen(pstrName) + 1);
			strcpy(g_HostAPI[iCurrHostAPIFunc].pstrName, pstrName);
			strupr(g_HostAPI[iCurrHostAPIFunc].pstrName);
			g_HostAPI[iCurrHostAPIFunc].fnFunc = fnFunc;

			// Set the function to active

			g_HostAPI[iCurrHostAPIFunc].iIsActive = true;
		}
	}
}


/******************************************************************************************
	*
	*	PushFrame ()
	*
	*	Pushes a stack frame.
	*/

void PushFrame(int iThreadIndex, int iSize)
{
	// Increment the top index by the size of the frame

	g_Scripts[iThreadIndex].Stack.iTopIndex += iSize;

	// Move the frame index to the new top of the stack

	g_Scripts[iThreadIndex].Stack.iFrameIndex = g_Scripts[iThreadIndex].Stack.iTopIndex;
}

/******************************************************************************************
	*
	*	XS_ResetScript ()
	*
	*	Resets the script. This function accepts a thread index rather than relying on the
	*	currently active thread, because scripts can (and will) need to be reset arbitrarily.
	*/

void XS_ResetScript(int iThreadIndex)
{
	// Get _Main ()'s function index in case we need it

	int iMainFuncIndex = g_Scripts[iThreadIndex].iMainFuncIndex;

	// If the function table is present, set the entry point

	if (g_Scripts[iThreadIndex].FuncTable.pFuncs)
	{
		// If _Main () is present, read _Main ()'s index of the function table to get its
		// entry point

		if (g_Scripts[iThreadIndex].iIsMainFuncPresent)
		{
			g_Scripts[iThreadIndex].InstrStream.iCurrInstr = g_Scripts[iThreadIndex].FuncTable.pFuncs[iMainFuncIndex].iEntryPoint;
		}
	}

	// Clear the stack

	g_Scripts[iThreadIndex].Stack.iTopIndex = 0;
	g_Scripts[iThreadIndex].Stack.iFrameIndex = 0;

	// Set the entire stack to null

	for (int iCurrElmntIndex = 0; iCurrElmntIndex < g_Scripts[iThreadIndex].Stack.iSize; ++iCurrElmntIndex)
		g_Scripts[iThreadIndex].Stack.pElmnts[iCurrElmntIndex].iType = OP_TYPE_NULL;

	// Unpause the script

	g_Scripts[iThreadIndex].iIsPaused = false;

	// Allocate space for the globals

	PushFrame(iThreadIndex, g_Scripts[iThreadIndex].iGlobalDataSize);

	// If _Main () is present, push its stack frame (plus one extra stack element to
	// compensate for the function index that usually sits on top of stack frames and
	// causes indices to start from -2)

	PushFrame(iThreadIndex, g_Scripts[iThreadIndex].FuncTable.pFuncs[iMainFuncIndex].iLocalDataSize + 1);
}


/******************************************************************************************
	*
	*	XS_Init ()
	*
	*	Initializes the runtime environment.
	*/

void XS_Init()
{
	// ---- Initialize the script array

	for (int iCurrScriptIndex = 0; iCurrScriptIndex < MAX_THREAD_COUNT; ++iCurrScriptIndex)
	{
		g_Scripts[iCurrScriptIndex].iIsActive = false;

		g_Scripts[iCurrScriptIndex].iIsRunning = false;
		g_Scripts[iCurrScriptIndex].iIsMainFuncPresent = false;
		g_Scripts[iCurrScriptIndex].iIsPaused = false;

		g_Scripts[iCurrScriptIndex].InstrStream.pInstrs = NULL;
		g_Scripts[iCurrScriptIndex].Stack.pElmnts = NULL;
		g_Scripts[iCurrScriptIndex].FuncTable.pFuncs = NULL;
		g_Scripts[iCurrScriptIndex].HostAPICallTable.ppstrCalls = NULL;
	}

	// ---- Initialize the host API

	for (int iCurrHostAPIFunc = 0; iCurrHostAPIFunc < MAX_HOST_API_SIZE; ++iCurrHostAPIFunc)
	{
		g_HostAPI[iCurrHostAPIFunc].iIsActive = false;
		g_HostAPI[iCurrHostAPIFunc].pstrName = NULL;
	}

	// ---- Set up the threads

	g_iCurrThreadMode = THREAD_MODE_MULTI;
	g_iCurrThread = 0;
}


/******************************************************************************************
	*
	*	XS_LoadScript ()
	*
	*	Loads an .XSE file into memory.
	*/

int XS_LoadScript(char * pstrFilename, int & iThreadIndex, int iThreadTimeslice)
{
	// ---- Find the next free script index

	int iFreeThreadFound = false;
	for (int iCurrThreadIndex = 0; iCurrThreadIndex < MAX_THREAD_COUNT; ++iCurrThreadIndex)
	{
		// If the current thread is not in use, use it

		if (!g_Scripts[iCurrThreadIndex].iIsActive)
		{
			iThreadIndex = iCurrThreadIndex;
			iFreeThreadFound = true;
			break;
		}
	}

	// If a thread wasn't found, return an out of threads error

	if (!iFreeThreadFound)
		return XS_LOAD_ERROR_OUT_OF_THREADS;

	// ---- Open the input file

	FILE * pScriptFile;
	if (!(pScriptFile = fopen(pstrFilename, "rb")))
		return XS_LOAD_ERROR_FILE_IO;

	// ---- Read the header

	// Create a buffer to hold the file's ID string (4 bytes + 1 null terminator = 5)

	char * pstrIDString;
	if (!(pstrIDString = (char *)malloc(5)))
		return XS_LOAD_ERROR_OUT_OF_MEMORY;

	// Read the string (4 bytes) and append a null terminator

	fread(pstrIDString, 4, 1, pScriptFile);
	pstrIDString[strlen(XSE_ID_STRING)] = '\0';

	// Compare the data read from the file to the ID string and exit on an error if they don't
	// match

	if (strcmp(pstrIDString, XSE_ID_STRING) != 0)
		return XS_LOAD_ERROR_INVALID_XSE;

	// Free the buffer

	free(pstrIDString);

	// Read the script version (2 bytes total)

	int iMajorVersion = 0,
		iMinorVersion = 0;

	fread(&iMajorVersion, 1, 1, pScriptFile);
	fread(&iMinorVersion, 1, 1, pScriptFile);

	// Validate the version, since this prototype only supports version 0.8 scripts

	if (iMajorVersion != 0 || iMinorVersion != 9)
		return XS_LOAD_ERROR_UNSUPPORTED_VERS;

	// Read the stack size (4 bytes)

	fread(&g_Scripts[iThreadIndex].Stack.iSize, 4, 1, pScriptFile);

	// Check for a default stack size request

	if (g_Scripts[iThreadIndex].Stack.iSize == 0)
		g_Scripts[iThreadIndex].Stack.iSize = DEF_STACK_SIZE;

	// Allocate the runtime stack

	int iStackSize = g_Scripts[iThreadIndex].Stack.iSize;
	if (!(g_Scripts[iThreadIndex].Stack.pElmnts = (Value *)malloc(iStackSize * sizeof(Value))))
		return XS_LOAD_ERROR_OUT_OF_MEMORY;

	// Read the global data size (4 bytes)

	fread(&g_Scripts[iThreadIndex].iGlobalDataSize, 4, 1, pScriptFile);

	// Check for presence of _Main () (1 byte)

	fread(&g_Scripts[iThreadIndex].iIsMainFuncPresent, 1, 1, pScriptFile);

	// Read _Main ()'s function index (4 bytes)

	fread(&g_Scripts[iThreadIndex].iMainFuncIndex, 4, 1, pScriptFile);

	// Read the priority type (1 byte)

	int iPriorityType = 0;
	fread(&iPriorityType, 1, 1, pScriptFile);

	// Read the user-defined priority (4 bytes)

	fread(&g_Scripts[iThreadIndex].iTimesliceDur, 4, 1, pScriptFile);

	// Override the script-specified priority if necessary

	if (iThreadTimeslice != XS_THREAD_PRIORITY_USER)
		iPriorityType = iThreadTimeslice;

	// If the priority type is not set to user-defined, fill in the appropriate timeslice
	// duration

	switch (iPriorityType)
	{
	case XS_THREAD_PRIORITY_LOW:
		g_Scripts[iThreadIndex].iTimesliceDur = THREAD_PRIORITY_DUR_LOW;
		break;

	case XS_THREAD_PRIORITY_MED:
		g_Scripts[iThreadIndex].iTimesliceDur = THREAD_PRIORITY_DUR_MED;
		break;

	case XS_THREAD_PRIORITY_HIGH:
		g_Scripts[iThreadIndex].iTimesliceDur = THREAD_PRIORITY_DUR_HIGH;
		break;
	}

	// ---- Read the instruction stream

	// Read the instruction count (4 bytes)

	fread(&g_Scripts[iThreadIndex].InstrStream.iSize, 4, 1, pScriptFile);

	// Allocate the stream

	if (!(g_Scripts[iThreadIndex].InstrStream.pInstrs = (Instr *)malloc(g_Scripts[iThreadIndex].InstrStream.iSize * sizeof(Instr))))
		return XS_LOAD_ERROR_OUT_OF_MEMORY;

	// Read the instruction data

	for (int iCurrInstrIndex = 0; iCurrInstrIndex < g_Scripts[iThreadIndex].InstrStream.iSize; ++iCurrInstrIndex)
	{
		// Read the opcode (2 bytes)

		g_Scripts[iThreadIndex].InstrStream.pInstrs[iCurrInstrIndex].iOpcode = 0;
		fread(&g_Scripts[iThreadIndex].InstrStream.pInstrs[iCurrInstrIndex].iOpcode, 2, 1, pScriptFile);

		// Read the operand count (1 byte)

		g_Scripts[iThreadIndex].InstrStream.pInstrs[iCurrInstrIndex].iOpCount = 0;
		fread(&g_Scripts[iThreadIndex].InstrStream.pInstrs[iCurrInstrIndex].iOpCount, 1, 1, pScriptFile);

		int iOpCount = g_Scripts[iThreadIndex].InstrStream.pInstrs[iCurrInstrIndex].iOpCount;

		// Allocate space for the operand list in a temporary pointer

		Value * pOpList;
		if (!(pOpList = (Value *)malloc(iOpCount * sizeof(Value))))
			return XS_LOAD_ERROR_OUT_OF_MEMORY;

		// Read in the operand list (N bytes)

		for (int iCurrOpIndex = 0; iCurrOpIndex < iOpCount; ++iCurrOpIndex)
		{
			// Read in the operand type (1 byte)

			pOpList[iCurrOpIndex].iType = 0;
			fread(&pOpList[iCurrOpIndex].iType, 1, 1, pScriptFile);

			// Depending on the type, read in the operand data

			switch (pOpList[iCurrOpIndex].iType)
			{
				// Integer literal

			case OP_TYPE_INT:
				fread(&pOpList[iCurrOpIndex].iIntLiteral, sizeof(int), 1, pScriptFile);
				break;

				// Floating-point literal

			case OP_TYPE_FLOAT:
				fread(&pOpList[iCurrOpIndex].fFloatLiteral, sizeof(float), 1, pScriptFile);
				break;

				// String index

			case OP_TYPE_STRING:

				// Since there's no field in the Value structure for string table
				// indices, read the index into the integer literal field and set
				// its type to string index

				fread(&pOpList[iCurrOpIndex].iIntLiteral, sizeof(int), 1, pScriptFile);
				pOpList[iCurrOpIndex].iType = OP_TYPE_STRING;
				break;

				// Instruction index

			case OP_TYPE_INSTR_INDEX:
				fread(&pOpList[iCurrOpIndex].iInstrIndex, sizeof(int), 1, pScriptFile);
				break;

				// Absolute stack index

			case OP_TYPE_ABS_STACK_INDEX:
				fread(&pOpList[iCurrOpIndex].iStackIndex, sizeof(int), 1, pScriptFile);
				break;

				// Relative stack index

			case OP_TYPE_REL_STACK_INDEX:
				fread(&pOpList[iCurrOpIndex].iStackIndex, sizeof(int), 1, pScriptFile);
				fread(&pOpList[iCurrOpIndex].iOffsetIndex, sizeof(int), 1, pScriptFile);
				break;

				// Function index

			case OP_TYPE_FUNC_INDEX:
				fread(&pOpList[iCurrOpIndex].iFuncIndex, sizeof(int), 1, pScriptFile);
				break;

				// Host API call index

			case OP_TYPE_HOST_API_CALL_INDEX:
				fread(&pOpList[iCurrOpIndex].iHostAPICallIndex, sizeof(int), 1, pScriptFile);
				break;

				// Register

			case OP_TYPE_REG:
				fread(&pOpList[iCurrOpIndex].iReg, sizeof(int), 1, pScriptFile);
				break;
			}
		}

		// Assign the operand list pointer to the instruction stream

		g_Scripts[iThreadIndex].InstrStream.pInstrs[iCurrInstrIndex].pOpList = pOpList;
	}

	// ---- Read the string table

	// Read the table size (4 bytes)

	int iStringTableSize;
	fread(&iStringTableSize, 4, 1, pScriptFile);

	// If the string table exists, read it

	if (iStringTableSize)
	{
		// Allocate a string table of this size

		char ** ppstrStringTable;
		if (!(ppstrStringTable = (char **)malloc(iStringTableSize * sizeof(char *))))
			return XS_LOAD_ERROR_OUT_OF_MEMORY;

		// Read in each string

		for (int iCurrStringIndex = 0; iCurrStringIndex < iStringTableSize; ++iCurrStringIndex)
		{
			// Read in the string size (4 bytes)

			int iStringSize;
			fread(&iStringSize, 4, 1, pScriptFile);

			// Allocate space for the string plus a null terminator

			char * pstrCurrString;
			if (!(pstrCurrString = (char *)malloc(iStringSize + 1)))
				return XS_LOAD_ERROR_OUT_OF_MEMORY;

			// Read in the string data (N bytes) and append the null terminator

			fread(pstrCurrString, iStringSize, 1, pScriptFile);
			pstrCurrString[iStringSize] = '\0';

			// Assign the string pointer to the string table

			ppstrStringTable[iCurrStringIndex] = pstrCurrString;
		}

		// Run through each operand in the instruction stream and assign copies of string
		// operand's corresponding string literals

		for (int iCurrInstrIndex = 0; iCurrInstrIndex < g_Scripts[iThreadIndex].InstrStream.iSize; ++iCurrInstrIndex)
		{
			// Get the instruction's operand count and a copy of it's operand list

			int iOpCount = g_Scripts[iThreadIndex].InstrStream.pInstrs[iCurrInstrIndex].iOpCount;
			Value * pOpList = g_Scripts[iThreadIndex].InstrStream.pInstrs[iCurrInstrIndex].pOpList;

			// Loop through each operand

			for (int iCurrOpIndex = 0; iCurrOpIndex < iOpCount; ++iCurrOpIndex)
			{
				// If the operand is a string index, make a local copy of it's corresponding
				// string in the table

				if (pOpList[iCurrOpIndex].iType == OP_TYPE_STRING)
				{
					// Get the string index from the operand's integer literal field

					int iStringIndex = pOpList[iCurrOpIndex].iIntLiteral;

					// Allocate a new string to hold a copy of the one in the table

					char * pstrStringCopy;
					if (!(pstrStringCopy = (char *)malloc(strlen(ppstrStringTable[iStringIndex]) + 1)))
						return XS_LOAD_ERROR_OUT_OF_MEMORY;

					// Make a copy of the string

					strcpy(pstrStringCopy, ppstrStringTable[iStringIndex]);

					// Save the string pointer in the operand list

					pOpList[iCurrOpIndex].pstrStringLiteral = pstrStringCopy;
				}
			}
		}

		// ---- Free the original strings
		int iCurrStringIndex = 0;
		for (iCurrStringIndex = 0; iCurrStringIndex < iStringTableSize; ++iCurrStringIndex)
			free(ppstrStringTable[iCurrStringIndex]);

		// ---- Free the string table itself

		free(ppstrStringTable);
	}

	// ---- Read the function table

	// Read the function count (4 bytes)

	int iFuncTableSize;
	fread(&iFuncTableSize, 4, 1, pScriptFile);

	g_Scripts[iThreadIndex].FuncTable.iSize = iFuncTableSize;

	// Allocate the table

	if (!(g_Scripts[iThreadIndex].FuncTable.pFuncs = (Func *)malloc(iFuncTableSize * sizeof(Func))))
		return XS_LOAD_ERROR_OUT_OF_MEMORY;

	// Read each function

	for (int iCurrFuncIndex = 0; iCurrFuncIndex < iFuncTableSize; ++iCurrFuncIndex)
	{
		// Read the entry point (4 bytes)

		int iEntryPoint;
		fread(&iEntryPoint, 4, 1, pScriptFile);

		// Read the parameter count (1 byte)

		int iParamCount = 0;
		fread(&iParamCount, 1, 1, pScriptFile);

		// Read the local data size (4 bytes)

		int iLocalDataSize;
		fread(&iLocalDataSize, 4, 1, pScriptFile);

		// Calculate the stack size

		int iStackFrameSize = iParamCount + 1 + iLocalDataSize;

		// Read the function name length (1 byte)

		int iFuncNameLength = 0;
		fread(&iFuncNameLength, 1, 1, pScriptFile);

		// Read the function name (N bytes) and append a null-terminator

		fread(&g_Scripts[iThreadIndex].FuncTable.pFuncs[iCurrFuncIndex].pstrName, iFuncNameLength, 1, pScriptFile);
		g_Scripts[iThreadIndex].FuncTable.pFuncs[iCurrFuncIndex].pstrName[iFuncNameLength] = '\0';

		// Write everything to the function table

		g_Scripts[iThreadIndex].FuncTable.pFuncs[iCurrFuncIndex].iEntryPoint = iEntryPoint;
		g_Scripts[iThreadIndex].FuncTable.pFuncs[iCurrFuncIndex].iParamCount = iParamCount;
		g_Scripts[iThreadIndex].FuncTable.pFuncs[iCurrFuncIndex].iLocalDataSize = iLocalDataSize;
		g_Scripts[iThreadIndex].FuncTable.pFuncs[iCurrFuncIndex].iStackFrameSize = iStackFrameSize;
	}

	// ---- Read the host API call table

	// Read the host API call count

	fread(&g_Scripts[iThreadIndex].HostAPICallTable.iSize, 4, 1, pScriptFile);

	// Allocate the table

	if (!(g_Scripts[iThreadIndex].HostAPICallTable.ppstrCalls = (char **)malloc(g_Scripts[iThreadIndex].HostAPICallTable.iSize * sizeof(char *))))
		return XS_LOAD_ERROR_OUT_OF_MEMORY;

	// Read each host API call

	for (int iCurrCallIndex = 0; iCurrCallIndex < g_Scripts[iThreadIndex].HostAPICallTable.iSize; ++iCurrCallIndex)
	{
		// Read the host API call string size (1 byte)

		int iCallLength = 0;
		fread(&iCallLength, 1, 1, pScriptFile);

		// Allocate space for the string plus the null terminator in a temporary pointer

		char * pstrCurrCall;
		if (!(pstrCurrCall = (char *)malloc(iCallLength + 1)))
			return XS_LOAD_ERROR_OUT_OF_MEMORY;

		// Read the host API call string data and append the null terminator

		fread(pstrCurrCall, iCallLength, 1, pScriptFile);
		pstrCurrCall[iCallLength] = '\0';

		// Assign the temporary pointer to the table

		g_Scripts[iThreadIndex].HostAPICallTable.ppstrCalls[iCurrCallIndex] = pstrCurrCall;
	}

	// ---- Close the input file

	fclose(pScriptFile);

	// The script is fully loaded and ready to go, so set the active flag

	g_Scripts[iThreadIndex].iIsActive = true;

	// Reset the script

	XS_ResetScript(iThreadIndex);

	// Return a success code

	return XS_LOAD_OK;
}

/******************************************************************************************
	*
	*   CopyValue ()
	*
	*   Copies a value structure to another, taking strings into account.
	*/

/******************************************************************************************
	*
	*   XS_ReturnStringFromHost ()
	*
	*   Returns a string from a host API function.
	*/

void XS_ReturnStringFromHost(int iThreadIndex, int iParamCount, char * pstrString)
{
	// Clear the parameters off the stack

	g_Scripts[iThreadIndex].Stack.iTopIndex -= iParamCount;

	// Put the return value and type in _RetVal

	Value ReturnValue;
	ReturnValue.iType = OP_TYPE_STRING;
	ReturnValue.pstrStringLiteral = pstrString;
	CopyValue(&g_Scripts[iThreadIndex]._RetVal, ReturnValue);
}

void XS_ReturnString(int iThreadIndex, int iParamCount, char * pstrString)
{
	XS_ReturnStringFromHost(iThreadIndex, iParamCount, pstrString);
	return;
}

/******************************************************************************************
	*
	*   CoereceValueToInt ()
	*
	*   Coerces a Value structure from it's current type to an integer value.
	*/

int CoerceValueToInt(Value Val)
{
	// Determine which type the Value currently is

	switch (Val.iType)
	{
		// It's an integer, so return it as-is

	case OP_TYPE_INT:
		return Val.iIntLiteral;

		// It's a float, so cast it to an integer

	case OP_TYPE_FLOAT:
		return (int)Val.fFloatLiteral;

		// It's a string, so convert it to an integer

	case OP_TYPE_STRING:
		return atoi(Val.pstrStringLiteral);

		// Anything else is invalid

	default:
		return 0;
	}
}

/******************************************************************************************
	*
	*   XS_GetParamAsInt ()
	*
	*   Returns the specified integer parameter to a host API function.
	*/

int XS_GetParamAsInt(int iThreadIndex, int iParamIndex)
{
	// Get the current top element

	int iTopIndex = g_Scripts[g_iCurrThread].Stack.iTopIndex;
	Value Param = g_Scripts[iThreadIndex].Stack.pElmnts[iTopIndex - (iParamIndex + 1)];

	// Coerce the top element of the stack to an integer

	int iInt = CoerceValueToInt(Param);

	// Return the value

	return iInt;
}

/******************************************************************************************
	*
	*	XS_StartScript ()
	*
	*   Starts the execution of a script.
	*/

void XS_StartScript(int iThreadIndex)
{
	// Make sure the thread index is valid and active

	if (!IsThreadActive(iThreadIndex))
		return;

	// Set the thread's execution flag

	g_Scripts[iThreadIndex].iIsRunning = true;

	// Set the current thread to the script

	g_iCurrThread = iThreadIndex;

	// Set the activation time for the current thread to get things rolling

	g_iCurrThreadActiveTime = GetTickCount();
}

/******************************************************************************************
	*
	*	XS_RunScripts ()
	*
	*	Runs the currenty loaded script array for a given timeslice duration.
	*/

void XS_RunScripts(int iTimesliceDur)
{
	// Begin a loop that runs until a keypress. The instruction pointer has already been
		// initialized with a prior call to ResetScripts (), so execution can begin

		// Create a flag that instructions can use to break the execution loop

	int iExitExecLoop = FALSE;

	// Create a variable to hold the time at which the main timeslice started

	int iMainTimesliceStartTime = GetTickCount();

	// Create a variable to hold the current time

	int iCurrTime;

	while (TRUE)
	{
		// Check to see if all threads have terminated, and if so, break the execution
		// cycle

		int iIsStillActive = FALSE;
		for (int iCurrThreadIndex = 0; iCurrThreadIndex < MAX_THREAD_COUNT; ++iCurrThreadIndex)
		{
			if (g_Scripts[iCurrThreadIndex].iIsActive && g_Scripts[iCurrThreadIndex].iIsRunning)
				iIsStillActive = TRUE;
		}
		if (!iIsStillActive)
			break;

		// Update the current time

		iCurrTime = GetTickCount();

		// Check for a context switch if the threading mode is set for multithreading

		if (g_iCurrThreadMode == THREAD_MODE_MULTI)
		{
			// If the current thread's timeslice has elapsed, or if it's terminated switch
			// to the next valid thread

			if (iCurrTime > g_iCurrThreadActiveTime + g_Scripts[g_iCurrThread].iTimesliceDur ||
				!g_Scripts[g_iCurrThread].iIsRunning)
			{
				// Loop until the next thread is found

				while (TRUE)
				{
					// Move to the next thread in the array

					++g_iCurrThread;

					// If we're past the end of the array, loop back around

					if (g_iCurrThread >= MAX_THREAD_COUNT)
						g_iCurrThread = 0;

					// If the thread we've chosen is active and running, break the loop

					if (g_Scripts[g_iCurrThread].iIsActive && g_Scripts[g_iCurrThread].iIsRunning)
						break;
				}

				// Reset the timeslice

				g_iCurrThreadActiveTime = iCurrTime;
			}
		}

		// Is the script currently paused?

		if (g_Scripts[g_iCurrThread].iIsPaused)
		{
			// Has the pause duration elapsed yet?

			if (iCurrTime >= g_Scripts[g_iCurrThread].iPauseEndTime)
			{
				// Yes, so unpause the script

				g_Scripts[g_iCurrThread].iIsPaused = FALSE;
			}
			else
			{
				// No, so skip this iteration of the execution cycle

				continue;
			}
		}

		// Make a copy of the instruction pointer to compare later

		int iCurrInstr = g_Scripts[g_iCurrThread].InstrStream.iCurrInstr;

		// Get the current opcode

		int iOpcode = g_Scripts[g_iCurrThread].InstrStream.pInstrs[iCurrInstr].iOpcode;

		//if (g_iCurrThread == 0)
		//{
		//	printf("%d", iOpcode);
		//	printf("\n");
		//}

		switch (iOpcode)
		{

		case INSTR_MOV:

		case INSTR_ADD:
		case INSTR_SUB:
		case INSTR_MUL:
		case INSTR_DIV:
		case INSTR_MOD:
		case INSTR_EXP:

		case INSTR_AND:
		case INSTR_OR:
		case INSTR_XOR:
		case INSTR_SHL:
		case INSTR_SHR:
		{
			//get the local copy of the dest operand
			Value Dest = ResolveOpValue(0);
			//get copy of the source
			Value Source = ResolveOpValue(1);

			switch (iOpcode)
			{
			case INSTR_MOV:
			{
				//if both registers are the same, stop
				if (ResolveOpPntr(0) == ResolveOpPntr(1))
					break;

				//copy the source operand into the dest
				CopyValue(&Dest, Source);
				break;

			}
			case INSTR_ADD:
				if (Dest.iType == OP_TYPE_INT)
					Dest.iIntLiteral += ResolveOpAsInt(1);
				else
					Dest.fFloatLiteral += ResolveOpAsFloat(1);
				break;

			case INSTR_SUB:
			{
				if (Dest.iType == OP_TYPE_INT)
					Dest.iIntLiteral -= ResolveOpAsInt(1);
				else
					Dest.fFloatLiteral -= ResolveOpAsFloat(1);
				break;
			}
			case INSTR_MUL:
			{
				if (Dest.iType == OP_TYPE_INT)
					Dest.iIntLiteral *= ResolveOpAsInt(1);
				else
					Dest.fFloatLiteral *= ResolveOpAsFloat(1);
				break;
			}
			case INSTR_DIV:
			{
				if (Dest.iType == OP_TYPE_INT)
					Dest.iIntLiteral /= ResolveOpAsInt(1);
				else
					Dest.fFloatLiteral /= ResolveOpAsFloat(1);
				break;
			}
			case INSTR_MOD:
			{
				if (Dest.iType == OP_TYPE_INT)
					Dest.iIntLiteral %= ResolveOpAsInt(1);
				break;
			}
			case INSTR_EXP:
			{
				if (Dest.iType == OP_TYPE_INT)
					Dest.iIntLiteral = (int)pow(double(Dest.iIntLiteral), double(ResolveOpAsInt(1)));
				else
					Dest.fFloatLiteral = (float)pow((float)Dest.fFloatLiteral, (float)(ResolveOpAsFloat(1)));
				break;
			}
			case INSTR_AND:
			{
				if (Dest.iType == OP_TYPE_INT)
					Dest.iIntLiteral &= ResolveOpAsInt(1);
				break;
			}
			case INSTR_OR:
			{
				if (Dest.iType == OP_TYPE_INT)
					Dest.iIntLiteral |= ResolveOpAsInt(1);
				break;
			}

			case INSTR_XOR:
			{
				if (Dest.iType == OP_TYPE_INT)
					Dest.iIntLiteral ^= ResolveOpAsInt(1);
				break;
			}

			case INSTR_SHL:
			{
				if (Dest.iType == OP_TYPE_INT)
					Dest.iIntLiteral <<= ResolveOpAsInt(1);
				break;
			}

			case INSTR_SHR:
			{
				if (Dest.iType == OP_TYPE_INT)
					Dest.iIntLiteral >>= ResolveOpAsInt(1);
				break;
			}


			default:
				cout << "INVALID OP!!!!!!!!!11" << " " << iOpcode << endl;
				iExitExecLoop = true;
				globalExit = true;
				break;

			}

			*ResolveOpPntr(0) = Dest;


			break;
		}
		case INSTR_NEG:
		case INSTR_NOT:
		case INSTR_INC:
		case INSTR_DEC:
		{
			//get the destination type
			int iDestStoreType = GetOpType(0);

			//get a local copy of the dest
			Value Dest = ResolveOpValue(0);

			switch (iOpcode)
			{
				case INSTR_NEG:
				{
					if (Dest.iType == OP_TYPE_INT)
						Dest.iIntLiteral = -Dest.iIntLiteral;
					else
						Dest.fFloatLiteral = -Dest.fFloatLiteral;
					break;
				}
				case INSTR_NOT:
				{
					if (Dest.iType == OP_TYPE_INT)
						Dest.iIntLiteral = ~Dest.iIntLiteral;
					break;
				}
				case INSTR_INC:
				{
					if (Dest.iType == OP_TYPE_INT)
						++Dest.iIntLiteral;
					else
						++Dest.fFloatLiteral;
					break;
				}
				case INSTR_DEC:
				{
					if (Dest.iType == OP_TYPE_INT)
						--Dest.iIntLiteral;
					else
						--Dest.fFloatLiteral;
					break;
				}
				default:
					cout << "not valid!!!!" << endl;
			}

			*ResolveOpPntr(0) = Dest;
			break;
		}
		case INSTR_EXIT:
		{
			//find the exit code
			Value ExitCode = ResolveOpValue(0);
			//get it from thee integer attrib
			int iExitCode = ExitCode.iIntLiteral;

			//stop executing
			g_Scripts[g_iCurrThread].iIsRunning = FALSE;
			globalExit = true;
			break;
			
		}

			case INSTR_PUSH:
			{
				//get local copy of source op
				Value Source = ResolveOpValue(0);
				//push the value onto the stack
				Push(g_iCurrThread, Source);
				break;
			}
			case INSTR_POP:
			{
				*ResolveOpPntr(0) = Pop(g_iCurrThread);
				break;
			}

			case INSTR_CALL:
			{
				//get alocal copy of the function index
				int iFuncIndex = ResolveOpAsFuncIndex(0);

				//advance the instruction pointer so it points to the instruction in the func
				++g_Scripts[g_iCurrThread].InstrStream.iCurrInstr;

				//call function
				CallFunc(g_iCurrThread, iFuncIndex);
				break;
			}

			case INSTR_RET:
			{
				//get the current function index off the top of the stack and use it to get the func struct
				Value FuncIndex = Pop(g_iCurrThread);
				//check the presence of a stack base marker
				if (FuncIndex.iType = OP_TYPE_STACK_BASE_MARKER)
					iExitExecLoop = TRUE;
				//get the prev func index

				Func CurrFunc = GetFunc(g_iCurrThread, FuncIndex.iFuncIndex);
				int iFrameIndex = FuncIndex.iOffsetIndex;

				//read the return address structure from the stack, which is stored one index below the loacl data
				Value ReturnAddr = GetStackValue(g_iCurrThread, g_Scripts[g_iCurrThread].Stack.iTopIndex - (CurrFunc.iLocalDataSize + 1));

				//pop the stack frame along with the return address
				PopFrame(CurrFunc.iStackFrameSize);

				//restore the prev frame index
				g_Scripts[g_iCurrThread].Stack.iFrameIndex = iFrameIndex;

				//make the jump to the return address
				g_Scripts[g_iCurrThread].InstrStream.iCurrInstr = ReturnAddr.iInstrIndex;

				break;
			}

			case INSTR_JMP:
			{
				//get the index of the target instruction opcode index
				int iTargetIndex = ResolveOpAsInstrIndex(0);

				//move the instruction pointer to the target
				g_Scripts[g_iCurrThread].InstrStream.iCurrInstr = iTargetIndex;
				break;
			}

			case INSTR_JE:
			case INSTR_JNE:
			case INSTR_JG:
			case INSTR_JL:
			case INSTR_JGE:
			case INSTR_JLE:
			{	//not resolving
				//get the two operands
				Value Op0 = ResolveOpValue(0);
				Value Op1 = ResolveOpValue(1);

				//get the index of the target instruction
				int iTargetIndex = ResolveOpAsInstrIndex(2);

				//perform the specified compare and jump if true
				int iJump = false;

				if (Op0.iType != Op1.iType)
					break;

				switch (iOpcode)
				{
					//jump if equal
					case INSTR_JE:
					{
						switch (Op0.iType)
						{//OP_TYPE_STRING
							case OP_TYPE_INT:
							{
								if (Op0.iIntLiteral == Op1.iIntLiteral)
									iJump = true;
								break;
							}
							case OP_TYPE_STRING:
							{
								if (strcmp(Op0.pstrStringLiteral, Op1.pstrStringLiteral) == 0)
								{
									iJump = TRUE;
								}
								break;
							}
							default:
								cout << "INVALID OP!!!!!!!!!22" << " " << iOpcode << endl;
								iExitExecLoop = true;
								globalExit = true;
								break;
						}
						break;					
					}
					case INSTR_JNE:
					{
						switch (Op0.iType)
						{
							case OP_TYPE_INT:
								if (Op0.iIntLiteral != Op1.iIntLiteral)
									iJump = TRUE;
								break;
							case OP_TYPE_FLOAT:
								if (Op0.fFloatLiteral != Op1.fFloatLiteral)
									iJump = TRUE;
								break;
							case OP_TYPE_STRING:
								if (strcmp(Op0.pstrStringLiteral,Op1.pstrStringLiteral) != 0)
									iJump = TRUE;
								break;
						}
						break;
					}
					case INSTR_JG:
					{
						if (Op0.iType == OP_TYPE_INT)
						{
							if (Op0.iIntLiteral > Op1.iIntLiteral)
								iJump = true;
						}
						else
						{
							if (Op0.fFloatLiteral > Op1.fFloatLiteral)
								iJump = true;
						}
						break;
					}
					case INSTR_JL:
					{
						if (Op0.iType == OP_TYPE_INT)
						{
							if (Op0.iIntLiteral < Op1.iIntLiteral)
							{
								iJump = TRUE;
							}
						}
						else
						{
							if (Op0.fFloatLiteral < Op1.fFloatLiteral)
							{
								iJump = TRUE;
							}
						}
						break;
					}
					case INSTR_JGE:
					{
						if (Op0.iType == OP_TYPE_INT)
						{
							if (Op0.iIntLiteral >= Op1.iIntLiteral)
								iJump = TRUE;
						}
						else
						{
							if (Op0.fFloatLiteral >= Op1.fFloatLiteral)
								iJump = TRUE;
						}
						break;
					}

					default:
						cout << "INVALID OP!!!!!!!!!33" << " " << iOpcode << endl;
						iExitExecLoop = true;
						globalExit = true;
						break;
				}
				
				if (iJump)
				{
					g_Scripts[g_iCurrThread].InstrStream.iCurrInstr = iTargetIndex;
				}

				break;
			}
			case INSTR_CALLHOST:
			{
				//use operand zero to index the host API call table and get the host function name
				Value HostAPICall = ResolveOpValue(0);
				int iHostAPICallIndex = HostAPICall.iHostAPICallIndex;

				//get the name of the host API Function
				char* pstrFuncName = GetHostAPICall(iHostAPICallIndex);

				//search through the host API unti matched function name is found
				int iMatchFound = false;
				int iHostAPIFuncIndex = 0;
				for (iHostAPIFuncIndex = 0; iHostAPIFuncIndex < MAX_HOST_API_SIZE; ++iHostAPIFuncIndex)
				{
					//get a pointer to the name of the API func
					char* pstrCurrHostAPIFunc = g_HostAPI[iHostAPIFuncIndex].pstrName;

					//if it equals, then it's a match
					if (strcmp(pstrFuncName, pstrCurrHostAPIFunc) == 0)
					{
						//make sure the function is visible to the current thread
						int iThreadIndex = g_HostAPI[iHostAPIFuncIndex].iThreadIndex;
						if (iThreadIndex == g_iCurrThread || iThreadIndex == XS_GLOBAL_FUNC)
						{
							iMatchFound = true;
							break;
						}
					}
				}
				if (iMatchFound)
				{
					//if found match,  call the API function with the curr thread index
					g_HostAPI[iHostAPIFuncIndex].fnFunc(g_iCurrThread);
				}
				else
				{
					cout << "INVALID HOSTFunction!!!!!!!!44" << " : "<< pstrFuncName<< endl;
					iExitExecLoop = true;
					globalExit = true;
				}
				break;
			}
			default:
				cout << "INVALID OP!!!!!!!!!55" <<" : "<< iOpcode <<endl;
				iExitExecLoop=true;
				globalExit = true;
				break;
		}

		//if the instruction pointer is not atomic, increment
		if (iCurrInstr == g_Scripts[g_iCurrThread].InstrStream.iCurrInstr)
		{
			++g_Scripts[g_iCurrThread].InstrStream.iCurrInstr;
		}

		//if not run indef, check to see timesplice has ended
		if (iTimesliceDur != XS_INFINITE_TIMESLICE)
		{
			if (iCurrTime > iMainTimesliceStartTime + iTimesliceDur)
				break;
		}

		if (iExitExecLoop)
			break;
	}
}