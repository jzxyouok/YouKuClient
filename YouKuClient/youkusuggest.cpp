#include "youkusuggest.h"
#include "network.h"
#include "remoterequest.h"

#include <QEvent>
#include <QTimer>
#include <QTreeWidget>
#include <QHeaderView>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QPalette>
#include <QColor>

#define HITS_COUNT 4
#define SUGGEST_URL "https://openapi.youku.com/v2/searches/keyword/complete.json?client_id=a8556d66d167afd0&keyword=%1"

class YouKuSuggest::Private
{
public:
    Private()
    {
        network = Network::instance();
    }

    void fetchKeyword(const QString &url)
    {
        QNetworkReply *reply = network->get(url);
        replyMap.insert(reply, FetchKeyWord);
    }

    Network *network;
    QTreeWidget *m_popup;              //弹出树
    QTimer *m_timer;                   //计时器(定时器)
    QMap<QNetworkReply *, RemoteRequest> replyMap;
};

YouKuSuggest::YouKuSuggest(QLineEdit *parent) :
    QObject(parent), m_editor(parent),
    d(new YouKuSuggest::Private)
{
    d->m_popup = new QTreeWidget;

    //设置控件的窗口标志
    d->m_popup->setWindowFlags(Qt::Popup);
    //设置控件不接受键盘焦点
    d->m_popup->setFocusPolicy(Qt::NoFocus);
    //设置控件的窗口代理窗口为parent
    d->m_popup->setFocusProxy(parent);
    //设置控件启用鼠标跟踪，无按键被按下，控件任接受鼠标移动事件
    d->m_popup->setMouseTracking(true);

    //设置栏数
    d->m_popup->setColumnCount(1);
    //设置树形视图中(控件)的所有项有相同的高度
    d->m_popup->setUniformRowHeights(true);
    //去掉控件的虚线边框(到时候验证一下)
    d->m_popup->setRootIsDecorated(false);
    //设置控件为不可编辑
    d->m_popup->setEditTriggers(QTreeWidget::NoEditTriggers);
    //设置控件的选择行为为行选择
    d->m_popup->setSelectionBehavior(QTreeWidget::SelectRows);
    //设置控件的框架类型,内容用框包围，且框和内容看起来和周围一样高
    d->m_popup->setFrameStyle(QFrame::Box | QFrame::Plain);
    //去掉控件的水平滚动条
    d->m_popup->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    //隐藏控件的头
    d->m_popup->header()->hide();

    //安装一个事件过滤器
    d->m_popup->installEventFilter(this);

    //当弹出控件里的项被点击时，执行槽函数
    connect(d->m_popup, SIGNAL(itemClicked(QTreeWidgetItem*,int)),
            SLOT(doneCompletion()));

    //计时器
    d->m_timer = new QTimer(this);
    //设置计时器为单发计时器，只触发一次
    d->m_timer->setSingleShot(true);
    //设置计时器超时的时间间隔(毫秒)
    d->m_timer->setInterval(100);

    //当计时器超时时，自动提出建议
    connect(d->m_timer, SIGNAL(timeout()), SLOT(autoSuggest()));
    //当文本编辑时，开始计时
    connect(m_editor, SIGNAL(textEdited(QString)), d->m_timer, SLOT(start()));
    //获取关键词联想的网络请求结束，处理返回的数据
    connect(d->network, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(handleReplyData(QNetworkReply*)));
}

YouKuSuggest::~YouKuSuggest()
{
    delete d;
    d = 0;
}

//函数返回false，则过滤该事件，若使用该事件则返回true
bool YouKuSuggest::eventFilter(QObject *object, QEvent *event)
{
    if(object != d->m_popup)
        return false;

    //鼠标按下事件
    if(event->type() == QEvent::MouseButtonPress)
    {
        d->m_popup->hide();
        m_editor->setFocus();
        return true;
    }

    //键盘按下事件
    if(event->type() == QEvent::KeyPress)
    {
        bool consumed = false;
        int key = static_cast<QKeyEvent*>(event)->key();

        switch(key)
        {
        //响应回车事件
        //Qt中的回车键对应的是两个键值：Qt::Key_Enter和Qt::Key_Return
        case Qt::Key_Enter:
        case Qt::Key_Return:
            doneCompletion();
            consumed = true;

        //响应Esc事件
        case Qt::Key_Escape:
            m_editor->setFocus();
            d->m_popup->hide();
            consumed = true;

        case Qt::Key_Up:
        case Qt::Key_Down:
        case Qt::Key_Home:
        case Qt::Key_End:
        case Qt::Key_PageUp:
        case Qt::Key_PageDown:
            break;

        default:
            m_editor->setFocus();
            m_editor->event(event);
            d->m_popup->hide();
            break;
        }

        return consumed;
    }

    return false;
}

