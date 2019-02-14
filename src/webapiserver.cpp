/*********************************************************************************************
**                                                                                          **
** Copyright © «2019» «OBSHCHESTVO S OGRANICHENNOY OTVETSTVENNOST'YU "INTERNET VESHCHEY"»   **
** Copyright © «2019» «ОБЩЕСТВО С ОГРАНИЧЕННОЙ ОТВЕТСТВЕННОСТЬЮ "ИНТЕРНЕТ ВЕЩЕЙ"»           **
**                                                                                          **
** Модуль обработки входящих подключений                                                    **
**                                                                                          **
**********************************************************************************************/
#include "webapiserver.h"

QT_USE_NAMESPACE

WebApiServer::WebApiServer(quint16 port, bool useSsl, QObject *parent) :
    QObject(parent),
    m_pWebSocketServer(Q_NULLPTR),
    m_clients()
{
    net868Client = nullptr;
    if(useSsl){
        m_pWebSocketServer = new QWebSocketServer(QStringLiteral("LoRa Pack Converter"),
                                                  QWebSocketServer::SecureMode,
                                                  this);
        QSslConfiguration sslConfiguration;
        QFile certFile;
        QVariant _cert = Settings::getSetting("Security", "CA_CERTIFICATE");
        if(!_cert.isNull()){
            certFile.setFileName(qApp->applicationDirPath() + "/" + _cert.toString());
        } else {
            Settings::setSetting("Security", "CA_CERTIFICATE", "");

        }
        QFile keyFile;
        QVariant _keyFile = Settings::getSetting("Security", "RSA_PRIVATE_KEY");
        if(!_keyFile.isNull()){
            keyFile.setFileName(qApp->applicationDirPath() + "/" + _keyFile.toString());
        } else {
            Settings::setSetting("Security", "RSA_PRIVATE_KEY", "");

        }
        if(certFile.open(QIODevice::ReadOnly) && keyFile.open(QIODevice::ReadOnly)){
            QSslCertificate certificate(&certFile, QSsl::Pem);
            QSslKey sslKey(&keyFile, QSsl::Rsa, QSsl::Pem);
            certFile.close();
            keyFile.close();
            sslConfiguration.setPeerVerifyMode(QSslSocket::VerifyNone);
            sslConfiguration.setLocalCertificate(certificate);
            sslConfiguration.setPrivateKey(sslKey);
            sslConfiguration.setProtocol(QSsl::TlsV1SslV3);
            m_pWebSocketServer->setSslConfiguration(sslConfiguration);
        } else {
            qDebug() << "E::Error open SSL files. Check security configuration. The server cannot start.";
            return;
        }
    } else {
        m_pWebSocketServer = new QWebSocketServer(QStringLiteral("LoRa Pack Converter"),
                                                  QWebSocketServer::NonSecureMode,
                                                  this);
    }

    if (m_pWebSocketServer->listen(QHostAddress::Any, port))
    {
        qDebug() << "I::Server listening on port" << port;
        connect(m_pWebSocketServer, &QWebSocketServer::newConnection,
                this, &WebApiServer::onNewConnection);
        connect(m_pWebSocketServer, &QWebSocketServer::sslErrors,
                this, &WebApiServer::onSslErrors);
        net868Client = new Net868Client(this);
    } else {
        qDebug() << "E::ERROR listening on port" << port;
        exit(0);
    }
}

WebApiServer::~WebApiServer()
{
    if(net868Client) delete net868Client;
    m_pWebSocketServer->close();
    qDeleteAll(m_clients.begin(), m_clients.end());
}

void WebApiServer::onNewConnection()
{
    QWebSocket *pSocket = m_pWebSocketServer->nextPendingConnection();

    qDebug() << "D::Client connected:" << pSocket << pSocket->peerAddress().toIPv4Address();

    connect(pSocket, &QWebSocket::textMessageReceived, this, &WebApiServer::processTextMessage);
    connect(pSocket, &QWebSocket::binaryMessageReceived, this, &WebApiServer::processBinaryMessage);
    connect(pSocket, &QWebSocket::disconnected, this, &WebApiServer::socketDisconnected);

    m_clients << pSocket;
}

