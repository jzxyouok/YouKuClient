#ifndef SEARCHBOX_H
#define SEARCHBOX_H

#include <QLineEdit>
#include "youkusuggest.h"
#include "network.h"

class SearchBox : public QLineEdit
{
    Q_OBJECT

public:
    SearchBox(QWidget *parent = 0);
    ~SearchBox();

signals:
    void showSearchResult(QJsonObject *object);

protected slots:
    void doSearch();

private:
    class Private;
    friend class Private;
    Private *d;
};

#endif // SEARCHBOX_H
