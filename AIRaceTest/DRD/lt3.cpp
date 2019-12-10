#include <fstream>
#include <iomanip>
#include <iostream>
#include <string.h>
#include <malloc.h>
#include <stdio.h>
#include <algorithm>
#include "pin.H"
using namespace std;

typedef struct InsInfo
{
    ADDRINT addre;
    INT32 num;
    char op;
    VOID* memaddre;
    UINT32 _THREAD_id;
    INT32 line;
    string _disassemble;
    INT32 iflock;
} INS_INFO; 

//int report[1000][3];
FILE * trace;
PIN_LOCK pinLock;
int i = 0, Totalline = 0, sum=0, m = 0, lockvalue = 0;
int racecount = 0;
INS_INFO prace[1000000];
int rep = 0;


VOID RecordMemRead(ADDRINT ip, VOID * addr)
{
    int k = 0, j = 0, con = 0, *dra;
    //dra = new int[100];
    rep = 0;
    
    INT32* column = (INT32*)malloc(sizeof(INT32));
    std::string filename;
    PIN_LockClient();
    PIN_GetSourceLocation(ip, column, &prace[i].line, &filename);
    PIN_UnlockClient();
    
    if(prace[i].line)
    {
        Totalline++;

    }
    else
	return;
    dra = new int[1000];
    prace[i]._THREAD_id = PIN_ThreadId();
    prace[i].addre = ip;
    prace[i].memaddre = addr;
    prace[i].op = 'R';
    prace[i].iflock = lockvalue;
    //prace[i]._disassemble = disassemble;
    for(k = 0; k < i; k++)
    {    
	if(prace[i]._THREAD_id != prace[k]._THREAD_id)
	{
	    sum++;

	    if((prace[k].memaddre == prace[i].memaddre) && (prace[k].op == 'W') && ((prace[k].iflock == 0) || (prace[i].iflock == 0)))
	    {	        
		if((con == 0) && ((prace[i].line != prace[i - 1].line) || (prace[i - 1].memaddre != prace[i].memaddre)) && prace[k].line)
	        {
	            dra[j++] = prace[k].line;
		  //  report[m][0] = prace[k].line;
		   // report[m++][1] = prace[i].line;
		    racecount++;
		    rep++;
		}
		con++;
	    }
	    else
		con = 0;
	}
    }
    if(sum)
    {
        fprintf(trace,"num:%d  insaddress:%ld  operation:%c  memaddress:%p  ThreadId:%d  racecount:%d  sumtwain:%d  line:%d  Totalline:%d  iflock:%d", i, prace[i].addre, prace[i].op, prace[i].memaddre, prace[i]._THREAD_id, racecount, sum, prace[i].line, Totalline, prace[i].iflock);
	if(j)
	{	
	    fprintf(trace,"  racewith:");
	    for(int l = 0; l < j; l++)
	    fprintf(trace,"%d ", dra[l]);
	}		
	fprintf(trace,"\n");
    }
    else
	fprintf(trace,"num:%d  insaddress:%ld  operation:%c  memaddress:%p  ThreadId:%d  racecount:%d  sumtwain:%d  line:%d  Totalline:%d  iflock:%d\n", i, prace[i].addre, prace[i].op, prace[i].memaddre, prace[i]._THREAD_id, racecount, sum, prace[i].line, Totalline, prace[i].iflock);
    delete[] dra;
    i++;
}

