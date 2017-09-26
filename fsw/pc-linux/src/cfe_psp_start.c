/******************************************************************************
** File:  cfe_psp_start.c
**
**
**      Copyright (c) 2004-2011, United States Government as represented by 
**      Administrator for The National Aeronautics and Space Administration. 
**      All Rights Reserved.
**
**      This is governed by the NASA Open Source Agreement and may be used,
**      distributed and modified only pursuant to the terms of that agreement. 
**
**

** Purpose:
**   cFE BSP main entry point.
**
** History:
**   2005/07/26  A. Cudmore      | Initial version for OS X/Linux 
**
******************************************************************************/

/*
**  Include Files
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <sys/time.h>
#include <getopt.h>
#include <string.h>
#include <limits.h>
#include <pthread.h>
#include <sched.h>
#include <errno.h>

/*
** cFE includes 
*/
#include "common_types.h"
#include "osapi.h"

#include "cfe_psp.h"

#ifdef _ENHANCED_BUILD_

/*
 * The preferred way to obtain the CFE tunable values at runtime is via
 * the dynamically generated configuration object.  This allows a single build
 * of the PSP to be completely CFE-independent.
 */
#include <target_config.h>
#include "cfe_psp_module.h"

#define CFE_ES_MAIN_FUNCTION        (GLOBAL_CONFIGDATA.CfeConfig->SystemMain)
#define CFE_TIME_1HZ_FUNCTION       (GLOBAL_CONFIGDATA.CfeConfig->System1HzISR)
#define CFE_PLATFORM_ES_NONVOL_STARTUP_FILE  (GLOBAL_CONFIGDATA.CfeConfig->NonvolStartupFile)
#define CFE_PLATFORM_CPU_ID                  (GLOBAL_CONFIGDATA.Default_CpuId)
#define CFE_PLATFORM_CPU_NAME                (GLOBAL_CONFIGDATA.Default_CpuName)
#define CFE_MISSION_SPACECRAFT_ID            (GLOBAL_CONFIGDATA.Default_SpacecraftId)

#else

/*
 * cfe_platform_cfg.h needed for CFE_PLATFORM_ES_NONVOL_STARTUP_FILE, CFE_PLATFORM_CPU_ID/CPU_NAME/SPACECRAFT_ID
 *
 *  - this should NOT be included here -
 *
 * it is only for compatibility with the old makefiles.  Including this makes the PSP build
 * ONLY compatible with a CFE build using this exact same CFE platform config.
 */

#include "cfe_platform_cfg.h"

extern void CFE_ES_Main(uint32 StartType, uint32 StartSubtype, uint32 ModeId, const char *StartFilePath );
extern void CFE_TIME_Local1HzISR(void);

#define CFE_ES_MAIN_FUNCTION     CFE_ES_Main
#define CFE_TIME_1HZ_FUNCTION    CFE_TIME_Local1HzISR

/*
 * The classic build does not support static modules,
 * so stub the ModuleInit() function out right here
 */
void CFE_PSP_ModuleInit(void)
{
}

#endif

/*
** Defines
*/

#define CFE_PSP_CPU_NAME_LENGTH  32
#define CFE_PSP_RESET_NAME_LENGTH 10

/*
** Typedefs for this module
*/

/*
** Structure for the Command line parameters
*/
typedef struct
{   
   char     ResetType[CFE_PSP_RESET_NAME_LENGTH];   /* Reset type can be "PO" for Power on or "PR" for Processor Reset */
   uint32   GotResetType;    /* did we get the ResetType parameter ? */

   uint32   SubType;         /* Reset Sub Type ( 1 - 5 )  */
   uint32   GotSubType;      /* did we get the ResetSubType parameter ? */
   
   char     CpuName[CFE_PSP_CPU_NAME_LENGTH];     /* CPU Name */
   uint32   GotCpuName;      /* Did we get a CPU Name ? */

   uint32   CpuId;            /* CPU ID */
   uint32   GotCpuId;         /* Did we get a CPU Id ?*/

   uint32   SpacecraftId;     /* Spacecraft ID */ 
   uint32   GotSpacecraftId;  /* Did we get a Spacecraft ID */
   
} CFE_PSP_CommandData_t;

