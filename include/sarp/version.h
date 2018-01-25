#ifndef SARP_VERSION_H
#define SARP_VERSION_H

#ifndef SARP_VERSION_MAJ
#define SARP_VERSION_MAJ "0"
#endif

#ifndef SARP_VERSION_MIN
#define SARP_VERSION_MIN "0"
#endif

#ifndef SARP_VERSION_PATCH
#define SARP_VERSION_PATCH "0"
#endif

#ifndef SARP_VERSION_NUM
#ifdef GIT_REV
#define SARP_VERSION_NUM "-"SARP_VERSION_MAJ"."SARP_VERSION_MIN"."SARP_VERSION_PATCH"-"GIT_REV
#else
#define SARP_VERSION_NUM "-"SARP_VERSION_MAJ"."SARP_VERSION_MIN"."SARP_VERSION_PATCH
#endif
#endif
#define SARP_VERSION "llarpd"SARP_VERSION_NUM

#endif