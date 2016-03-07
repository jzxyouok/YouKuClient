#include "network.h"
#include <QSslConfiguration>

class Network::Private
{
public:
    Private(Network *q) :
        manager(new QNetworkAccessManager(q))
    {
    }

    QNetworkAccessManager *manager;
};

Network *Network::instance()
{
    static Network network;
    return &network;
}

Network::Network(QObject *parent) :
    QObject(parent),
    d(new Network::Private(this))
{
    connect(d->manager, &QNetworkAccessManager::finished,
            this, &Network::finished);
}

Network::~Network()
{
    delete d;
    d = 0;
}

QNetworkReply *Network::get(const QString &url)
{
    QNetworkRequest request;
    QSslConfiguration config;

    QSslConfiguration conf = request.sslConfiguration();
    conf.setPeerVerifyMode(QSslSocket::VerifyNone);      //设置检验证书模式
    conf.setProtocol(QSsl::TlsV1SslV3);                  //设置检验证书协议
    request.setSslConfiguration(conf);

    request.setUrl(QUrl(url));
    return d->manager->get(request);
}