/*
** Prototypes for this module
*/
void CFE_PSP_SigintHandler (int signal);
void CFE_PSP_TimerHandler (int signum);
void CFE_PSP_DisplayUsage(char *Name );
void CFE_PSP_ProcessArgumentDefaults(CFE_PSP_CommandData_t *CommandData);
void CFE_PSP_SetupSystemTimer(void);

/*
**  External Declarations
*/
extern void CFE_PSP_DeleteProcessorReservedMemory(void);

/*
** Global variables
*/
uint32              TimerCounter;
CFE_PSP_CommandData_t CommandData;
uint32              CFE_PSP_SpacecraftId;
uint32              CFE_PSP_CpuId;
char                CFE_PSP_CpuName[CFE_PSP_CPU_NAME_LENGTH];

/*
** getopts parameter passing options string
*/
static const char *optString = "R:S:C:I:N:h";

/*
** getopts_long long form argument table
*/
static const struct option longOpts[] = {
   { "reset",     required_argument, NULL, 'R' },
   { "subtype",   required_argument, NULL, 'S' },
   { "cpuid",     required_argument, NULL, 'C' },
   { "scid",      required_argument, NULL, 'I'},
   { "cpuname",   required_argument, NULL, 'N'},
   { "help",      no_argument,       NULL, 'h' },
   { NULL,        no_argument,       NULL,  0 }
};

                                                                                                                                                            
/******************************************************************************
**  Function:  main()
**
**  Purpose:
**    BSP Application entry point.
**
**  Arguments:
**    (none)
**
**  Return:
**    (none)
*/
int main(int argc, char *argv[])
{
   uint32             reset_type;
   uint32             reset_subtype;
   int                opt = 0;
   int                longIndex = 0;

   /*
   ** Initialize the CommandData struct 
   */
   memset(&(CommandData), 0, sizeof(CFE_PSP_CommandData_t));
      
   /* 
   ** Process the arguments with getopt_long(), then 
   ** start the cFE
   */
   opt = getopt_long( argc, argv, optString, longOpts, &longIndex );
   while( opt != -1 ) 
   {
      switch( opt ) 
      {
         case 'R':
            strncpy(CommandData.ResetType, optarg, CFE_PSP_RESET_NAME_LENGTH);
            if ((strncmp(CommandData.ResetType, "PO", CFE_PSP_RESET_NAME_LENGTH ) != 0 ) &&
                (strncmp(CommandData.ResetType, "PR", CFE_PSP_RESET_NAME_LENGTH ) != 0 ))
            {
               printf("\nERROR: Invalid Reset Type: %s\n\n",CommandData.ResetType);
               CommandData.GotResetType = 0;
               CFE_PSP_DisplayUsage(argv[0]);
               break;
            }
            printf("CFE_PSP: Reset Type: %s\n",(char *)optarg);
            CommandData.GotResetType = 1;
            break;
				
         case 'S':
            CommandData.SubType = strtol(optarg, NULL, 0 );
            if ( CommandData.SubType < 1 || CommandData.SubType > 5 )
            {
               printf("\nERROR: Invalid Reset SubType: %s\n\n",optarg);
               CommandData.SubType = 0;
               CommandData.GotSubType = 0;
               CFE_PSP_DisplayUsage(argv[0]);
               break;
            }
            printf("CFE_PSP: Reset SubType: %d\n",(int)CommandData.SubType);
            CommandData.GotSubType = 1;
            break;

         case 'N':
            strncpy(CommandData.CpuName, optarg, CFE_PSP_CPU_NAME_LENGTH );
            printf("CFE_PSP: CPU Name: %s\n",CommandData.CpuName);
            CommandData.GotCpuName = 1;
            break;

         case 'C':
            CommandData.CpuId = strtol(optarg, NULL, 0 );
            printf("CFE_PSP: CPU ID: %d\n",(int)CommandData.CpuId);
            CommandData.GotCpuId = 1;
            break;

         case 'I':
            CommandData.SpacecraftId = strtol(optarg, NULL, 0 );
            printf("CFE_PSP: Spacecraft ID: %d\n",(int)CommandData.SpacecraftId);
            CommandData.GotSpacecraftId = 1;
            break;

         case 'h':
            CFE_PSP_DisplayUsage(argv[0]);
            break;
	
         default:
            CFE_PSP_DisplayUsage(argv[0]);
            break;
       }
		
       opt = getopt_long( argc, argv, optString, longOpts, &longIndex );
   } /* end while */
   
   /*
   ** Set the defaults for values that were not given for the 
   ** optional arguments, and check for arguments that are required.
   */
   CFE_PSP_ProcessArgumentDefaults(&CommandData);

   /*
   ** Set the reset type
   */
   if (strncmp("PR", CommandData.ResetType, 2 ) == 0 )
   {
     reset_type = CFE_PSP_RST_TYPE_PROCESSOR;
      OS_printf("CFE_PSP: Starting the cFE with a PROCESSOR reset.\n");
   }
   else
   {
      reset_type = CFE_PSP_RST_TYPE_POWERON;
      OS_printf("CFE_PSP: Starting the cFE with a POWER ON reset.\n");
   }

   /*
   ** Assign the Spacecraft ID, CPU ID, and CPU Name
   */
   CFE_PSP_SpacecraftId = CommandData.SpacecraftId;
   CFE_PSP_CpuId = CommandData.CpuId;
   strncpy(CFE_PSP_CpuName, CommandData.CpuName, CFE_PSP_CPU_NAME_LENGTH);

   /*
   ** Set the reset subtype
   */
   reset_subtype = CommandData.SubType;

   /*
    * If _ENHANCED_BUILD_ is set, confirm that the GLOBAL_CONFIGDATA
    * structure pointers are not null.  These are used later, and it
    * simplifies conditional logic.
    *
    * If these pointers are null then it indicates a build system failure
    * or link issue.  (These are constant structures populated at compile time).
    */
#ifdef _ENHANCED_BUILD_
   if (GLOBAL_CONFIGDATA.CfeConfig == NULL ||
           GLOBAL_CONFIGDATA.PspConfig == NULL)
   {
       OS_printf("CFE_PSP: Panic due to NULL configuration pointer\n");
       CFE_PSP_Panic(CFE_PSP_INVALID_POINTER);
   }
#endif

   /*
   ** Install sigint_handler as the signal handler for SIGINT.
   */
   signal(SIGINT, CFE_PSP_SigintHandler);

   /*
   ** Initialize the OS API data structures
   */
   OS_API_Init();

   /*
   ** Start the timer
   ** Done here so that the modules can also use it, if desired
   */
   CFE_PSP_SetupSystemTimer();

   /*
   ** Initialize the statically linked modules (if any)
   ** This is only applicable to CMake build - classic build
   ** does not have the logic to selectively include/exclude modules
   */
   CFE_PSP_ModuleInit();
     
   sleep(1);

   /*
   ** Initialize the reserved memory 
   */
   CFE_PSP_InitProcessorReservedMemory(reset_type);


   /*
   ** Call cFE entry point.
   */
   CFE_ES_MAIN_FUNCTION(reset_type, reset_subtype, 1, CFE_PLATFORM_ES_NONVOL_STARTUP_FILE);

   /*
   ** Let the main thread sleep.
   **
   ** OS_IdleLoop() will wait forever and return if
   ** someone calls OS_ApplicationShutdown(TRUE)
   */
   OS_IdleLoop();

   /*
    * The only way OS_IdleLoop() will return is if SIGINT is captured
    * Handle cleanup duties.
    */
   OS_printf("\nCFE_PSP: Control-C Captured - Exiting cFE\n");

   /* Deleting these memories will unlink them, but active references should still work */
   CFE_PSP_DeleteProcessorReservedMemory();

   OS_printf("CFE_PSP: NOTE: After quitting the cFE with a Control-C signal, it MUST be started next time\n");
   OS_printf("     with a Poweron Reset ( --reset PO ). \n");

   OS_DeleteAllObjects();


   return(0);
}

