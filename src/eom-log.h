/**************************************************************************

eom (external output manager)

Copyright 2014 Samsung Electronics co., Ltd. All Rights Reserved.

Contact:
SooChan Lim <sc1.lim@samsung.com>
Boram Park <boram1288.park@samsung.com>
Changyeon Lee <cyeon.lee@samsung.com>

Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sub license, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice (including the
next paragraph) shall be included in all copies or substantial portions
of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
IN NO EVENT SHALL PRECISION INSIGHT AND/OR ITS SUPPLIERS BE LIABLE FOR
ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

**************************************************************************/

#ifndef __EOM_LOG_H__
#define __EOM_LOG_H__

#include <config.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#ifndef API
#define API __attribute__ ((visibility("default")))
#endif

#undef HAVE_DLOG

extern bool eom_debug_on;

#ifndef HAVE_CAPI_0_1_1
static inline void set_last_result (int i) {;}
#endif

#ifdef HAVE_DLOG
#define LOG_TAG	"EOM"
#include <dlog.h>
#define INFO(fmt, arg...)  \
    if (eom_debug_on) SECURE_SLOGI(fmt, ##arg)
#define WARN(fmt, arg...)  \
    if (eom_debug_on) SECURE_SLOGW(fmt, ##arg)
#define ERR(fmt, arg...)   SECURE_SLOGE(fmt, ##arg)
#define ERRNO(fmt, arg...) SECURE_SLOGE("(err=%s(%d)) "fmt, strerror(errno), errno, ##arg)
#define FATAL(fmt, arg...) SECURE_SLOGF(fmt, ##arg)
#else   /* HAVE_DLOG */
#include <stdlib.h>
#define INFO(fmt, arg...)  \
    if (eom_debug_on) fprintf(stdout,"[%s:%d] "fmt"\n", __FUNCTION__, __LINE__, ##arg)
#define WARN(fmt, arg...)  \
    if (eom_debug_on) fprintf(stderr,"[%s:%d] "fmt"\n", __FUNCTION__, __LINE__, ##arg)
#define ERR(fmt, arg...)   fprintf(stderr,"[%s:%d] "fmt"\n", __FUNCTION__, __LINE__, ##arg)
#define ERRNO(fmt, arg...) fprintf(stderr,"[%s:%d](err=%s(%d)) "fmt"\n", __FUNCTION__, __LINE__, strerror(errno), errno, ##arg)
#define FATAL(fmt, arg...) fprintf(stderr,"[%s:%d] "fmt"\n", __FUNCTION__, __LINE__, ##arg)
#endif   /* HAVE_DLOG */

#define WARN_IF_FAIL(cond)              {if (!(cond)) { ERR ("'%s' failed", #cond);}}
#define RET_IF_FAIL(cond)               {if (!(cond)) { ERR ("'%s' failed", #cond); return; }}
#define RETV_IF_FAIL(cond, val)         {if (!(cond)) { ERR ("'%s' failed", #cond); return val; }}
#define RETV_IF_ERRNO(cond, val, errno) {if (!(cond)) { ERRNO ("'%s' failed", #cond); return val; }}
#define GOTO_IF_FAIL(cond, dst)         {if (!(cond)) { ERR ("'%s' failed", #cond); goto dst; }}
#define GOTO_IF_ERRNO(cond, dst, errno) {if (!(cond)) { ERRNO ("'%s' failed", #cond); goto dst; }}
#define NEVER_GET_HERE()                ERR("** NEVER GET HERE **\n")

#endif  /* __EOM_LOG_H__ */
