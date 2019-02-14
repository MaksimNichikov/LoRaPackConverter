/*********************************************************************************************
**                                                                                          **
** Copyright © «2019» «OBSHCHESTVO S OGRANICHENNOY OTVETSTVENNOST'YU "INTERNET VESHCHEY"»   **
** Copyright © «2019» «ОБЩЕСТВО С ОГРАНИЧЕННОЙ ОТВЕТСТВЕННОСТЬЮ "ИНТЕРНЕТ ВЕЩЕЙ"»           **
**                                                                                          **
**********************************************************************************************/
#ifndef NET868CLIENT_H
#define NET868CLIENT_H

#include <QObject>
#include <QVariant>
#include <QThread>
#include <QMap>
#include <QList>
#include <QFile>
#include <QDir>
#include <QWebSocket>
#include <QMutex>
#include <QPair>
#include <QEventLoop>
#include <QDateTime>
#include <QTimer>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>

#include "packparser.h"
#include "settings.h"

#define WEB_SERV_ADR "wss://bs.net868.ru"
#define WEB_SERV_PORT 20010


class Net868Client : public QObject
{
    Q_OBJECT
public:
    explicit Net868Client(QObject *parent = 0);

    ~Net868Client();

    PackParser *parser;

signals:

    void receivePackage(quint8 _interface, QString _devEui, QByteArray _data);

    void createBufferAnswer(quint8 _resultCode, quint8 _port, quint16 _dataSize, QString _dev_eui);

    void sendDataBlockAnswer(quint8 _resultCode, quint8 _port, quint16 _offset, quint16 _dataWriten, QString _dev_eui);

    void finishSendPackAnswer(quint8 _resultCode, QString _dev_eui);

    void createBufferCmd(quint8 _port, quint16 _dataSize, QString _dev_eui);

    void receivedDataBlock(quint8 _port, quint16 _offset, QByteArray _data, QString _dev_eui);

    void finishReceivedPackAnswer(quint8 _port, quint16 _dataSize, qint32 _crc, QString _dev_eui);

private slots:

    void startWebSocket();

    void onConnected();

    void onTextMessageReceived(QString message);

    void webSocketClosed();

    void printSslErrors(QList<QSslError> errors);

    void printWsErrors(QAbstractSocket::SocketError);

    void prepareParser();

    void sendMessageToNetServer(LORA_PACK _loraPack);

    void addDataToQuery(LORA_PACK _lp);

    void transitReceivePackage(quint8 _interface, QString _devEui, QByteArray _data);

    void transitCreateBufferAnswer(quint8 _resultCode, quint8 _port, quint16 _dataSize, QString _dev_eui);

    void transitSendDataBlockAnswer(quint8 _resultCode, quint8 _port, quint16 _offset, quint16 _dataWriten, QString _dev_eui);

    void transitFinishSendPackAnswer(quint8 _resultCode, QString _dev_eui);

    void transitCreateBufferCmd(quint8 _port, quint16 _dataSize, QString _dev_eui);

    void transitReceivedDataBlock(quint8 _port, quint16 _offset, QByteArray _data, QString _dev_eui);

    void transitFinishReceivedPackAnswer(quint8 _port, quint16 _dataSize, qint32 _crc, QString _dev_eui);

protected:

    QThread *thread;

    QMultiMap <QString, QMap <QString, QByteArray *>> deviceIdList;

    QMap <int, QString> dbDevIdList;

    QWebSocket m_webSocket;

    bool reconnectFlag;

    QThread *parserThread;

    QQueue<QPair<QString, QByteArray>> loraQue;

    QStringList alreadyUpdatingList;

    QMutex mutex;

    QString net868Token;

    int net868Port;

    QString net868Adr;
};

#endif // NET868CLIENT_H
