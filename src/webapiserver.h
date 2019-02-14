/*********************************************************************************************
**                                                                                          **
** Copyright © «2019» «OBSHCHESTVO S OGRANICHENNOY OTVETSTVENNOST'YU "INTERNET VESHCHEY"»   **
** Copyright © «2019» «ОБЩЕСТВО С ОГРАНИЧЕННОЙ ОТВЕТСТВЕННОСТЬЮ "ИНТЕРНЕТ ВЕЩЕЙ"»           **
**                                                                                          **
**********************************************************************************************/
#ifndef WEBAPISERVER_H
#define WEBAPISERVER_H

#include <QObject>
#include <QList>
#include <QByteArray>
#include <QSslError>
#include "QWebSocketServer"
#include "QWebSocket"
#include <QDebug>
#include <QFile>
#include <QSslCertificate>
#include <QSslKey>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QCoreApplication>
#include <random>
#include "net868client.h"
#include "converter.h"
#include "settings.h"
#include "appinfo.h"

class WebApiServer : public QObject
{
    Q_OBJECT
public:

    explicit WebApiServer(quint16 port, bool useSsl, QObject *parent = Q_NULLPTR);

    virtual ~WebApiServer();

private Q_SLOTS:

    void onNewConnection();

    void processTextMessage(QString message);

    void processBinaryMessage(QByteArray message);

    void socketDisconnected();

    void onSslErrors(const QList<QSslError> &_err);

    quint64 genCid();

    void sendAnswer(QByteArray _data);

    void printMsg(QObject *obj);

    void idleDisconnectTimeout(QString _devEui);

    void closeWebSocket(Converter *);

private:

    QWebSocketServer *m_pWebSocketServer;

    QList<QWebSocket *> m_clients;

    Net868Client *net868Client;

    QMap <QString, Converter *> converterList;

    QMutex m_mutex;
};

#endif //WEBAPISERVER_H