VOID RecordMemWrite(ADDRINT ip, VOID * addr)
{
    int k = 0, j = 0, con = 0, *dra;
   // dra = new int[100];
    
    INT32* column = (INT32*)malloc(sizeof(INT32));
    std::string filename;
    PIN_LockClient();
    PIN_GetSourceLocation(ip, column, &prace[i].line, &filename);
    PIN_UnlockClient();
    if(prace[i].line)
        Totalline++;
    else
	return;
    dra = new int[1000];
    prace[i]._THREAD_id = PIN_ThreadId();
    prace[i].addre = ip;
    prace[i].memaddre = addr;
    prace[i].op = 'W';
    prace[i].iflock = lockvalue;
   // prace[i]._disassemble = disassemble;

    for(k = 0; k < i; k++)
    {
	if(prace[i]._THREAD_id != prace[k]._THREAD_id)    
	{
	    sum++;
	    if((prace[k].memaddre == prace[i].memaddre) && ((prace[k].iflock == 0) || (prace[i].iflock == 0)))
            {
                if((con == 0) && ((prace[i].line != prace[i - 1].line) || (prace[i - 1].memaddre != prace[i].memaddre) || (prace[i -1].op == 'R')) && prace[k].line)
	        {
	            dra[j++] = prace[k].line;
		//    report[m][0] = prace[k].line;
		 //   report[m++][1] = prace[i].line;
		    racecount++;
		}
		con++;
	    }
	    else
		con = 0;
	}
    }
    if((prace[i].line == prace[i - 1].line) && (prace[i - 1].memaddre == prace[i].memaddre) && (prace[i - 1].op == 'R'))
    {
	racecount -= rep;
	
    }
    if(sum)
    {
	fprintf(trace,"num:%d  insaddress:%ld  operation:%c  memaddress:%p  ThreadId:%d  racecount:%d  sumtwain:%d  line:%d  Totalline:%d  iflock:%d", i, prace[i].addre, prace[i].op, prace[i].memaddre, prace[i]._THREAD_id, racecount, sum, prace[i].line, Totalline, prace[i].iflock);	
	if(j)
	{	
	    fprintf(trace,"  racewith:");
	    for(int l = 0; l < j; l++)
	    fprintf(trace,"%d ", dra[l]);
	}	
	fprintf(trace,"\n");
    }
    else
	fprintf(trace,"num:%d  insaddress:%ld  operation:%c  memaddress:%p  ThreadId:%d  racecount:%d  sumtwain:%d  line:%d  Totalline:%d  iflock:%d\n", i, prace[i].addre, prace[i].op, prace[i].memaddre, prace[i]._THREAD_id, racecount, sum, prace[i].line, Totalline, prace[i].iflock);	
    delete[] dra;
    i++;
}


// Note that opening a file in a callback is only supported on Linux systems.
// See buffer-win.cpp for how to work around this issue on Windows.
//
// This routine is executed every time a thread is created.
VOID ThreadStart(THREADID threadid, CONTEXT *ctxt, INT32 flags, VOID *v)
{
    PIN_GetLock(&pinLock, threadid+1);
    fprintf(trace, "thread begin %d\n",threadid);
    fflush(trace);
    PIN_ReleaseLock(&pinLock);
}

// This routine is executed every time a thread is destroyed.
VOID ThreadFini(THREADID threadid, const CONTEXT *ctxt, INT32 code, VOID *v)
{
    PIN_GetLock(&pinLock, threadid+1);
    fprintf(trace, "thread end %d code %d\n",threadid, code);
    fflush(trace);
    PIN_ReleaseLock(&pinLock);
}

// This routine is executed each time malloc is called.
VOID BeforeMalloc( int size, THREADID threadid )
{
    PIN_GetLock(&pinLock, threadid+1);
    fprintf(trace, "thread %d entered lock(%d)\n", threadid, size);
    lockvalue = 1;
    fflush(trace);
    PIN_ReleaseLock(&pinLock);
}

VOID AfterMalloc( int size, THREADID threadid )
{
    PIN_GetLock(&pinLock, threadid+1);
    fprintf(trace, "thread %d release lock(%d)\n", threadid, size);
    lockvalue = 0;
    fflush(trace);
    PIN_ReleaseLock(&pinLock);
}


//====================================================================
// Instrumentation Routines
//====================================================================

