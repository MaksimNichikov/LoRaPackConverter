/*********************************************************************************************
**                                                                                          **
** Copyright © «2019» «OBSHCHESTVO S OGRANICHENNOY OTVETSTVENNOST'YU "INTERNET VESHCHEY"»   **
** Copyright © «2019» «ОБЩЕСТВО С ОГРАНИЧЕННОЙ ОТВЕТСТВЕННОСТЬЮ "ИНТЕРНЕТ ВЕЩЕЙ"»           **
**                                                                                          **
**********************************************************************************************/
#ifndef APPINFO_H
#define APPINFO_H

#define stringify(v1) #v1
#define quote(v1) stringify(v1)

#define V_MAJOR 1
#define V_MINOR 0
#define V_BUILD 9
#define V_REVISION REVISION_VER

#define V_ALPHA 0
#define V_RC 0

#define V_VERSION  V_MAJOR.V_MINOR.V_BUILD.V_REVISION
#define V_SVERSION  V_MAJOR.V_MINOR.V_BUILD
#define APP_VERSION quote(V_SVERSION)

#if V_ALPHA > 0
#define V_SVERSION_STR quote(V_SVERSION)"-a"
#elif V_RC > 0
#define V_SVERSION_STR quote(V_SVERSION)"-rc"quote(V_RC)
#else
#define V_SVERSION_STR quote(V_SVERSION)
#endif

#define VER_FILEVERSION             V_MAJOR,V_MINOR,V_BUILD,V_REVISION
#define VER_FILEVERSION_STR         quote(V_VERSION)

#define VER_PRODUCTVERSION          V_MAJOR,V_MINOR,V_BUILD,V_REVISION
#define VER_PRODUCTVERSION_STR      quote(V_SVERSION)

#endif // APPINFO_H