void YouKuSuggest::doneCompletion()
{
    d->m_timer->stop();
    d->m_popup->hide();
    m_editor->setFocus();
    QTreeWidgetItem *item = d->m_popup->currentItem();
    if(item)
    {
        //设置编辑器的文本为项第0列的文本
        m_editor->setText(item->text(0));
        QMetaObject::invokeMethod(m_editor, "returnPressed");
    }
}

//自动提示
void YouKuSuggest::autoSuggest()
{
    QString str = m_editor->text();
    QString url = QString(SUGGEST_URL).arg(str);
    //发送获取关键词联想的请求
    d->fetchKeyword(url);
}

void YouKuSuggest::preventSuggest()
{
    d->m_timer->stop();
}

//处理返回的数据(json文件)
void YouKuSuggest::handleReplyData(QNetworkReply *reply)
{
    RemoteRequest request = d->replyMap.value(reply);
    qDebug() << "requestsuggest " << request;
    switch(request)
    {
    case FetchKeyWord:
    {
        if(reply->error() != QNetworkReply::NoError)
            return;

        QJsonParseError error;
        QJsonDocument jsonDocument = QJsonDocument::fromJson(reply->readAll(), &error);
        if(!(jsonDocument.isNull() || jsonDocument.isEmpty()) &&
                jsonDocument.isObject())
        {
            QStringList hits;
            //qDebug() << "jsonDocumentsuggest    " << jsonDocument <<endl<<endl<<endl;
            //获取最外层的对象
            QJsonObject jsonObject = jsonDocument.object();
            if(jsonObject.contains("r") && jsonObject.value("r").isArray())
            {
                QJsonArray jsonArray = jsonObject["r"].toArray();
                //在弹出控件最多显示四个项
                int count = jsonArray.count();
                int hitCount;
                if(count < HITS_COUNT)
                    hitCount = count;
                else
                    hitCount = HITS_COUNT;

                for(int i = 0; i < hitCount; i++)
                {
                    QJsonObject object = jsonArray.at(i).toObject();
                    QVariantMap data = object.toVariantMap();
                    QString str;
                    str = data[QLatin1String("c")].toString();
                    hits.append(str);
                }
            }

            showCompletion(hits);
        }
        break;
    }
    }

    reply->deleteLater();
}

//设置并展示弹出控件的项
void YouKuSuggest::showCompletion(const QStringList &hits)
{
    if(hits.isEmpty())
        return;

    const QPalette &pal = m_editor->palette();
    //将调色板的ColorGroup枚举类型设置为不可用的状态
    //将调色板的ColorRole枚举类型设置为通用的前景色
    QColor color = pal.color(QPalette::Disabled, QPalette::WindowText);

    //禁止控件更新
    d->m_popup->setUpdatesEnabled(false);
    d->m_popup->clear();
    //添加控件的项
    for(int i = 0; i < hits.count(); i++)
    {
        QTreeWidgetItem *item = new QTreeWidgetItem(d->m_popup);
        item->setText(0, hits[i]);
        item->setTextAlignment(0, Qt::AlignLeft);
        item->setTextColor(0, color);
    }
    //设置控件的当前项
    //d->m_popup->setCurrentItem(d->m_popup->topLevelItem(0));
    d->m_popup->resizeColumnToContents(0);
    //设置字体大小
    d->m_popup->setStyleSheet("QTreeWidget{font-size: 18px}");
    d->m_popup->adjustSize();
    d->m_popup->setUpdatesEnabled(true);

    int h = d->m_popup->sizeHintForRow(0) * qMin(7, hits.count()) + 3;
    //设置控件的大小
    d->m_popup->resize(m_editor->width(), h);

    d->m_popup->move(m_editor->mapToGlobal(QPoint(0, m_editor->height())));
    d->m_popup->setFocus();
    d->m_popup->show();
}
