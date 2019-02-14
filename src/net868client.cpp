/*********************************************************************************************
**                                                                                          **
** Copyright © «2019» «OBSHCHESTVO S OGRANICHENNOY OTVETSTVENNOST'YU "INTERNET VESHCHEY"»   **
** Copyright © «2019» «ОБЩЕСТВО С ОГРАНИЧЕННОЙ ОТВЕТСТВЕННОСТЬЮ "ИНТЕРНЕТ ВЕЩЕЙ"»           **
**                                                                                          **
** Класс взаимодействия с сервером Сеть868                                                  **
**                                                                                          **
**********************************************************************************************/
#include "net868client.h"

Net868Client::Net868Client(QObject *parent) : QObject(parent)
{
    qDebug() << "D::Net868 client started!";

    net868Token.clear();
    net868Adr = WEB_SERV_ADR;
    net868Port = WEB_SERV_PORT;

    QVariant _tok = Settings::getSetting("Net868_Server", "token");
    if(!_tok.isNull()){
        net868Token = _tok.toString();
    } else {
        Settings::setSetting("Net868_Server", "token", net868Token);
    }
    QVariant _serv = Settings::getSetting("Net868_Server", "adr");
    if(!_serv.isNull()){
        net868Adr = _serv.toString();
    } else {
        Settings::setSetting("Net868_Server", "adr", net868Adr);
    }
    QVariant _port = Settings::getSetting("Net868_Server", "port");
    if(!_port.isNull()){
        net868Port = _port.toInt();
    } else {
        Settings::setSetting("Net868_Server", "port", net868Port);
    }

    if(net868Token.isEmpty() || net868Adr.isEmpty()){
        qDebug() << "Connection parameters invalid. Check connection parameters and try againe.";
        exit(0);
    }

    startWebSocket();
    prepareParser();
}

Net868Client::~Net868Client()
{
    parser->stopParser();
    QEventLoop loop;
    QTimer::singleShot(3000, &loop, SLOT(quit()));
    connect(parserThread, SIGNAL(finished()), &loop, SLOT(quit()));
    loop.exec();
    disconnect(&m_webSocket, &QWebSocket::disconnected, this, &Net868Client::webSocketClosed);
    m_webSocket.close();
    qDebug() << "D::Destroyed Net868 client.";
}

void Net868Client::onConnected()
{
    qDebug() << "I::Connect to Net868 server OK!";
    //connect(&m_webSocket, &QWebSocket::textMessageReceived, this, &Net868Client::onTextMessageReceived);
    QString req = "{\"type\":\"Config\",\"container\":{\"deviceDataPortGlobal\":{\"exclude\":\"false\",\"port\":[\"2\",\"4\"]},\"deviceStatusPortGlobal\":{\"exclude\":\"false\",\"port\":[\"4\",\"2\"]},"
                  "\"deviceGlobal\":{\"exclude\":\"true\",\"eui\":[]},\"gatewayGlobal\":{\"exclude\":\"true\",\"eui\":[]},\"deviceDataPort\":{\"1\":{\"exclude\":\"false\",\"port\":[\"1\",\"2\"]}}}}";
    m_webSocket.sendTextMessage(req);
}

void Net868Client::startWebSocket()
{
    qDebug() << "D::SSL lib ver: " << QSslSocket::sslLibraryBuildVersionString();
    connect(&m_webSocket, &QWebSocket::connected, this, &Net868Client::onConnected);
    connect(&m_webSocket, &QWebSocket::disconnected, this, &Net868Client::webSocketClosed);
    connect(&m_webSocket, SIGNAL(sslErrors(QList<QSslError>)), this, SLOT(printSslErrors(QList<QSslError>)));
    connect(&m_webSocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(printWsErrors(QAbstractSocket::SocketError)));
    connect(&m_webSocket, &QWebSocket::textMessageReceived, this, &Net868Client::onTextMessageReceived);
    m_webSocket.open(QUrl(QString(net868Adr) + ":" + QString::number(net868Port) + "/wsapi?token=" + net868Token));
    qDebug() << "D::Connecting to server Net868 " << net868Adr << ":" << net868Port;
}

void Net868Client::onTextMessageReceived(QString message)
{
    //qDebug() << "D::Message received: " << message;

    QJsonDocument jsDoc = QJsonDocument::fromJson(message.toUtf8());
    QJsonObject jsObj = jsDoc.object();

    if(jsObj.value("type") == "AppDataMessage"){
        QJsonObject params = jsObj.value("container").toObject();
        QByteArray _arr;
        _arr.append(params.value("port").toInt());
        _arr.append(QByteArray::fromHex(params.value("data").toString().toUtf8()));
        loraQue.enqueue(qMakePair(params.value("deviceEui").toString(), _arr));
    }
    if(jsObj.value("type") == "ExternalCommandStatus"){
        QJsonObject params = jsObj.value("container").toObject();
        QString _status = params.value("status").toString();
        if(_status == "overdue") loraQue.enqueue(qMakePair(params.value("deviceEui").toString(), _status.toUtf8()));
    }
}

