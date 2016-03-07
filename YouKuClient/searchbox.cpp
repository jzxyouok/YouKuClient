#include "searchbox.h"
#include "remoterequest.h"

#define VIDEO_SEARCH_URL "https://openapi.youku.com/v2/searches/video/by_keyword.json?client_id=a8556d66d167afd0&keyword=%1"

class SearchBox::Private
{
public:
    Private()
    {
        network = Network::instance();
    }

    void fetchVideoInfoByKey(const QString &url)
    {
        QNetworkReply *reply = network->get(url);
        replyMap.insert(reply, FetchVideoInfoByKey);
    }

    YouKuSuggest *completer;
    Network *network;
    QMap<QNetworkReply *, RemoteRequest> replyMap;
};

SearchBox::SearchBox(QWidget *parent)
    : QLineEdit(parent),
      d(new SearchBox::Private)
{
    d->completer = new YouKuSuggest(this);

    connect(this, SIGNAL(returnPressed()), this, SLOT(doSearch()));

    adjustSize();
    //resize(400, height());
    resize(400,30);
    setFocus();
}

SearchBox::~SearchBox()
{

}

void SearchBox::doSearch()
{
    d->completer->preventSuggest();
    QString url = QString(VIDEO_SEARCH_URL).arg(text());
    //发送通过关键词获取视频信息的网络请求
    d->fetchVideoInfoByKey(url);
    connect(d->network, &Network::finished, [=](QNetworkReply *reply) {
        RemoteRequest request = d->replyMap.value(reply);
        qDebug() << "requestsearchbox " << request;
        switch(request)
        {
        case FetchVideoInfoByKey:
        {
            if(reply->error() != QNetworkReply::NoError)
                return;

            QJsonParseError error;
            QJsonDocument jsonDocument = QJsonDocument::fromJson(reply->readAll(), &error);
            if(!(jsonDocument.isNull() || jsonDocument.isEmpty()) &&
                    jsonDocument.isObject())
            {
                QJsonObject jsonObject = jsonDocument.object();
                //发送显示搜索结果的信号(在MainWindow中接收)
                emit showSearchResult(&jsonObject);
            }
            break;
        }
        }

        reply->deleteLater();
    });
}
