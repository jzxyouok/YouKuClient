#ifndef NETWORK_H
#define NETWORK_H

#include <QObject>
#include <QtNetwork/QNetworkReply>

class Network : public QObject
{
    Q_OBJECT
public:
    static Network * instance();
    ~Network();

    QNetworkReply *get(const QString &url);
signals:
    void finished(QNetworkReply *reply);
private:
    class Private;
    friend class Private;
    Private *d;

    explicit Network(QObject *parent = 0);
    Network(const Network &) Q_DECL_EQ_DELETE;
    Network& operator=(Network rhs) Q_DECL_EQ_DELETE;
};
#endif // NETWORK_H
