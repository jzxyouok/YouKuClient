#include "mainwindow.h"
#include "videomessage.h"
#include "network.h"
#include "searchbox.h"
#include "remoterequest.h"
#include "mysqloperate.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QList>
#include <QPixmap>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QEventLoop>
#include <QListView>
#include <QWebView>
#include <QFile>
#include <QSignalMapper>
#include <QPushButton>
#include <QLabel>

#define LIST_COUNT 20
#define ICON_WIDTH 150
#define ICON_HEIGHT 150
#define ITEM_SIZE 150

#define VIDEO_INFO_URL "https://openapi.youku.com/v2/videos/by_category.json?client_id=a8556d66d167afd0&category=%1"

class MainWindow::Private
{
public:
    Private()
    {
        network = Network::instance();
    }

    void fetchVideoInfo(const QString &id)
    {
        QString url = QString(VIDEO_INFO_URL).arg(id);
        QNetworkReply *reply = network->get(QString(url));
        replyMap.insert(reply, FetchVideoInfo);
    }

    QNetworkReply *fetchVideoScreenShot(const QString &url)
    {
        network->get(url);
    }

    Network *network;
    QToolButton *m_record;             //播放记录按钮
    QToolButton *m_moviesButton;       //电影选项按钮
    QToolButton *m_tvButton;           //电视剧选项按钮
    QToolButton *m_animeButton;        //动漫选项按钮
    QToolButton *m_varietyButton;      //综艺选项按钮
    QToolButton *m_musicButton;        //音乐选项按钮
    QToolButton *m_educationButton;    //教育选项按钮
    QToolButton *m_documentaryButton;  //纪录片选项按钮
    QToolButton *m_sportsButton;       //体育选项按钮
    QToolButton *m_autosButton;        //汽车选项按钮
    QToolButton *m_techButton;         //科技选项按钮
    QToolButton *m_gamesButton;        //游戏选项按钮
    QToolButton *m_lifeStyleButton;    //生活选项按钮
    QToolButton *m_fashionButton;      //时尚选项按钮
    QToolButton *m_parentingButton;    //亲子选项按钮
    SearchBox *m_searchBox;            //搜索框
    QPushButton *m_searchButton;       //搜索按钮
    QLabel *m_lable;
    QSignalMapper *m_categoryMapper;
    QSignalMapper *m_searchMapper;
    QListWidget *m_vListWidget;
    QWidget *m_widget;
    QMap<QNetworkReply *, RemoteRequest> replyMap;
    QList<VideoMessage *> messages;    //视频信息列表
    QList<VideoMessage *> searchResult;//搜索结果列表
    QPixmap images[LIST_COUNT];        //截图数组
};

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      d(new MainWindow::Private)
{
    resize(1000, 600);

    //设置窗口控件
    setWindowWidget();

    //设置背景色
    QPalette palette(this->palette());
    palette.setColor(QPalette::Background, Qt::white);
    this->setPalette(palette);

    //请求tv类型视频信息
    requestVideoInfo("TV");

    connect(d->m_vListWidget, SIGNAL(currentRowChanged(int)),
            this, SLOT(currentRow(int)));

    //QSignalMapper将一个无参的信号翻译成带参数(int、QString、QObject *、
    //QWidget *)的信号，并将之转发
    d->m_categoryMapper =  new QSignalMapper;
    d->m_searchMapper = new QSignalMapper;

    connectButtonWithMapper(d->m_widget->children());
    //将视频类型按钮转发的信号连接到最终的槽函数
    connect(d->m_categoryMapper, SIGNAL(mapped(QString)),
            this, SLOT(categoryButtonMapper(QString)));
    //接收搜索结果信号，将接收的结果显示到列表中
    connect(d->m_searchBox, SIGNAL(showSearchResult(QJsonObject *)),
            this, SLOT(handleSearchResult(QJsonObject *)));
    //搜索按钮点击
    connect(d->m_searchButton, SIGNAL(clicked()),
            d->m_searchBox, SLOT(doSearch()));
    //记录按钮点击，从数据库中读出近期播放视频信息并显示在列表
    connect(d->m_record, SIGNAL(clicked()),
            this, SLOT(showPlayRecord()));
}

MainWindow::~MainWindow()
{
    delete d;
    d = 0;
}

