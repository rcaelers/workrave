/*
 * harpoon.c
 *
 * Copyright (C) 2002-2003 Raymond Penners <raymond@dotsphinx.com>
 * All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * $Id$
 */

static const char rcsid[] = "$Id$";

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "crashlog.h"

static void unwind_stack(FILE *log, HANDLE process, PCONTEXT context);

static 
DWORD GetModuleBase(DWORD dwAddress)
{
  MEMORY_BASIC_INFORMATION Buffer;
	
  return VirtualQuery((LPCVOID) dwAddress, &Buffer, sizeof(Buffer)) ? (DWORD) Buffer.AllocationBase : 0;
}

EXCEPTION_DISPOSITION __cdecl 
exception_handler(struct _EXCEPTION_RECORD *exception_record, 
                  void *establisher_frame, 
                  struct _CONTEXT *context_record, 
                  void *dispatcher_context)
{
  char crash_log_name[MAX_PATH];
  char crash_text[1024];
  TCHAR szModule[MAX_PATH];
  HMODULE hModule;

  GetModuleFileName(GetModuleHandle(NULL), crash_log_name, sizeof(crash_log_name));
  // crash_log_name == c:\program files\workrave\lib\workrave.exe
  char *s = strrchr(crash_log_name, '\\');
  assert (s);
  *s = '\0';
  // crash_log_name == c:\program files\workrave\lib
  s = strrchr(crash_log_name, '\\');
  assert (s);
  *s = '\0';
  // crash_log_name == c:\program files\workrave
  strcat(crash_log_name, "\\workrave.log");
  
  FILE *log = fopen(crash_log_name, "w");
  if (log != NULL)
    {

      fprintf(log, "Exception Report\n");
      fprintf(log, "----------------\n\n");
      
      fprintf(log, "code = %x\n", exception_record->ExceptionCode);
      fprintf(log, "flags = %x\n", exception_record->ExceptionFlags);
      fprintf(log, "address = %x\n", exception_record->ExceptionAddress);
      fprintf(log, "params = %d\n", exception_record->NumberParameters);

      fprintf(log, "%s caused ",  GetModuleFileName(NULL, szModule, MAX_PATH) ? szModule : "Application");
      switch (exception_record->ExceptionCode)
        {
        case EXCEPTION_ACCESS_VIOLATION:
          fprintf(log, "an Access Violation");
          break;
	
        case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:
          fprintf(log, "an Array Bound Exceeded");
          break;
	
        case EXCEPTION_BREAKPOINT:
          fprintf(log, "a Breakpoint");
          break;
	
        case EXCEPTION_DATATYPE_MISALIGNMENT:
          fprintf(log, "a Datatype Misalignment");
          break;
	
        case EXCEPTION_FLT_DENORMAL_OPERAND:
          fprintf(log, "a Float Denormal Operand");
          break;
	
        case EXCEPTION_FLT_DIVIDE_BY_ZERO:
          fprintf(log, "a Float Divide By Zero");
          break;
	
        case EXCEPTION_FLT_INEXACT_RESULT:
          fprintf(log, "a Float Inexact Result");
          break;
	
        case EXCEPTION_FLT_INVALID_OPERATION:
          fprintf(log, "a Float Invalid Operation");
          break;
	
        case EXCEPTION_FLT_OVERFLOW:
          fprintf(log, "a Float Overflow");
          break;
	
        case EXCEPTION_FLT_STACK_CHECK:
          fprintf(log, "a Float Stack Check");
          break;
	
        case EXCEPTION_FLT_UNDERFLOW:
          fprintf(log, "a Float Underflow");
          break;
	
        case EXCEPTION_GUARD_PAGE:
          fprintf(log, "a Guard Page");
          break;

        case EXCEPTION_ILLEGAL_INSTRUCTION:
          fprintf(log, "an Illegal Instruction");
          break;
	
        case EXCEPTION_IN_PAGE_ERROR:
          fprintf(log, "an In Page Error");
          break;
	
        case EXCEPTION_INT_DIVIDE_BY_ZERO:
          fprintf(log, "an Integer Divide By Zero");
          break;
	
        case EXCEPTION_INT_OVERFLOW:
          fprintf(log, "an Integer Overflow");
          break;
	
        case EXCEPTION_INVALID_DISPOSITION:
          fprintf(log, "an Invalid Disposition");
          break;
	
        case EXCEPTION_INVALID_HANDLE:
          fprintf(log, "an Invalid Handle");
          break;

        case EXCEPTION_NONCONTINUABLE_EXCEPTION:
          fprintf(log, "a Noncontinuable Exception");
          break;

        case EXCEPTION_PRIV_INSTRUCTION:
          fprintf(log, "a Privileged Instruction");
          break;
	
        case EXCEPTION_SINGLE_STEP:
          fprintf(log, "a Single Step");
          break;
	
        case EXCEPTION_STACK_OVERFLOW:
          fprintf(log, "a Stack Overflow");
          break;

        case DBG_CONTROL_C:
          fprintf(log, "a Control+C");
          break;
	
        case DBG_CONTROL_BREAK:
          fprintf(log, "a Control+Break");
          break;
	
        case DBG_TERMINATE_THREAD:
          fprintf(log, "a Terminate Thread");
          break;
	
        case DBG_TERMINATE_PROCESS:
          fprintf(log, "a Terminate Process");
          break;
	
        case RPC_S_UNKNOWN_IF:
          fprintf(log, "an Unknown Interface");
          break;
	
        case RPC_S_SERVER_UNAVAILABLE:
          fprintf(log, "a Server Unavailable");
          break;
	
        default:
          fprintf(log, "an Unknown [0x%lX] Exception", exception_record->ExceptionCode);
          break;
        }

      fprintf(log, " at location %08x", (DWORD) exception_record->ExceptionAddress);
      if ((hModule = (HMODULE) GetModuleBase((DWORD) exception_record->ExceptionAddress) && GetModuleFileName(hModule, szModule, sizeof(szModule))))
        fprintf(log, " in module %s", szModule);
	
      // If the exception was an access violation, print out some additional information, to the error log and the debugger.
      if(exception_record->ExceptionCode == EXCEPTION_ACCESS_VIOLATION && exception_record->NumberParameters >= 2)
        fprintf(log, " %s location %08x\n\n", exception_record->ExceptionInformation[0] ? "writing to" : "reading from", exception_record->ExceptionInformation[1]);
  
      DWORD pid = GetCurrentProcessId();
      HANDLE process = OpenProcess(PROCESS_ALL_ACCESS, TRUE, pid);

      unwind_stack(log, process, context_record);
      
      fclose(log);
    }

  snprintf(crash_text, 1023,
           "Workrave unexpectedly crashed. A crash log has been saved to "
           "%s. Please mail this file to to workrave-devel@sourceforge.net or "
           "file a bugreport at our bugzilla: "
           "http://workrave.org/cgi-bin/bugzilla/enter_bug.cgi. "
           "Thanks.", crash_log_name);
  
  MessageBox(NULL, crash_text, "Exception", MB_OK);
  exit(1);
}


