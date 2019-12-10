
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

VOID handleCallInstruction( THREADID  id,
			    CONTEXT * ctxt,
			    ADDRINT   ip
			  )
{
	uint8_t dataBuf[4];

	ADDRINT eip   = PIN_GetContextReg(ctxt, REG_EIP);

	// heuristic : just check if 5 bytes derive from the 'crash' !
	// we make it 5 as 
	int     bytes = PIN_SafeCopy( dataBuf,
		       		     (ADDRINT *)eip,
			     	     sizeof(ADDRINT)	     
				    );
	if (bytes == 0)
	{
		return;
	}

	// now check if the desired value exists in the crashing input.
	for(unsigned int i = 0; i < sizeof(inputContent); i = i + 1)
	{
		if( ( (uint8_t)(inputContent[i]) == dataBuf[0]) &&
		    ( (uint8_t)(inputContent[i+1]) == dataBuf[1]) &&
		    ( (uint8_t)(inputContent[i+2]) == dataBuf[2]) &&
		    ( (uint8_t)(inputContent[i+3]) == dataBuf[3])
		  )
		{
			/* 2: call shellcode ERROR !
			 *
			 * */
			OutFile << "2";
			OutFile.close();
			exit(0);
		}
	}	
}


VOID handleRetInstruction( THREADID  id,
		 	   CONTEXT * ctxt,
			   ADDRINT   ip
			 ) 
{
	unsigned int getReturnAddress = 0;

	ADDRINT esp   = PIN_GetContextReg(ctxt, REG_ESP);
	int     bytes = PIN_SafeCopy( &getReturnAddress,
		       		      (ADDRINT *)esp,
			      	      sizeof(ADDRINT)	      
				    );
	if (bytes == 0)
	{
		return;
	}
	
	// now check if the desired value exists in the crashing input.
	for(unsigned int i = 0; i < sizeof(inputContent); i = i + 1)
	{
		if( ( (uint8_t)(inputContent[i]) == (uint8_t)(getReturnAddress & 0xff) ) &&
		    ( (uint8_t)(inputContent[i+1]) == (uint8_t)((getReturnAddress & 0xff00) >> 8) ) &&
		    ( (uint8_t)(inputContent[i+2]) == (uint8_t)((getReturnAddress & 0xff0000) >> 16) ) &&
		    ( (uint8_t)(inputContent[i+3]) == (uint8_t)((getReturnAddress & 0xff000000) >> 24) )
		  )
		{
			/* 1: EIP Overwritten ERROR !
			 *
			 * */
			OutFile << "1";
			OutFile.close();
			exit(0);
		}
	}	
	
	
}
 

VOID Instruction(INS ins, VOID *v)
{
	if (INS_IsRet(ins))
	{
		ADDRINT address = INS_Address(ins);
		if (address <= (ADDRINT)0xc0000000)
		{
			INS_InsertCall( ins,
					IPOINT_BEFORE,
					(AFUNPTR)handleRetInstruction,
					IARG_THREAD_ID,
					IARG_CONTEXT,
					IARG_INST_PTR,
					//IARG_PTR,
					IARG_END
			      	      );
		}
		
	}    
	
	if (INS_IsCall(ins))
	{
		ADDRINT address = INS_Address(ins);
		if (address <= (ADDRINT)0xc0000000)
		{
			INS_InsertCall( ins,
					IPOINT_BEFORE,
					(AFUNPTR)handleCallInstruction,
					IARG_THREAD_ID,
					IARG_CONTEXT,
					IARG_INST_PTR,
					IARG_END
				      );
		}
	}
}


VOID LeavePin(INT32 code, VOID * v)
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

	//struct stat my_stat; 
	//stat("/home/hhui/Experiments/RHG/ExpGen/crashes/1/payload01", &my_stat);
	inputLength  = 1136; //my_stat.st_size;
	inputContent = (char *)malloc(sizeof(char) * inputLength);
	
	FILE * fd = fopen("/home/hhui/Experiments/RHG/ExpGen/crashes/1/payload01", "rb");
	fread(inputContent, inputLength, 1, fd);
	fclose(fd);
	
	
	// Callback function to invoke for every execution of an instruction
	INS_AddInstrumentFunction(Instruction, 0);
	
	// register a function will be called when the application exits
	PIN_AddFiniFunction(LeavePin, 0);	

	// Never returns
	PIN_StartProgram();
	
	return 0;
}