void MainWindow::setWindowWidget()
{
    d->m_widget = new QWidget;

    QPixmap pixmap(":/youku.png");
    d->m_lable = new QLabel;
    d->m_lable->setPixmap(pixmap);
    //d->m_lable->resize(20,20);

    d->m_searchBox = new SearchBox;
    d->m_searchButton = new QPushButton;
    d->m_searchButton->setIcon(QIcon(":/search.png"));
    d->m_searchButton->setIconSize(QSize(20,20));
    d->m_searchButton->setText(tr("搜索"));

    QHBoxLayout *searchLayout = new QHBoxLayout;
    searchLayout->addWidget(d->m_lable);
    searchLayout->addSpacing(180);
    searchLayout->addWidget(d->m_searchBox);
    searchLayout->addWidget(d->m_searchButton);
    searchLayout->addSpacing(180);

    //播放记录选项按钮的设置
    d->m_record = new QToolButton;
    d->m_record->setText(tr("记录"));
    //保持按钮被选中的状态
    d->m_record->setCheckable(true);
    //保持只有一个选中的按钮
    d->m_record->setAutoExclusive(true);

    //电视剧选项按钮的设置
    d->m_tvButton = new QToolButton;
    d->m_tvButton->setText(tr("电视剧"));
    d->m_tvButton->setCheckable(true);
    d->m_tvButton->setAutoExclusive(true);

    //电影选项按钮的设置
    d->m_moviesButton = new QToolButton;
    d->m_moviesButton->setText(tr("电影"));
    d->m_moviesButton->setCheckable(true);
    d->m_moviesButton->setAutoExclusive(true);

    //动漫选项按钮的设置
    d->m_animeButton = new QToolButton;
    d->m_animeButton->setText(tr("动漫"));
    d->m_animeButton->setCheckable(true);
    d->m_animeButton->setAutoExclusive(true);

    //综艺选项按钮的设置
    d->m_varietyButton = new QToolButton;
    d->m_varietyButton->setText(tr("综艺"));
    d->m_varietyButton->setCheckable(true);
    d->m_varietyButton->setAutoExclusive(true);

    //音乐选项按钮的设置
    d->m_musicButton = new QToolButton;
    d->m_musicButton->setText(tr("音乐"));
    d->m_musicButton->setCheckable(true);
    d->m_musicButton->setAutoExclusive(true);

    //教育选项按钮的设置
    d->m_educationButton = new QToolButton;
    d->m_educationButton->setText(tr("教育"));
    d->m_educationButton->setCheckable(true);
    d->m_educationButton->setAutoExclusive(true);

    //纪录片选项按钮的设置
    d->m_documentaryButton = new QToolButton;
    d->m_documentaryButton->setText(tr("纪录片"));
    d->m_documentaryButton->setCheckable(true);
    d->m_documentaryButton->setAutoExclusive(true);

    //体育选项按钮的设置
    d->m_sportsButton = new QToolButton;
    d->m_sportsButton->setText(tr("体育"));
    d->m_sportsButton->setCheckable(true);
    d->m_sportsButton->setAutoExclusive(true);

    //汽车选项按钮的设置
    d->m_autosButton = new QToolButton;
    d->m_autosButton->setText(tr("汽车"));
    d->m_autosButton->setCheckable(true);
    d->m_autosButton->setAutoExclusive(true);

    //科技选项按钮的设置
    d->m_techButton = new QToolButton;
    d->m_techButton->setText(tr("科技"));
    d->m_techButton->setCheckable(true);
    d->m_techButton->setAutoExclusive(true);

    //游戏选项按钮的设置
    d->m_gamesButton = new QToolButton;
    d->m_gamesButton->setText(tr("游戏"));
    d->m_gamesButton->setCheckable(true);
    d->m_gamesButton->setAutoExclusive(true);

    //生活选项按钮的设置
    d->m_lifeStyleButton = new QToolButton;
    d->m_lifeStyleButton->setText(tr("生活"));
    d->m_lifeStyleButton->setCheckable(true);
    d->m_lifeStyleButton->setAutoExclusive(true);

    //时尚选项按钮的设置
    d->m_fashionButton = new QToolButton;
    d->m_fashionButton->setFixedHeight(25);
    d->m_fashionButton->setText(tr("时尚"));
    d->m_fashionButton->setCheckable(true);
    d->m_fashionButton->setAutoExclusive(true);

    //亲子选项按钮的设置
    d->m_parentingButton = new QToolButton;
    d->m_parentingButton->setText(tr("亲子"));
    d->m_parentingButton->setCheckable(true);
    d->m_parentingButton->setAutoExclusive(true);

    //对按钮进行水平布局
    QVBoxLayout *buttonLayout = new QVBoxLayout;
    buttonLayout->addWidget(d->m_record);
    buttonLayout->addWidget(d->m_tvButton);
    buttonLayout->addWidget(d->m_moviesButton);
    buttonLayout->addWidget(d->m_animeButton);
    buttonLayout->addWidget(d->m_varietyButton);
    buttonLayout->addWidget(d->m_musicButton);
    buttonLayout->addWidget(d->m_documentaryButton);
    buttonLayout->addWidget(d->m_sportsButton);
    buttonLayout->addWidget(d->m_autosButton);
    buttonLayout->addWidget(d->m_techButton);
    buttonLayout->addWidget(d->m_gamesButton);
    buttonLayout->addWidget(d->m_lifeStyleButton);
    buttonLayout->addWidget(d->m_fashionButton);
    buttonLayout->addWidget(d->m_parentingButton);
    buttonLayout->addWidget(d->m_educationButton);
    buttonLayout->setContentsMargins(0,0,0,0);
    buttonLayout->addStretch();

    //显示视频信息的列表的定义
    d->m_vListWidget = new QListWidget;
    QHBoxLayout *layout = new QHBoxLayout;
    layout->addLayout(buttonLayout);
    layout->addSpacing(15);
    layout->addWidget(d->m_vListWidget);
    QVBoxLayout *mainLayout = new QVBoxLayout(d->m_widget);
    mainLayout->addLayout(searchLayout);
    mainLayout->addLayout(layout);

    setCentralWidget(d->m_widget);
}

