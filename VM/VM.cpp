//////////////////////////////////////////////////////////////////////
//2024 Copyright David Martin. All Rights Reserved. 
//
/////////////////////////////////////////////////////////////////////
#include "functions.h"




/******************************************************************************************
	*
	*   HAPI_PrintString ()
	*
	*   This is a simple host API function that scripts can call to print strings a specified
	*   number of times, as well as receive an arbitrary return value.
	*/

void HAPI_PrintString(int iThreadIndex)
{
	// Read in the parameters
	
	//***************^%^&*&^%$%^&*&^%^&*&^%^&^%^&
	char * pstrString = XS_GetParamAsString(iThreadIndex, 0);
	//int iCount = XS_GetParamAsInt(iThreadIndex, 1);
	int iCount = 1;

	//printf("******************\n");
	//printf("HAPI_PrintString\n");
	//printf("%s\n", pstrString);
	//printf("%d\n", iCount);
	//printf("******************\n");

	//if (iCount == 1)
	//{
	//	printf("one");
	//}
	//else
	//	printf("zero");

	// Print the specified string the specified number of times (print everything with a
	// leading tab to separate it from the text printed by the host)

	//for (int iCurrString = 0; iCurrString < iCount; ++iCurrString)
		printf("\t%s\n", pstrString);

	// Return a value

	XS_ReturnString(iThreadIndex, 1, "This is a return value.");
}

void HAPI_Scanf(int iThreadIndex)
{
	// Read in the parameters

	//***************^%^&*&^%$%^&*&^%^&*&^%^&^%^&
	char * pstrString = XS_GetParamAsString(iThreadIndex, 0);
	//int iCount = XS_GetParamAsInt(iThreadIndex, 1);
	int iCount = 1;

	//printf("******************\n");
	//printf("HAPI_PrintString\n");
	//printf("%s\n", pstrString);
	//printf("%d\n", iCount);
	//printf("******************\n");

	//if (iCount == 1)
	//{
	//	printf("one");
	//}
	//else
	//	printf("zero");

	// Print the specified string the specified number of times (print everything with a
	// leading tab to separate it from the text printed by the host)

	//for (int iCurrString = 0; iCurrString < iCount; ++iCurrString)
	printf("\t%s\n", pstrString);

	// Return a value

	XS_ReturnString(iThreadIndex, 2, "This is a return value.");
}




int main()
{

	
	// Print the logo

	printf("XVM Final\n");
	printf("XtremeScript Virtual Machine\n");
	printf("Written by Alex Varanese\n");
	printf("\n");

	XS_Init();

	// Declare the thread indices

	int iThreadIndex;

	// An error code

	int iErrorCode;

	// Load the demo script
	//10 failed

	iErrorCode = XS_LoadScript("DEMO10.VSE", iThreadIndex, XS_THREAD_PRIORITY_USER);

	// Check for an error

	if (iErrorCode != XS_LOAD_OK)
	{
		// Print the error based on the code

		printf("Error: ");

		switch (iErrorCode)
		{
		case XS_LOAD_ERROR_FILE_IO:
			printf("File I/O error");
			break;

		case XS_LOAD_ERROR_INVALID_XSE:
			printf("Invalid .XSE file");
			break;

		case XS_LOAD_ERROR_UNSUPPORTED_VERS:
			printf("Unsupported .XSE version");
			break;

		case XS_LOAD_ERROR_OUT_OF_MEMORY:
			printf("Out of memory");
			break;

		case XS_LOAD_ERROR_OUT_OF_THREADS:
			printf("Out of threads");
			break;
		}

		printf(".\n");
		return 0;
	}
	else
	{
		// Print a success message

		printf("Script loaded successfully.\n");
	}
	printf("\n");

	// Register the string printing function

	XS_RegisterHostAPIFunc(XS_GLOBAL_FUNC, "PrintString", HAPI_PrintString);

	// Start up the script

	XS_StartScript(iThreadIndex);

	printf("Calling DoStuff () asynchronously:\n");
	printf("\n");
	//int counter = 0;
	//printf("%d", counter);
	//printf("\n");
	//while (!kbhit())
	while (!kbhit() && !globalExit)
	{
		XS_RunScripts(50);
		//printf("%d", counter);
		//printf("\n");
		//counter++;
	}

	//printf("%d", counter);
		

	//XS_ShutDown();
}