/*
** File   : cfe_psp_voltab.c
** Author : Nicholas Yanchik / GSFC Code 582
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
** cFE PSP Volume table for file systems
*/

/****************************************************************************************
                                    INCLUDE FILES
****************************************************************************************/
#include "common_types.h"
#include "osapi.h"
#include "osconfig.h"   

/* 
** OSAL volume table. This is the only file in the PSP that still has the 
** OS_ naming convention, since it belongs to the OSAL. 
*/
OS_VolumeInfo_t OS_VolumeTable [NUM_TABLE_ENTRIES] = 
{
/* Dev Name  Phys Dev  Vol Type        Volatile?  Free?     IsMounted? Volname  MountPt BlockSz */
{"/ramdev0", "./ram",       FS_BASED,        TRUE,      TRUE,     FALSE,     " ",      " ",     0        },
{"/ramdev1", "./ram1",      FS_BASED,        TRUE,      TRUE,     FALSE,     " ",      " ",     0        },
{"/ramdev2", "./ram2",      FS_BASED,        TRUE,      TRUE,     FALSE,     " ",      " ",     0        },
{"/ramdev3", "./ram3",      FS_BASED,        TRUE,      TRUE,     FALSE,     " ",      " ",     0        },
{"/ramdev4", "./ram4",      FS_BASED,        TRUE,      TRUE,     FALSE,     " ",      " ",     0        },

/*
** The following entry is a "pre-mounted" path to a non-volatile device
*/
{"/eedev0",  "./cf",      FS_BASED,        FALSE,     FALSE,    TRUE,     "CF",      "/cf",     512   },

{"unused",   "unused",    FS_BASED,        TRUE,      TRUE,     FALSE,     " ",      " ",     0        },
{"unused",   "unused",    FS_BASED,        TRUE,      TRUE,     FALSE,     " ",      " ",     0        },
{"unused",   "unused",    FS_BASED,        TRUE,      TRUE,     FALSE,     " ",      " ",     0        },
{"unused",   "unused",    FS_BASED,        TRUE,      TRUE,     FALSE,     " ",      " ",     0        },
{"unused",   "unused",    FS_BASED,        TRUE,      TRUE,     FALSE,     " ",      " ",     0        },
{"unused",   "unused",    FS_BASED,        TRUE,      TRUE,     FALSE,     " ",      " ",     0        },
{"unused",   "unused",    FS_BASED,        TRUE,      TRUE,     FALSE,     " ",      " ",     0        },
{"unused",   "unused",    FS_BASED,        TRUE,      TRUE,     FALSE,     " ",      " ",     0        }
};