//请求视频信息
void MainWindow::requestVideoInfo(QString category)
{
    //发送获取视频信息请求
    d->fetchVideoInfo(category);
    //当请求返回时，执行相关槽函数
    connect(d->network, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(replyFinish(QNetworkReply*)));
}

//处理网络返回数据
void MainWindow::replyFinish(QNetworkReply *reply)
{
    RemoteRequest request = d->replyMap.value(reply);
    //qDebug() << "requestInfo " << request;
    switch(request)
    {
    case FetchVideoInfo:
    {
        if(reply->error() != QNetworkReply::NoError)
            return;

        QJsonParseError error;
        QJsonDocument jsonDocument = QJsonDocument::fromJson(reply->readAll(), &error);
        if(!(jsonDocument.isNull() || jsonDocument.isEmpty()) &&
                jsonDocument.isObject())
        {
            //获取最外层的对象
            QJsonObject jsonObject = jsonDocument.object();

            saveVideoInfo(jsonObject);
        }
    }
    }
    reply->deleteLater();
    //所有视频信息搜索完成后刷新列表
    reloadData();
}

//保存视频信息，并设置截图
void MainWindow::saveVideoInfo(QJsonObject jsonObject)
{
    if(jsonObject.contains("videos") && jsonObject.value("videos").isArray())
    {
        d->messages.clear();
        d->messages.clear();
        QJsonArray jsonArray = jsonObject["videos"].toArray();

        int count = jsonArray.count();
        if(count > LIST_COUNT)
            count = LIST_COUNT;

        for(int i = 0; i < count; i++)
        {
            //解析出一个视频信息，并将之封装到VideoMessage中
            //再添加到messages数组中便于以后操作
            QJsonObject object = jsonArray.at(i).toObject();
            QVariantMap data = object.toVariantMap();
            VideoMessage *message = new VideoMessage;
            message->setId(data[QLatin1String("id")].toString());
            message->setTitle(data[QLatin1String("title")].toString());
            message->setThumbnail(data[QLatin1String("thumbnail")].toString());
            message->setLink(data[QLatin1String("link")].toString());
            message->setCategory(data[QLatin1String("category")].toString());
            d->messages.append(message);
            //qDebug() << "thumnail:" << message->thumbnail() << endl;
            setScreenShot(message->thumbnail(), i);
        }
    }
}

void MainWindow::setScreenShot(const QString &url, int index)
{
    qDebug() << index;
    QEventLoop loop;

    //发送获取视频截图请求
    QNetworkAccessManager manager;
    QNetworkReply *reply = manager.get(QNetworkRequest(url));
    //QNetworkReply *reply = d->fetchVideoScreenShot(url);
    //qDebug() << reply;

    //请求结束并下载完成后，退出子事件循环
    connect(reply, SIGNAL(finished()),
            &loop, SLOT(quit()));
    //开启子事件循环
    loop.exec();

    QPixmap pixmap;
    QByteArray data = reply->readAll();
    //qDebug() << "data " << data.toBase64() << endl;
    pixmap.loadFromData(data);
    d->images[index] = pixmap;
    reloadData();
}