/******************************************************************************
**  Function:  CFE_PSP_SigintHandler()
**
**  Purpose:
**    SIGINT routine for linux/OSX
**
**  Arguments:
**    (none)
**
**  Return:
**    (none)
*/

void CFE_PSP_SigintHandler (int signal)
{
    OS_ApplicationShutdown(TRUE);
}

/******************************************************************************
**  Function:  CFE_PSP_TimerHandler()
**
**  Purpose:
**    1hz "isr" routine for linux/OSX
**    This timer handler will execute 4 times a second.
**
**  Arguments:
**    (none)
**
**  Return:
**    (none)
*/
void CFE_PSP_TimerHandler (int signum)
{
      /*
      ** call the CFE_TIME 1hz ISR
      */
      if((TimerCounter % 4) == 0) CFE_TIME_1HZ_FUNCTION();

	  /* update timer counter */
	  TimerCounter++;
}

/******************************************************************************
**  Function:  CFE_PSP_DisplayUsage
**
**  Purpose:
**    Display program usage, and exit.
**
**  Arguments:
**    Name -- the name of the binary.
**
**  Return:
**    (none)
*/
void CFE_PSP_DisplayUsage(char *Name )
{

   printf("usage : %s [-R <value>] [-S <value>] [-C <value] [-N <value] [-I <value] [-h] \n", Name);
   printf("\n");
   printf("        All parameters are optional and can be used in any order\n");
   printf("\n");
   printf("        Parameters include:\n");
   printf("        -R [ --reset ] Reset Type is one of:\n");
   printf("             PO   for Power On reset ( default )\n");
   printf("             PR   for Processor Reset\n");
   printf("        -S [ --subtype ] Reset Sub Type is one of\n");
   printf("             1   for  Power on ( default )\n");
   printf("             2   for  Push Button Reset\n");
   printf("             3   for  Hardware Special Command Reset\n");
   printf("             4   for  Watchdog Reset\n");
   printf("             5   for  Reset Command\n");
   printf("        -C [ --cpuid ]   CPU ID is an integer CPU identifier.\n");
   printf("             The default  CPU ID is from the platform configuration file: %d\n",CFE_PLATFORM_CPU_ID);
   printf("        -N [ --cpuname ] CPU Name is a string to identify the CPU.\n");
   printf("             The default  CPU Name is from the platform configuration file: %s\n",CFE_PLATFORM_CPU_NAME);
   printf("        -I [ --scid ]    Spacecraft ID is an integer Spacecraft identifier.\n");
   printf("             The default Spacecraft ID is from the mission configuration file: %d\n",CFE_MISSION_SPACECRAFT_ID);
   printf("        -h [ --help ]    This message.\n");
   printf("\n");
   printf("       Example invocation:\n");
   printf(" \n");
   printf("       Short form:\n");
   printf("       %s -R PO -S 1 -C 1 -N CPU1 -I 32\n",Name);
   printf("       Long form:\n");
   printf("       %s --reset PO --subtype 1 --cpuid 1 --cpuname CPU1 --scid 32\n",Name);
   printf(" \n");

   exit( 1 );
}
/******************************************************************************
**  Function: CFE_PSP_ProcessArgumentDefaults
**
**  Purpose:
**    This function assigns defaults to parameters and checks to make sure
**    the user entered required parameters 
**
**  Arguments:
**    CFE_PSP_CommandData_t *CommandData -- A pointer to the command parameters.
**
**  Return:
**    (none)
*/
void CFE_PSP_ProcessArgumentDefaults(CFE_PSP_CommandData_t *CommandData)
{
   if ( CommandData->GotResetType == 0 )
   {
      strncpy(CommandData->ResetType, "PO", 2 );
      printf("CFE_PSP: Default Reset Type = PO\n");
      CommandData->GotResetType = 1;
   }
   
   if ( CommandData->GotSubType == 0 )
   {
      CommandData->SubType = 1;
      printf("CFE_PSP: Default Reset SubType = 1\n");
      CommandData->GotSubType = 1;
   }
   
   if ( CommandData->GotCpuId == 0 )
   {
      CommandData->CpuId = CFE_PLATFORM_CPU_ID;
      printf("CFE_PSP: Default CPU ID = %d\n",CFE_PLATFORM_CPU_ID);
      CommandData->GotCpuId = 1;
   }
   
   if ( CommandData->GotSpacecraftId == 0 )
   {
      CommandData->SpacecraftId = CFE_MISSION_SPACECRAFT_ID;
      printf("CFE_PSP: Default Spacecraft ID = %d\n",CFE_MISSION_SPACECRAFT_ID);
      CommandData->GotSpacecraftId = 1;
   }
   
   if ( CommandData->GotCpuName == 0 )
   {
      strncpy(CommandData->CpuName, CFE_PLATFORM_CPU_NAME, CFE_PSP_CPU_NAME_LENGTH );
      printf("CFE_PSP: Default CPU Name: %s\n",CFE_PLATFORM_CPU_NAME);
      CommandData->GotCpuName = 1;
   }

}