void WebApiServer::processTextMessage(QString message)
{
    QJsonDocument jsDoc = QJsonDocument::fromJson(message.toUtf8());
    QWebSocket *pClient = qobject_cast<QWebSocket *>(sender());
    if (pClient)
    {
        qDebug() << "D::Msg from websocket: " << message;

        QString _devEUI;
        if(jsDoc.isObject()){
            QJsonObject jsObj = jsDoc.object();
            if(!jsObj.value("DevEUI").toString().isEmpty()
                    && !jsObj.value("Data").toString().isEmpty()
                    && !jsObj.value("Interface").toString().isEmpty()){

                _devEUI = jsObj.value("DevEUI").toString().toUpper();
                LORA_PACK _lp;
                _lp.dev_eui = _devEUI;
                _lp.port = 4; //LoRa Port Number
                _lp.port_interface = jsObj.value("Interface").toString().toUInt();
                _lp.data = QByteArray::fromHex(jsObj.value("Data").toString().toUtf8());

                if(!converterList.contains(_devEUI)){
                    Converter *converter = new Converter(pClient);
                    qDebug() << "D::Converter " << converter << " created with DevEUI: " << _devEUI;
                    m_mutex.lock();
                    converterList.insert(_devEUI, converter);
                    m_mutex.unlock();
                    connect(converter, SIGNAL(sendLoraPack(LORA_PACK)), net868Client, SLOT(addDataToQuery(LORA_PACK)));
                    connect(net868Client, SIGNAL(receivePackage(quint8,QString,QByteArray)), converter, SLOT(receivePackage(quint8,QString,QByteArray)));
                    connect(converter, SIGNAL(sendAnswer(QByteArray)), this, SLOT(sendAnswer(QByteArray)));
                    connect(net868Client, SIGNAL(createBufferAnswer(quint8,quint8,quint16,QString)), converter, SLOT(createBufferAnswer(quint8,quint8,quint16,QString)));
                    connect(net868Client, SIGNAL(sendDataBlockAnswer(quint8,quint8,quint16,quint16,QString)), converter, SLOT(sendDataBlockAnswer(quint8,quint8,quint16,quint16,QString)));
                    connect(net868Client, SIGNAL(finishSendPackAnswer(quint8,QString)), converter, SLOT(finishSendPackAnswer(quint8,QString)));
                    connect(net868Client, SIGNAL(createBufferCmd(quint8,quint16,QString)), converter, SLOT(createBufferCmd(quint8,quint16,QString)));
                    connect(net868Client, SIGNAL(receivedDataBlock(quint8,quint16,QByteArray,QString)), converter, SLOT(receivedDataBlock(quint8,quint16,QByteArray,QString)));
                    connect(net868Client, SIGNAL(finishReceivedPackAnswer(quint8,quint16,qint32,QString)), converter, SLOT(finishReceivedPackAnswer(quint8,quint16,qint32,QString)));

                    connect(converter, SIGNAL(idleDisconnectTimeout(QString)), this, SLOT(idleDisconnectTimeout(QString)));
                    connect(converter, SIGNAL(destroyed(QObject*)), this, SLOT(printMsg(QObject*)));
                    connect(pClient, SIGNAL(disconnected()), converter, SLOT(deleteLater()));

                    converter->setDevEUI(_devEUI);
                    converter->setDataPackage(_lp);
                } else {
                    //converterList.value(_devEUI)->setDataPackage(_lp);
                    QJsonObject jsObj;
                    jsObj.insert("status", "ERROR");
                    jsObj.insert("info", "Data already sent.");
                    jsObj.insert("DevEUI", _devEUI);
                    pClient->sendTextMessage(QJsonDocument(jsObj).toJson());
                }

            } else {
                QJsonObject _jsObj;
                _jsObj.insert("status", "ERROR");
                _jsObj.insert("info", "Invalid request parameters.");
                _jsObj.insert("DevEUI", _devEUI);
                pClient->sendTextMessage(QJsonDocument(_jsObj).toJson());
                qDebug() << "E::Invalid request parameters.";
            }
        } else {
            QJsonObject _jsObj;
            _jsObj.insert("status", "ERROR");
            _jsObj.insert("info", "Invalid JsonObject frame.");
            pClient->sendTextMessage(QJsonDocument(_jsObj).toJson());
            qDebug() << "E::Bad JSON frame!";
        }
    }
}

void WebApiServer::processBinaryMessage(QByteArray message)
{
    QWebSocket *pClient = qobject_cast<QWebSocket *>(sender());
    if (pClient)
    {
        pClient->sendBinaryMessage(message);
    }
}

void WebApiServer::socketDisconnected()
{
    QWebSocket *pClient = qobject_cast<QWebSocket *>(sender());
    if (pClient)
    {
        qDebug() << "D::Client disconnected";
        m_clients.removeAll(pClient);
        pClient->deleteLater();
    }
}

void WebApiServer::onSslErrors(const QList<QSslError> &_err)
{
    qDebug() << "E::Ssl errors occurred" << _err;
}

quint64 WebApiServer::genCid()
{
    std::random_device rd;
    std::mt19937_64 gen(rd());
    return gen();
}

void WebApiServer::sendAnswer(QByteArray _data)
{
    Converter *conv = qobject_cast<Converter *>(sender());
    if (conv){
        QWebSocket *pClient = qobject_cast<QWebSocket *>(conv->parent());
        if (pClient)
        {
            //qDebug() << "D::Msg from parcer: " << _data;
            qDebug() << "D::Send message to app...";
            pClient->sendTextMessage(_data);
        }
    }
}

void WebApiServer::printMsg(QObject *obj)
{
    foreach (QString key, converterList.keys()){
        if(converterList.value(key) == obj){
            QMutexLocker locker(&m_mutex);
            converterList.take(key);
            qDebug() << "D::Converter " << obj << " with DevEUI: " << key << " deleted!";
        }
    }
}

void WebApiServer::idleDisconnectTimeout(QString _devEui)
{
    Converter *conv = converterList.take(_devEui);
    qDebug() << "I::Converter with DevEUI: " << _devEui << " " << conv << " finished";
    if(converterList.count() <= 0){
        qDebug() << "I::All converters closed.";
        //closeWebSocket(conv);
    }
    conv->deleteLater();
}

void WebApiServer::closeWebSocket(Converter *conv)
{
    if (conv){
        QWebSocket *pClient = qobject_cast<QWebSocket *>(conv->parent());
        if (pClient)
        {
            qDebug() << "I::Close connection by idle timeout";
            pClient->close();
        }
    }
}