void Net868Client::webSocketClosed()
{
    qDebug() << "I::Net868 server connection lost";
    //disconnect(&m_webSocket, &QWebSocket::textMessageReceived, this, &Net868Client::onTextMessageReceived);
    QEventLoop loop;
    QTimer::singleShot(3000, &loop, SLOT(quit()));
    loop.exec();
    qDebug() << "D::Reconnecting to Net868 server...";
    m_webSocket.open(QUrl(QString(net868Adr) + ":" + QString::number(net868Port) + "/wsapi?token=" + net868Token));
}

void Net868Client::printSslErrors(QList<QSslError> errors)
{
    qDebug() << "E::SSL Errors: " << errors;
}

void Net868Client::printWsErrors(QAbstractSocket::SocketError)
{
    qDebug() << "E::WS Errors: " << m_webSocket.errorString();
}

void Net868Client::prepareParser()
{
    parser = new PackParser();
    parser->setLoraQueue(&loraQue);
    parserThread = new QThread();
    parser->moveToThread(parserThread);
    connect(parserThread, SIGNAL(started()), parser, SLOT(startParser()));
    connect(parser, SIGNAL(finished(int)), parserThread, SLOT(quit()));
    connect(parser, SIGNAL(finished(int)), parser, SLOT(deleteLater()));
    connect(parser, SIGNAL(sendLoraPack(LORA_PACK)), this, SLOT(sendMessageToNetServer(LORA_PACK)));
    connect(parser, SIGNAL(receivePackage(quint8,QString,QByteArray)), this, SLOT(transitReceivePackage(quint8,QString,QByteArray)));
    connect(parser, SIGNAL(createBufferAnswer(quint8,quint8,quint16,QString)), this, SLOT(transitCreateBufferAnswer(quint8,quint8,quint16,QString)));
    connect(parser, SIGNAL(sendDataBlockAnswer(quint8,quint8,quint16,quint16,QString)), this, SLOT(transitSendDataBlockAnswer(quint8,quint8,quint16,quint16,QString)));
    connect(parser, SIGNAL(finishSendPackAnswer(quint8,QString)), this, SLOT(transitFinishSendPackAnswer(quint8,QString)));
    connect(parser, SIGNAL(createBufferCmd(quint8,quint16,QString)), this, SLOT(transitCreateBufferCmd(quint8,quint16,QString)));
    connect(parser, SIGNAL(receivedDataBlock(quint8,quint16,QByteArray,QString)), this, SLOT(transitReceivedDataBlock(quint8,quint16,QByteArray,QString)));
    connect(parser, SIGNAL(finishReceivedPackAnswer(quint8,quint16,qint32,QString)), this, SLOT(transitFinishReceivedPackAnswer(quint8,quint16,qint32,QString)));

    parserThread->start();
}

void Net868Client::sendMessageToNetServer(LORA_PACK _loraPack)
{
    qDebug() << "D::Sending data to Net868 server!";
    mutex.lock();
    QString req = "{\"type\":\"SendData\",\"container\":{\"eui\":\"" + _loraPack.dev_eui + "\",\"port\":\""
            + QString::number(_loraPack.port) + "\",\"try_count\":\"0\",\"data\":\"" + _loraPack.data.toHex() + "\"}}";
    m_webSocket.sendTextMessage(req);
    mutex.unlock();
}

void Net868Client::addDataToQuery(LORA_PACK _lp)
{
    parser->addDataToQuery(_lp);
}

void Net868Client::transitReceivePackage(quint8 _interface, QString _devEui, QByteArray _data)
{
    emit receivePackage(_interface, _devEui, _data);
}

void Net868Client::transitCreateBufferAnswer(quint8 _resultCode, quint8 _port, quint16 _dataSize, QString _dev_eui)
{
    emit createBufferAnswer(_resultCode, _port, _dataSize, _dev_eui);
}

void Net868Client::transitSendDataBlockAnswer(quint8 _resultCode, quint8 _port, quint16 _offset, quint16 _dataWriten, QString _dev_eui)
{
    emit sendDataBlockAnswer(_resultCode, _port, _offset, _dataWriten, _dev_eui);
}

void Net868Client::transitFinishSendPackAnswer(quint8 _resultCode, QString _dev_eui)
{
    emit finishSendPackAnswer(_resultCode, _dev_eui);
}

void Net868Client::transitCreateBufferCmd(quint8 _port, quint16 _dataSize, QString _dev_eui)
{
    emit createBufferCmd(_port, _dataSize, _dev_eui);
}

void Net868Client::transitReceivedDataBlock(quint8 _port, quint16 _offset, QByteArray _data, QString _dev_eui)
{
    emit receivedDataBlock(_port, _offset, _data, _dev_eui);
}

void Net868Client::transitFinishReceivedPackAnswer(quint8 _port, quint16 _dataSize, qint32 _crc, QString _dev_eui)
{
    emit finishReceivedPackAnswer(_port, _dataSize, _crc, _dev_eui);
}
