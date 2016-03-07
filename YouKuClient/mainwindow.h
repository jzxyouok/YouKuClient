#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include <QToolButton>
#include <QListWidget>

class QNetworkReply;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    void setWindowWidget();
    void requestVideoInfo(QString category);
    void setScreenShot(const QString &url, int index);
    void reloadData();
    void connectButtonWithMapper(const QObjectList &objectList);
    void loadWebAndPlayVideo(int row);
    void saveVideoInfo(QJsonObject jsonObject);

private slots:
    void replyFinish(QNetworkReply *reply);
    void categoryButtonMapper(QString str);
    void currentRow(int row);
    void handleSearchResult(QJsonObject *object);
    void showPlayRecord();

private:
    class Private;
    friend class Private;
    Private *d;
};

#endif // MAINWINDOW_H
