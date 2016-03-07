#include "videomessage.h"

class VideoMessage::Private
{
public:
    Private()
    {

    }

    QString m_id;
    QString m_title;
    QString m_thumbnail;
    QString m_link;
    QString m_category;
};

VideoMessage::VideoMessage() :
    d(new VideoMessage::Private)
{

}


VideoMessage::~VideoMessage()
{
    delete d;
    d = 0;
}

//视频唯一ID的设置与返回
QString VideoMessage::id() const
{
    return d->m_id;
}

void VideoMessage::setId(const QString &id)
{
    d->m_id = id;
}

//视频标题的设置与返回
QString VideoMessage::title() const
{
    return d->m_title;
}

void VideoMessage::setTitle(const QString &title)
{
    d->m_title = title;
}

//视频截图的链接的设置与返回
QString VideoMessage::thumbnail() const
{
    return d->m_thumbnail;
}

void VideoMessage::setThumbnail(const QString &thumbnail)
{
    d->m_thumbnail = thumbnail;
}

//视频播放链接的设置与返回
QString VideoMessage::link() const
{
    return d->m_link;
}
void VideoMessage::setLink(const QString &link)
{
    d->m_link = link;
}

//视频分类的设置与返回
QString VideoMessage::category() const
{
    return d->m_category;
}

void VideoMessage::setCategory(const QString &category)
{
    d->m_category = category;
}
