/* RotateDefs.h -- Rotate functions
2015-03-25 : Igor Pavlov : Public domain */

#ifndef __ROTATE_DEFS_H
#define __ROTATE_DEFS_H

/* new compilers can translate these macros to fast commands. */

#define rotlFixed(x, n) (((x) << (n)) | ((x) >> (32 - (n))))
#define rotrFixed(x, n) (((x) >> (n)) | ((x) << (32 - (n))))

#endif
