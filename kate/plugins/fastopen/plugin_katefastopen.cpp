/*
    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
    ---
    Copyright (C) 2007,2009 Joseph Wenninger <jowenn@kde.org>
*/

#include "plugin_katefastopen.h"
#include "plugin_katefastopen.moc"

#include <ktexteditor/document.h>
#include <ktexteditor/view.h>

#include <kpluginfactory.h>
#include <kpluginloader.h>
#include <kaboutdata.h>

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

Q_DECLARE_METATYPE(QPointer<KTextEditor::Document>)

const int DocumentRole=Qt::UserRole+1;
const int SortFilterRole=Qt::UserRole+2;

K_PLUGIN_FACTORY(KateFastOpenFactory, registerPlugin<PluginKateFastOpen>();)
K_EXPORT_PLUGIN(KateFastOpenFactory(KAboutData("katefastopen","katefastopenplugin",ki18n("Fast Open"), "0.1", ki18n("Fast Open documents"), KAboutData::License_LGPL_V2)) )

//BEGIN: Plugin

PluginKateFastOpen::PluginKateFastOpen( QObject* parent , const QList<QVariant>&):
    Kate::Plugin ( (Kate::Application *)parent, "kate-fast-open-plugin" ) {
}

PluginKateFastOpen::~PluginKateFastOpen() {
}

Kate::PluginView *PluginKateFastOpen::createView (Kate::MainWindow *mainWindow) {
    return new PluginViewKateFastOpen(mainWindow);
}

//END: Plugin

//BEGIN: View
PluginViewKateFastOpen::PluginViewKateFastOpen(Kate::MainWindow *mainwindow):
    Kate::PluginView(mainwindow),
    Kate::XMLGUIClient(KateFastOpenFactory::componentData()) {

    KAction *a = actionCollection()->addAction("documents_fastopen");
    a->setText(i18n("Fast Open"));
    a->setShortcut(QKeySequence(Qt::META+Qt::Key_U) );
    connect( a, SIGNAL(triggered(bool)), this, SLOT(slotFastOpen()) );

    a = actionCollection()->addAction("documents_fastopen_buffer");
    a->setText(i18n("Fast Open Buffer"));
    a->setShortcut(QKeySequence(Qt::META+Qt::Key_Y) );
    connect( a, SIGNAL(triggered(bool)), this, SLOT(slotFastOpenBuffer()) );

    mainwindow->guiFactory()->addClient (this);

}

PluginViewKateFastOpen::~PluginViewKateFastOpen() {
  mainWindow()->guiFactory()->removeClient (this);
}

void PluginViewKateFastOpen::slotFastOpen() {
  QString filename = PluginViewKateFastOpenDialog::document(mainWindow()->window());
  open(filename);
}

void PluginViewKateFastOpen::slotFastOpenBuffer() {
  QString filename = QApplication::clipboard()->text( QClipboard::Clipboard );
  open(filename);
}

void PluginViewKateFastOpen::open(QString filename) {
  QString filetoopen = filename;
  if (filename != "") {
    if (filename.startsWith("www")) {
      filetoopen = "/Users/anhk/" + filename;
    }
    if (filename.startsWith("flib") ||
        filename.startsWith("html") ||
        filename.startsWith("lib") ||
        filename.startsWith("thrift")) {
      filetoopen = "/Users/anhk/www/" + filename;
    }
    mainWindow()->openUrl(KUrl(filetoopen));
  }
}
//END: View

//BEGIN: Dialog
QString PluginViewKateFastOpenDialog::document(QWidget *parent) {
    PluginViewKateFastOpenDialog dlg(parent);
    if (QDialog::Accepted==dlg.exec()) {
      return dlg.m_inputLine->text();
    }
    return "";
}

PluginViewKateFastOpenDialog::PluginViewKateFastOpenDialog(QWidget *parent):
    KDialog(parent) {
    setModal(true);

    setButtons( KDialog::Ok | KDialog::Cancel);
    setButtonGuiItem( KDialog::User1 , KGuiItem("Switch to") );
    showButtonSeparator(true);
    setCaption(i18n("Document Fast Open"));

    m_inputLine=new KLineEdit(this);

    setMainWidget(m_inputLine);
    m_inputLine->setFocus(Qt::OtherFocusReason);

    QDesktopWidget *desktop=new QDesktopWidget();
    setMinimumWidth(desktop->screenGeometry(parent).width()/2);
    delete desktop;
}

PluginViewKateFastOpenDialog::~PluginViewKateFastOpenDialog() {
}


//END: Dialog