static
BOOL WINAPI stack_walk(HANDLE process, LPSTACKFRAME stack_frame, PCONTEXT context_record)
{
  if (!stack_frame->Reserved[0])
    {
      stack_frame->Reserved[0] = 1;
		
      stack_frame->AddrPC.Mode = AddrModeFlat;
      stack_frame->AddrPC.Offset = context_record->Eip;
      stack_frame->AddrStack.Mode = AddrModeFlat;
      stack_frame->AddrStack.Offset = context_record->Esp;
      stack_frame->AddrFrame.Mode = AddrModeFlat;
      stack_frame->AddrFrame.Offset = context_record->Ebp;
      
      stack_frame->AddrReturn.Mode = AddrModeFlat;
      if (!ReadProcessMemory(process,
                             (LPCVOID) (stack_frame->AddrFrame.Offset + sizeof(DWORD)),
                             &stack_frame->AddrReturn.Offset, sizeof(DWORD), NULL))
        return FALSE;
    }
  else
    {
      stack_frame->AddrPC.Offset = stack_frame->AddrReturn.Offset;

      if (!ReadProcessMemory(process, (LPCVOID) stack_frame->AddrFrame.Offset,
                            &stack_frame->AddrFrame.Offset, sizeof(DWORD), NULL))
        return FALSE;
                
      if (!ReadProcessMemory(process, (LPCVOID) (stack_frame->AddrFrame.Offset + sizeof(DWORD)),
                             &stack_frame->AddrReturn.Offset, sizeof(DWORD), NULL))
        return FALSE;
    }

  ReadProcessMemory(process, (LPCVOID) (stack_frame->AddrFrame.Offset + 2*sizeof(DWORD)),
                    stack_frame->Params, sizeof(stack_frame->Params), NULL);
	
  return TRUE;	
}

static void
unwind_stack(FILE *log, HANDLE process, PCONTEXT context)
{
  STACKFRAME          sf;

  fprintf(log, "Stack trace:\n");

  ZeroMemory(&sf,  sizeof(STACKFRAME));
  sf.AddrPC.Offset    = context->Eip;
  sf.AddrPC.Mode      = AddrModeFlat;
  sf.AddrStack.Offset = context->Esp;
  sf.AddrStack.Mode   = AddrModeFlat;
  sf.AddrFrame.Offset = context->Ebp;
  sf.AddrFrame.Mode   = AddrModeFlat;

  fprintf(log, "PC        Frame     Ret\n");
  
  while (TRUE)
    {
      if (!stack_walk(process, &sf, context))
        break;
     
      if (sf.AddrFrame.Offset == 0)
        break;
     
      fprintf(log, "%08X  %08X  %08X\n",  
              sf.AddrPC.Offset,
              sf.AddrFrame.Offset,
              sf.AddrReturn.Offset);
    }
}
