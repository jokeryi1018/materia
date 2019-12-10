/*BEGIN_LEGAL 
Intel Open Source License 

Copyright (c) 2002-2017 Intel Corporation. All rights reserved.
 
Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:

Redistributions of source code must retain the above copyright notice,
this list of conditions and the following disclaimer.  Redistributions
in binary form must reproduce the above copyright notice, this list of
conditions and the following disclaimer in the documentation and/or
other materials provided with the distribution.  Neither the name of
the Intel Corporation nor the names of its contributors may be used to
endorse or promote products derived from this software without
specific prior written permission.
 
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE INTEL OR
ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
END_LEGAL */

#include <iostream>
#include <fstream>

#include <stdio.h>
#include "pin.H"
#include <sys/stat.h>
#include <unistd.h>
#include <iostream>
#include <string.h>
#include <malloc.h>


ofstream OutFile;

KNOB<string> KnobOutputFile( KNOB_MODE_WRITEONCE, 
			     "pintool",
      			     "o", 
			     "inscount.out", 
			     "output the CV-judgement on the given crash input"
			   );

char * inputContent = NULL;
int    inputLength  = 0;

VOID handleRetInstruction( THREADID id,
		 	   CONTRXT * ctxt,
			   ADDRINT   ip
			 ) 
{
	unsigned int getReturnAddress = 0;

	ADDRINT esp   = PIN_GetContextReg(ctxt, REG_ESP);
	int     bytes = PIN_SafeCopy( &getReturnAddress,
		       		      (ADDRINT *)esp,
			      	      sizeof(ADDRINT)	      
				    );
	if(bytes == 0)
	{
		return;
	}
	
	// now check if the desired value exists in the crashing input.
	for(int i = 0; i < sizeof(inputContent); i = i + 1)
	{
		if( ( inputContent[i] == (getReturnAddress & 0xff) ) &&
		    ( inputContent[i+1] == ((getReturnAddress & 0xff00) >> 8) ) &&
		    ( inputContent[i+2] == ((getReturnAddress & 0xff0000) >> 16) ) &&
		    ( inputContent[i+3] == ((getReturnAddress & 0xff000000) >> 24) )
		  )
		{
			// true case !
			exit(0);
		}
	}	
	
	
}
 

VOID Instruction(INS ins, VOID *v)
{
	if(INS_IsRet(ins))
	{
		ADDRINT address = INS_Address(ins);
		if(address <= (ADDRINT)0xc0000000)
		{
			INS_InsertCall( ins,
					IPOINT_BEFORE,
					(AFUNPTR)handleRetInstruction,
					IARG_THREAD_ID,
					IARG_CONTEXT,
					IARG_INST_PTR,
					IARG_PTR,
					IARG_END
			      	      );
		}
		
	}
    
}

VOID LeavePin(VOID *v)
{
    if(inputContent != NULL)
    {
	    free(inputContent);
    } 
}


int main(int argc, char * argv[])
{
	if (PIN_Init(argc, argv))
		return 0;
	
	OutFile.open(KnobOutputFile.Value().c_str());

	struct stat my_stat; 
	stat("CRASHFILE_NAME", &my_stat);
	inputLength  = my_stat.st_size;
	inputContent = (char *)malloc(sizeof(char) * inputLength);
	
	FILE * fd = fopen("CRASHFILE_NAME", "rb");
	fread(inputContent, inputContent, 1, fd);
	fclose(fd);
	
	
	// Callback function to invoke for every execution of an instruction
	INS_AddInstrumentFunction(Instruction, 0);
	
	// register a function will be called when the application exits
	PIN_AddFiniFunction(Fini, 0);	

	// Never returns
	PIN_StartProgram();
	
	return 0;
}
