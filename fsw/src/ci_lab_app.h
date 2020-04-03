/*******************************************************************************
**
**      GSC-18128-1, "Core Flight Executive Version 6.7"
**
**      Copyright (c) 2006-2019 United States Government as represented by
**      the Administrator of the National Aeronautics and Space Administration.
**      All Rights Reserved.
**
**      Licensed under the Apache License, Version 2.0 (the "License");
**      you may not use this file except in compliance with the License.
**      You may obtain a copy of the License at
**
**        http://www.apache.org/licenses/LICENSE-2.0
**
**      Unless required by applicable law or agreed to in writing, software
**      distributed under the License is distributed on an "AS IS" BASIS,
**      WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
**      See the License for the specific language governing permissions and
**      limitations under the License.
**
** File: ci_lab_app.h
**
** Purpose:
**   This file is main hdr file for the Command Ingest lab application.
**
*******************************************************************************/

#ifndef _ci_lab_app_h_
#define _ci_lab_app_h_

/*
** Required header files...
*/
#include "common_types.h"
#include "cfe_error.h"
#include "cfe_evs.h"
#include "cfe_sb.h"
#include "cfe_es.h"

#include "osapi.h"
#include "ccsds.h"

#include <string.h>
#include <errno.h>
#include <unistd.h>

/* Use the EDS generated type definitions */
#include "ci_lab_eds_defines.h"
#include "ci_lab_eds_typedefs.h"


/****************************************************************************/

#define CI_LAB_BASE_UDP_PORT 1235
#define CI_LAB_MAX_INGEST    1024
#define CI_LAB_PIPE_DEPTH    32

/************************************************************************
** Type Definitions
*************************************************************************/

/****************************************************************************/
/*
** Local function prototypes...
**
** Note: Except for the entry point (CI_LAB_AppMain), these
**       functions are not called from any other source module.
*/
void CI_Lab_AppMain(void);
void CI_LAB_TaskInit(void);
void CI_LAB_ResetCounters_Internal(void);
void CI_LAB_ReadUpLink(void);

#endif /* _ci_lab_app_h_ */