// This routine is executed for each image.
VOID ImageLoad(IMG img, VOID *)
{
    RTN rtn1 = RTN_FindByName(img, "pthread_mutex_unlock");
    if ( RTN_Valid( rtn1 ))
	
    	{
            RTN_Open(rtn1);
        
            RTN_InsertCall(rtn1, IPOINT_BEFORE, AFUNPTR(AfterMalloc),
                       IARG_FUNCARG_ENTRYPOINT_VALUE, 0,
                       IARG_THREAD_ID, IARG_END);

            RTN_Close(rtn1);

        
    
        }
    RTN rtn = RTN_FindByName(img, "pthread_mutex_lock");
    
    if ( RTN_Valid( rtn ))
    {
        RTN_Open(rtn);
        
        RTN_InsertCall(rtn, IPOINT_BEFORE, AFUNPTR(BeforeMalloc),
                       IARG_FUNCARG_ENTRYPOINT_VALUE, 0,
                       IARG_THREAD_ID, IARG_END);

        RTN_Close(rtn);

        //lockvalue = 1;
    
    }

    else
    {
	for (SEC sec = IMG_SecHead(img); SEC_Valid(sec); sec = SEC_Next(sec))
        {
        // RTN_InsertCall() and INS_InsertCall() are executed in order of
        // appearance.  In the code sequence below, the IPOINT_AFTER is
        // executed before the IPOINT_BEFORE.
            for (RTN rtn = SEC_RtnHead(sec); RTN_Valid(rtn); rtn = RTN_Next(rtn))
            {
            // Open the RTN.
                RTN_Open( rtn );
            
            // IPOINT_AFTER is implemented by instrumenting each return
            // instruction in a routine.  Pin tries to find all return
            // instructions, but success is not guaranteed.
                
            
            // Examine each instruction in the routine.
            	for( INS ins = RTN_InsHead(rtn); INS_Valid(ins); ins = INS_Next(ins) )
                {
		  //  UINT32 memOp;  
		            
		  if( INS_IsMemoryRead(ins) ) 
			INS_InsertCall( ins, IPOINT_BEFORE, (AFUNPTR)RecordMemRead,
                                   IARG_INST_PTR, IARG_MEMORYREAD_EA, IARG_END);
            	  if( INS_IsMemoryWrite(ins) ) 
			INS_InsertCall( ins, IPOINT_BEFORE, (AFUNPTR)RecordMemWrite,
                                   IARG_INST_PTR, IARG_MEMORYWRITE_EA, IARG_END);
                }
RTN_Close( rtn );
    	    }

        }    

    }

  //  RTN rtn1 = RTN_FindByName(img, "pthread_mutex_unlock");

}

// This routine is executed once at the end.
VOID Fini(INT32 code, VOID *v)
{
    fclose(trace);
}

/* ===================================================================== */
/* Print Help Message                                                    */
/* ===================================================================== */

INT32 Usage()
{
    PIN_ERROR("This Pintool prints a trace of malloc calls in the guest application\n"
              + KNOB_BASE::StringKnobSummary() + "\n");
    return -1;
}

/* ===================================================================== */
/* Main                                                                  */
/* ===================================================================== */

int main(INT32 argc, CHAR **argv)
{
    // Initialize the pin lock
    PIN_InitLock(&pinLock);
    
    // Initialize pin
    if (PIN_Init(argc, argv)) return Usage();
    PIN_InitSymbols();
    
    //trace = fopen(KnobOutputFile.Value().c_str(), "w");
    trace = fopen("pinatrace.out", "w");
    // Register ImageLoad to be called when each image is loaded.
    IMG_AddInstrumentFunction(ImageLoad, 0);

    // Register Analysis routines to be called when a thread begins/ends
    PIN_AddThreadStartFunction(ThreadStart, 0);
    PIN_AddThreadFiniFunction(ThreadFini, 0);

    // Register Fini to be called when the application exits
    PIN_AddFiniFunction(Fini, 0);
    
    // Never returns
    PIN_StartProgram();
    
    return 0;
}
