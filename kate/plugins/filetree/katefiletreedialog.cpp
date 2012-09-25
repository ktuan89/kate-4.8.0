#include "katefiletreedialog.h"


#include <klineedit.h>
#include <kactioncollection.h>
#include <kaction.h>
#include <qtreeview.h>
#include <qwidget.h>
#include <qboxlayout.h>
#include <qstandarditemmodel.h>
#include <qpointer.h>
#include <qevent.h>
#include <qlabel.h>
#include <qcoreapplication.h>
#include <QDesktopWidget>
#include <QString>
#include <QApplication>
#include <QClipboard>


KateFileTreeDialog::KateFileTreeDialog(QWidget *parent):
    KDialog(parent) {
    setModal(true);

    setButtons(KDialog::Cancel);
    setButtonGuiItem( KDialog::User1 , KGuiItem("Switch to") );
    showButtonSeparator(true);
    setCaption(i18n("File tree controller"));

    m_inputLine=new KLineEdit(this);
    m_inputLine->installEventFilter(this);

    setMainWidget(m_inputLine);
    m_inputLine->setFocus(Qt::OtherFocusReason);

    QDesktopWidget *desktop=new QDesktopWidget();
    setMinimumWidth(desktop->screenGeometry(parent).width()/2);
    delete desktop;
}

PluginViewKateFastOpenDialog::~PluginViewKateFastOpenDialog() {
}