/******************************************************************************
**  Function:  CFE_PSP_1HzTimerTick
**
**  Purpose:
**    Simple wrapper function to call the CFE 1Hz ISR function
**
*/
void CFE_PSP_1HzTimerTick(uint32 timer_id, void *arg)
{
    CFE_TIME_1HZ_FUNCTION();
}


/******************************************************************************
**  Function:  CFE_PSP_SetupSystemTimer
**
**  Purpose:
**    BSP system time base and timer object setup.
**    This does the necessary work to start the 1Hz time tick required by CFE
**
**  Arguments:
**    (none)
**
**  Return:
**    (none)
**
** NOTE:
**      The handles to the timebase/timer objects are "start and forget"
**      as they are supposed to run forever as long as CFE runs.
**
**      If needed for e.g. additional timer creation, they can be recovered
**      using an OSAL GetIdByName() call.
**
**      This is preferred anyway -- far cleaner than trying to pass the uint32 value
**      up to the application somehow.
*/

void CFE_PSP_SetupSystemTimer(void)
{
    uint32 SystemTimebase;
    int32  Status;
    struct sigaction    sa;
    struct itimerval    timer;
    int ret;

    /*
    ** Init timer counter
    */
    TimerCounter = 0;

    /* Create the cFS master timebase that will be the default clock for all
     * of the cFS components (individual apps could still use a different clock).
     * This will just operate from the CPU realtime clock - sync function is NULL. */
    Status = OS_TimeBaseCreate(&SystemTimebase, "cFS-Master", NULL);
    if (Status == OS_SUCCESS)
    {
        /* Set the clock to trigger with 50ms resolution - slow enough that
         * it will not hog CPU resources but fast enough to have sufficient resolution
         * for most timing purposes.
         * (It may be better to move this to the mission config file)
         */
        Status = OS_TimeBaseSet(SystemTimebase, 50000, 50000);
    }
    else if (Status == OS_ERR_NOT_IMPLEMENTED &&
#ifdef	_ENHANCED_BUILD_
            GLOBAL_CONFIGDATA.CfeConfig != NULL &&
#endif/*_ENHANCED_BUILD_*/
            &CFE_TIME_1HZ_FUNCTION != NULL)
    {
        /* If NOT_IMPLEMENTED, then setup timers directly right here */
        Status = OS_SUCCESS;

        /*
         * NOTE - there is a race condition here, as time services are not yet running,
         * the data structures used in the 1Hz ISR callback are not yet initialize nor
         * is the semaphore created yet.  If this timer fires before that happens, the
         * result is undetermined.
         */
        OS_printf("CFE_PSP: Compatibility mode - Setting up Local 1Hz Interrupt\n");

        /*
        ** Install timer_handler as the signal handler for SIGALRM.
        */
        memset (&sa, 0, sizeof (sa));
        sa.sa_handler = &CFE_PSP_TimerHandler;

        /*
        ** Configure the timer to expire after 250ms
        */
        timer.it_value.tv_sec  = 0;
        timer.it_value.tv_usec = 250000;
        timer.it_interval.tv_sec  = 0;
        timer.it_interval.tv_usec = 250000;

        ret = sigaction (SIGALRM, &sa, NULL);

        if (ret < 0)
        {
            OS_printf("CFE_PSP: sigaction() error %d: %s \n", ret, strerror(errno));
            Status = OS_ERROR;
        }
        else
        {
            ret = setitimer (ITIMER_REAL, &timer, NULL);
            if (ret < 0)
            {
                OS_printf("CFE_PSP: setitimer() error %d: %s \n", ret, strerror(errno));
                Status = OS_ERROR;
            }
        }
    }

    /*
     * If anything failed, cFE/cFS timing (1Hz function) will not run properly
     *
     * During the 2015-08-11 CCB it was decided that we should not panic for this
     * condition, as other functions of the CFE may still work.  Since the CFE syslog
     * is not available to the PSP, our only notification facility is OS_printf.
     */
    if (Status != OS_SUCCESS)
    {
        OS_printf("CFE_PSP: Error configuring cFS timing: %d\n", (int)Status);
    }

}

