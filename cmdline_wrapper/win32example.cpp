// win32Example.cpp
//
// Based on "Creating a Child Process with Redirected Input and Output"[1] from
// Microsoft Docs, but heavily modified and simplified for this specific use case.
//
// [1] https://docs.microsoft.com/en-us/windows/desktop/procthread/creating-a-child-process-with-redirected-input-and-output
//
#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <strsafe.h>
#include <math.h>

#define BUFSIZE 4096

/* Execute usbtenkiget and return values, converted to float. Nan is used
 * for signaling illegal values (some sensors may return "err" or Nan when
 * a low level error occurs.
 *
 * \param cmdline Pointer to a string for the command to execute. Eg: TEXT("usbtenkiget -i 1")
 * \param values Pointer to float*. A properly sized array of floats will be allocated with malloc. Must be freed using free() by the caller.
 * \param n_values Pointer to an int where the number of received fields will be stored.
 * \return false on error, true on success. values does not need to be freed if an error was returned.
 */
BOOL getUsbTenkiValues(TCHAR cmdline[], float **values, int *n_values)
{
	PROCESS_INFORMATION piProcInfo;
	STARTUPINFO siStartInfo;
	BOOL bSuccess = FALSE;
	HANDLE g_hChildStd_OUT_Rd = NULL;
	HANDLE g_hChildStd_OUT_Wr = NULL;
	SECURITY_ATTRIBUTES saAttr;
	saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
	saAttr.bInheritHandle = TRUE;
	saAttr.lpSecurityDescriptor = NULL;
	CHAR chBuf[BUFSIZE];
	TCHAR *cmdline_copy = NULL;

	if (!values || !n_values) {
		return false;
	}

	// Create a pipe for the child process's STDOUT.
	if (!CreatePipe(&g_hChildStd_OUT_Rd, &g_hChildStd_OUT_Wr, &saAttr, 0)) {
		fprintf(stderr, "Could not create pipe\n");
		return false;
	}

	// Set up members of the PROCESS_INFORMATION structure.
	ZeroMemory(&piProcInfo, sizeof(PROCESS_INFORMATION));
	ZeroMemory(&siStartInfo, sizeof(STARTUPINFO));
	siStartInfo.cb = sizeof(STARTUPINFO);
	siStartInfo.hStdOutput = g_hChildStd_OUT_Wr;
	siStartInfo.dwFlags |= STARTF_USESTDHANDLES;

	/* Create a copy of the command-line passed in argument. CreateProcessW
	 * may modify the string, so we must be sure the string passed to CreateProcess
	 * is NOT in read-only memory (such as a const variable or a literal string) */
	cmdline_copy = _tcsdup(cmdline);
	if (!cmdline_copy) {
		fprintf(stderr, "Could not allocate memory");
		CloseHandle(g_hChildStd_OUT_Wr);
		CloseHandle(g_hChildStd_OUT_Rd);
		return false;
	}

	// Create the child process.
	bSuccess = CreateProcess(NULL,
		cmdline_copy,  // command line
		NULL,          // process security attributes
		NULL,          // primary thread security attributes
		TRUE,          // handles are inherited
		0,             // creation flags
		NULL,          // use parent's environment
		NULL,          // use parent's current directory
		&siStartInfo,  // STARTUPINFO pointer
		&piProcInfo);  // receives PROCESS_INFORMATION

	// Free the command-line copy as it won't be needed anymore
	free(cmdline_copy);

	// Close the writing end of the pipe now that the child has inherited it. Otherwise
	// the read loop below will never stop.
	CloseHandle(g_hChildStd_OUT_Wr);

	if (!bSuccess) {
		CloseHandle(g_hChildStd_OUT_Rd);
		fprintf(stderr, "Could not run usbtenkiget\n");
		return false;
	}

	// Wait until usbtenkiget exits
	WaitForSingleObject(piProcInfo.hProcess, INFINITE);

	// Close handles to the child process
	CloseHandle(piProcInfo.hProcess);
	CloseHandle(piProcInfo.hThread);

	// Now read the output that was buffered in the pipe. Read at most
	// BUFSIZE-1 to be sure we always get a NUL-terminated string.
	ZeroMemory(chBuf, sizeof(chBuf));
	bSuccess = ReadFile(g_hChildStd_OUT_Rd, chBuf, BUFSIZE-1, NULL, NULL);

	// Close the read end of the pipe too, now that we are done.
	CloseHandle(g_hChildStd_OUT_Rd);

	if (!bSuccess) {
		fprintf(stderr, "Could not read from usbtenkiget\n");
		return false;
	}

	/* Now we have comma-separated values in chBuf. Count how many fields. */
	int fields = 1;
	for (char *c = chBuf; *c; c++) {
		if (*c == ',')
			fields++;
	}

	/* Allocate memory for the array of floats, and convert fields to floats. */
	*n_values = fields;
	*values = (float*)calloc(fields, sizeof(float));
	if (!*values) {
		fprintf(stderr, "Could not allocate memory for array of values\n");
		return false;
	}
	float *dst_value = *values;
	char *c = chBuf;
	for (char *c = chBuf; c; )
	{
		if (1 != sscanf_s(c, "%f", dst_value)) {
			*dst_value = nanf("");
		}
		dst_value++;
		c = strchr(c, ',');
		// skip , for next value, otherwise c is NULL and loop stops
		if (c)
			c++;
	}

	return true;
}

int main()
{
	BOOL bSuccess;
	float *values = NULL;
	int n_values;

	bSuccess = getUsbTenkiValues(TEXT("usbtenkiget -i 0,1,2\0"), &values, &n_values);
	if (!bSuccess) {
		return -1;
	}

	if (bSuccess) {
		// We run usbtenkiget with the -i 0,1,2 argument which requests
		// exactly 3 channels. Getting any other quantity is therefore an error.
		if (n_values != 3) {
			fprintf(stderr, "usbtenkiget returned wrong number of fields\n");
			return -1;
		}
		else {
			printf("Temperature (C): %.2f\n", values[0]);
			printf("RH......... (%%): %.2f\n", values[1]);
			printf("Pressure. (kPa): %.2f\n", values[2]);
			printf("Temperature (F): %.2f\n", values[0] * 9 / 5 + 32);
		}
	}

	if (values) {
		free(values);
	}

	return 0;
}