void MainWindow::reloadData()
{
    //刷新列表前清除之前的已经存在的行重头设置
    d->m_vListWidget->clear();
    //设置列表中的图片大小
    d->m_vListWidget->setIconSize(QSize(ICON_WIDTH,ICON_HEIGHT));
    //设置图片调整模式
    d->m_vListWidget->setResizeMode(QListView::Adjust);
    //设置列表显示模式
    d->m_vListWidget->setViewMode(QListView::IconMode);
    //设置列表单元项不可拖动
    d->m_vListWidget->setMovement(QListView::Static);
    //设置列表单元项间的间距
    d->m_vListWidget->setSpacing(10);

    for(int i = 0; i < d->messages.count(); i++)
    {
        VideoMessage *message = d->messages.at(i);
        QListWidgetItem *item = new QListWidgetItem(message->title());
        item->setSizeHint(QSize(ITEM_SIZE,ITEM_SIZE));
        if(!d->images[i].isNull())
            item->setIcon(QIcon(d->images[i]));
        d->m_vListWidget->addItem(item);
    }
}

//获取到当前行
void MainWindow::currentRow(int row)
{
    loadWebAndPlayVideo(row);
}

//加载网页播放视频
void MainWindow::loadWebAndPlayVideo(int row)
{
    if(row < 0 || row >= d->messages.count())
        return;

    //qDebug() << "row " << row;
    QWebView *webView = new QWebView;
    QString content;
    MysqlOperate database;
    QFile source(":/videosource.htm");

    if(source.open(QIODevice::ReadOnly))
    {
        QTextStream in(&source);
        content = in.readAll();
        //qDebug() << content;
        content.replace(QString("''"), QString("'" + d->messages.at(row)->id() + "'"));
        //qDebug() << content;
    }

    //在web页面启用插件
    webView->settings()->setAttribute(QWebSettings::PluginsEnabled, true);
    webView->setHtml(content);
    webView->show();

    //将近期播放的视频信息载入数据库
    QString str = "videoplay";
    if(database.connect(str))
    {
        database.insertData(d->messages.at(row));
    }
}

void MainWindow::connectButtonWithMapper(const QObjectList &objectList)
{
    for(int i = 0; i < objectList.length(); i++)
    {
        //判断对象列表里某一个对象的子列表是否为空
        //不为空就执行递归操作
        if(!objectList.at(i)->children().empty())
        {
            connectButtonWithMapper(objectList.at(i)->children());
        }
        else
        {
            QObject *object = objectList.at(i);
            //只对类名为QToolButton的变量进行操作
            if(object->inherits("QToolButton"))
            {
                QToolButton *button = qobject_cast<QToolButton *>(object);

                if(button->text() != "记录")
                {
                    //将原始信号传递给m_cacategoryMapper
                    connect(button, SIGNAL(clicked()),
                            d->m_categoryMapper, SLOT(map()));
                    //转换按钮的文本
                    QString str;

                    if(button->text() == "电视剧")
                        str = "TV";
                    if(button->text() == "电影")
                        str = "Movies";
                    if(button->text() == "动漫")
                        str = "Anime";
                    if(button->text() == "综艺")
                        str = "Variety";
                    if(button->text() == "音乐")
                        str = "Music";
                    if(button->text() == "教育")
                        str = "Education";
                    if(button->text() == "纪录片")
                        str = "Documentary";
                    if(button->text() == "体育")
                        str = "Sports";
                    if(button->text() == "汽车")
                        str = "Autos";
                    if(button->text() == "科技")
                        str = "Tech";
                    if(button->text() == "游戏")
                        str = "Games";
                    if(button->text() == "生活")
                        str = "LifeStyle";
                    if(button->text() == "时尚")
                        str = "Fashion";
                    if(button->text() == "亲子")
                        str = "Parenting";

                    //设置m_mapper的转发规则, 转发参数为QString类型的信号，
                    //并将按钮转换的文本内容作为实参传递
                    d->m_categoryMapper->setMapping(button, str);
                }//end if
            }//end if
        }//end else
    }//end for
}

void MainWindow::categoryButtonMapper(QString str)
{
    //qDebug()<<"str" << str;
    requestVideoInfo(str);
}

void MainWindow::handleSearchResult(QJsonObject *object)
{
    QJsonObject jsonObject = *object;

    saveVideoInfo(jsonObject);
}

void MainWindow::showPlayRecord()
{
    MysqlOperate database;
    QString str = "videoplay";
    QList<VideoMessage *> messages;

    if(database.connect(str))
    {
        database.readFromDB(messages);
        d->messages = messages;
        //qDebug() << d->messages.at(1)->thumbnail();
        for(int i = 0; i < messages.count(); i++)
        {
            setScreenShot(d->messages.at(i)->thumbnail(), i);
        }
    }
}
