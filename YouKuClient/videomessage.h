#ifndef VIDEOMESSAGE_H
#define VIDEOMESSAGE_H

#include <QString>

class VideoMessage
{
public:
    VideoMessage();
    ~VideoMessage();

    QString id() const;
    void setId(const QString &id);

    QString title() const;
    void setTitle(const QString &title);

    QString thumbnail() const;
    void setThumbnail(const QString &thumbnail);

    QString link() const;
    void setLink(const QString &link);

    QString category() const;
    void setCategory(const QString &category);

private:
    class Private;
    friend class Private;
    Private *d;
};

#endif // VIDEOMESSAGE_H
