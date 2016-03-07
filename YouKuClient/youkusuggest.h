#ifndef YOUKUSUGGEST_H
#define YOUKUSUGGEST_H

#include <QtWidgets>
#include <QNetworkReply>
#include <QLineEdit>
#include <QStringList>

class YouKuSuggest : public QObject
{
    Q_OBJECT
public:
    YouKuSuggest(QLineEdit *parent = 0);
    ~YouKuSuggest();

    bool eventFilter(QObject *object, QEvent *event);
    void showCompletion(const QStringList &hits);

public slots:
    void doneCompletion();
    void preventSuggest();
    void autoSuggest();
    void handleReplyData(QNetworkReply *reply);

private:
    class Private;
    friend class Private;
    Private *d;

    QLineEdit *m_editor;               //搜索编辑器
};

#endif // YOUKUSUGGEST_H
