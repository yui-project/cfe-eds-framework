/*
** File   : cfe_psp_voltab.c
** Author : Nicholas Yanchik / GSFC Code 582
**
**      Copyright (c) 2004-2011, United States Government as represented by 
**      Administrator for The National Aeronautics and Space Administration. 
**      All Rights Reserved.
**
**      This is governed by the NASA Open Source Agreement and may be used,
**      distributed and modified only pursuant to the terms of that agreement. 
**
**
** PSP Volume table for file systems
*/

/****************************************************************************************
                                    INCLUDE FILES
****************************************************************************************/
#include "common_types.h"
#include "osapi.h"

/* 
** OSAL volume table. This is the only file in the PSP that still has the 
** OS_ naming convention, since it belongs to the OSAL. 
*/
OS_VolumeInfo_t OS_VolumeTable [NUM_TABLE_ENTRIES] = 
{
/* Dev Name  Phys Dev  Vol Type        Volatile?  Free?     IsMounted? Volname  MountPt BlockSz */

/* cFE RAM Disk */
{"/ramdev0", " ",      RAM_DISK,        true,      true,     false,     " ",      " ",     0        },

/* cFE non-volatile Disk -- Auto-Mapped to an existing CF disk */
{"/eedev0",  "CF:0",      FS_BASED,        false,     false,     true,     "CF",      "/cf",     512        },

/* 
** Spare RAM disks to be used for SSR and other RAM disks 
*/
{"/ramdev1", " ",      RAM_DISK,        true,      true,     false,     " ",      " ",     0        },
{"/ramdev2", " ",      RAM_DISK,        true,      true,     false,     " ",      " ",     0        },
{"/ramdev3", " ",      RAM_DISK,        true,      true,     false,     " ",      " ",     0        },
{"/ramdev4", " ",      RAM_DISK,        true,      true,     false,     " ",      " ",     0        },
{"/ramdev5", " ",      RAM_DISK,        true,      true,     false,     " ",      " ",     0        },

/* 
** Hard disk mappings 
*/
{"/ssrdev0",  "/hd:0/SSR1", FS_BASED,        true,      true,     false,     " ",      " ",     0        },
{"/ssrdev1",  "/hd:0/SSR2", FS_BASED,        true,      true,     false,     " ",      " ",     0        },
{"/ssrdev2",  "/hd:0/SSR3", FS_BASED,        true,      true,     false,     " ",      " ",     0        },

{"unused",   "unused",    FS_BASED,        true,      true,     false,     " ",      " ",     0        },
{"unused",   "unused",    FS_BASED,        true,      true,     false,     " ",      " ",     0        },
{"unused",   "unused",    FS_BASED,        true,      true,     false,     " ",      " ",     0        },
{"unused",   "unused",    FS_BASED,        true,      true,     false,     " ",      " ",     0        }

};



