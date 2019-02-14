/*********************************************************************************************
**                                                                                          **
** Copyright © «2019» «OBSHCHESTVO S OGRANICHENNOY OTVETSTVENNOST'YU "INTERNET VESHCHEY"»   **
** Copyright © «2019» «ОБЩЕСТВО С ОГРАНИЧЕННОЙ ОТВЕТСТВЕННОСТЬЮ "ИНТЕРНЕТ ВЕЩЕЙ"»           **
**                                                                                          **
** Обработчик сигнала для корректного завершения приложения                                 **
**                                                                                          **
**********************************************************************************************/
#include "cleanexit.h"

static CleanExit *pCleanExit;

CleanExit::CleanExit(QObject *parent ) : QObject(parent)
{
    pCleanExit = this;
    signal(SIGINT, &CleanExit::exitQt);
    signal(SIGTERM, &(CleanExit::exitQt));
}

void CleanExit::exitQt(int sig)
{
    Q_UNUSED(sig);
    QCoreApplication::exit(0);
}
