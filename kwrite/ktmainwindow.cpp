#include <ktmainwindow.h>
#include <QString>
#include <QtGui/QWidget>

KTMainWindow :: KTMainWindow() : KMainWindow(0) {
    resize(800, 600);
    setStyleSheet(QString::fromUtf8("background:rgba(0,0,0,100);"));
    QWidget * centralwidget = new QWidget(this);
    centralwidget->setObjectName(QString::fromUtf8("centralwidget"));
    setCentralWidget(centralwidget);
}
