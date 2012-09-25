/*  This file is part of the Kate project.
 *
 *  Copyright (C) 2010 Christoph Cullmann <cullmann@kde.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 */

#include "plugin_katerecentfileviewer.h"
#include "plugin_katerecentfileviewer.moc"

#include <kate/documentmanager.h>
#include <kate/application.h>
#include <ktexteditor/view.h>
#include <ktexteditor/document.h>

#include <kaction.h>
#include <kactioncollection.h>
#include <klocale.h>
#include <kpluginfactory.h>
#include <kpluginloader.h>
#include <kaboutdata.h>
#include <kurl.h>
#include <KDebug>

#include <QListWidget>

#include <algorithm>
#include <cmath>
using namespace std;

K_PLUGIN_FACTORY(KatePluginRecentFileViewerFactory, registerPlugin<KatePluginRecentFileViewer>();)
K_EXPORT_PLUGIN(KatePluginRecentFileViewerFactory(KAboutData("katerecentfileviewer","katerecentfileviewer",ki18n("Recent File Viewer"), "0.1", ki18n("Plugin to show recent file"))) )

KatePluginRecentFileViewer::KatePluginRecentFileViewer( QObject* parent, const QList<QVariant>& )
    : Kate::Plugin( (Kate::Application*)parent, "kate-recent-file-viewer-plugin" )
{
}

KatePluginRecentFileViewer::~KatePluginRecentFileViewer()
{
}

Kate::PluginView *KatePluginRecentFileViewer::createView( Kate::MainWindow *mainWindow )
{
  return new KatePluginRecentFileViewerView( mainWindow );
}


KatePluginRecentFileViewerView::KatePluginRecentFileViewerView( Kate::MainWindow *mainWin )
    : Kate::PluginView( mainWin )
    , Kate::XMLGUIClient(KatePluginRecentFileViewerFactory::componentData())
    , m_mw(mainWin)
{
    m_viewChangedRunning = false;

  connect(Kate::application()->documentManager(), SIGNAL(documentDeleted(KTextEditor::Document*)),
            this, SLOT(slotDocumentDeleted(KTextEditor::Document*)));

  connect( mainWin, SIGNAL(viewChanged()), SLOT(slotViewChanged()) );

  KAction *a = actionCollection()->addAction( "recentfileviewer_show_1" );
  a->setText( i18n("Show Recent 1") );
  a->setShortcut(Qt::CTRL + Qt::Key_1);
  connect( a, SIGNAL(triggered(bool)), this, SLOT(slotShow1()) );
  a = actionCollection()->addAction( "recentfileviewer_show_2" );
  a->setText( i18n("Show Recent 2") );
  a->setShortcut(Qt::CTRL + Qt::Key_2);
  connect( a, SIGNAL(triggered(bool)), this, SLOT(slotShow2()) );
  a = actionCollection()->addAction( "recentfileviewer_show_3" );
  a->setText( i18n("Show Recent 3") );
  a->setShortcut(Qt::CTRL + Qt::Key_3);
  connect( a, SIGNAL(triggered(bool)), this, SLOT(slotShow3()) );
  a = actionCollection()->addAction( "recentfileviewer_show_4" );
  a->setText( i18n("Show Recent 4") );
  a->setShortcut(Qt::CTRL + Qt::Key_4);
  connect( a, SIGNAL(triggered(bool)), this, SLOT(slotShow4()) );
  a = actionCollection()->addAction( "recentfileviewer_show_5" );
  a->setText( i18n("Show Recent 5") );
  a->setShortcut(Qt::CTRL + Qt::Key_5);
  connect( a, SIGNAL(triggered(bool)), this, SLOT(slotShow5()) );

  a = actionCollection()->addAction( "recentfileviewer_show_6" );
  a->setText( i18n("Show Recent 6") );
  a->setShortcut(Qt::CTRL + Qt::Key_6);
  connect( a, SIGNAL(triggered(bool)), this, SLOT(slotShow6()) );
  a = actionCollection()->addAction( "recentfileviewer_show_7" );
  a->setText( i18n("Show Recent 7") );
  a->setShortcut(Qt::CTRL + Qt::Key_7);
  connect( a, SIGNAL(triggered(bool)), this, SLOT(slotShow7()) );
  a = actionCollection()->addAction( "recentfileviewer_show_8" );
  a->setText( i18n("Show Recent 8") );
  a->setShortcut(Qt::CTRL + Qt::Key_8);
  connect( a, SIGNAL(triggered(bool)), this, SLOT(slotShow8()) );
  a = actionCollection()->addAction( "recentfileviewer_show_9" );
  a->setText( i18n("Show Recent 9") );
  a->setShortcut(Qt::CTRL + Qt::Key_9);
  connect( a, SIGNAL(triggered(bool)), this, SLOT(slotShow9()) );

  m_toolview = mainWin->createToolView ("kate_plugin_recentfileviewer", Kate::MainWindow::Left, SmallIcon("utilities-terminal"), i18n("RecentFiles"));
  m_fileslist = new QListWidget(m_toolview);

  connect(m_fileslist, SIGNAL(itemClicked(QListWidgetItem *)),
          this,        SLOT(slotSelectItem(QListWidgetItem *)));

  mainWindow()->guiFactory()->addClient( this );
}

KatePluginRecentFileViewerView::~KatePluginRecentFileViewerView()
{
  mainWindow()->guiFactory()->removeClient( this );
  delete m_fileslist;
  delete m_toolview;
}

void KatePluginRecentFileViewerView::slotViewChanged() {
  KTextEditor::View* view = m_mw->activeView();
  if (!view) return;

  if (m_viewChangedRunning) return;

  m_viewChangedRunning = true;

  m_docList.removeAll(view->document());
  m_docList.append(view->document());
  if (m_docList.count() > 10) {
    m_docList.removeFirst();
  }

  m_fileslist->clear();

  for (int i = 0; i < 10 && i < m_docList.count(); ++i) {
    KTextEditor::Document *docdoc = m_docList.at(m_docList.count() - 1 - i);
    QString p_name = QString("%1 %2").arg(i, -4).arg(docdoc->documentName());
    RecentFileViewerListItem *item =
      new RecentFileViewerListItem(p_name, docdoc);
    m_fileslist->addItem(item);
  }

  m_viewChangedRunning = false;
}

void KatePluginRecentFileViewerView::slotDocumentDeleted(KTextEditor::Document *doc) {
  if (m_viewChangedRunning) return;

  m_viewChangedRunning = true;
  m_docList.removeAll(doc);

  for (int i = 0; i < m_fileslist->count(); ++i) {
    RecentFileViewerListItem *item = (RecentFileViewerListItem*)m_fileslist->item(i);
    if (item->getKtDoc() == doc) {
      m_fileslist->takeItem(i);
      delete item;
      --i;
    }
  }

  m_viewChangedRunning = false;
}

void KatePluginRecentFileViewerView::slotShow1() {
  showRecentDoc(1);
}
void KatePluginRecentFileViewerView::slotShow2() {
  showRecentDoc(2);
}
void KatePluginRecentFileViewerView::slotShow3() {
  showRecentDoc(3);
}
void KatePluginRecentFileViewerView::slotShow4() {
  showRecentDoc(4);
}
void KatePluginRecentFileViewerView::slotShow5() {
  showRecentDoc(5);
}

void KatePluginRecentFileViewerView::slotShow6() {
  showRecentDoc(6);
}

void KatePluginRecentFileViewerView::slotShow7() {
  showRecentDoc(7);
}

void KatePluginRecentFileViewerView::slotShow8() {
  showRecentDoc(8);
}

void KatePluginRecentFileViewerView::slotShow9() {
  showRecentDoc(9);
}

void KatePluginRecentFileViewerView::slotSelectItem(QListWidgetItem * item) {
  if (m_viewChangedRunning) return;
  m_viewChangedRunning = true;
  RecentFileViewerListItem *ii = (RecentFileViewerListItem*)item;
  if (ii) {
    mainWindow()->activateView(ii->getKtDoc());
  }
  m_viewChangedRunning = false;
}

void KatePluginRecentFileViewerView::showRecentDoc(int i)
{
  if (!mainWindow()) {
    return;
  }

  RecentFileViewerListItem *item = (RecentFileViewerListItem*)m_fileslist->item(i);
  if (item) {
    mainWindow()->activateView(item->getKtDoc());
  }
}

void KatePluginRecentFileViewerView::readSessionConfig( KConfigBase* config, const QString& groupPrefix )
{
  // If you have session-dependant settings, load them here.
  // If you have application wide settings, you have to read your own KConfig,
  // see the Kate::Plugin docs for more information.
  Q_UNUSED( config );
  Q_UNUSED( groupPrefix );
}

void KatePluginRecentFileViewerView::writeSessionConfig( KConfigBase* config, const QString& groupPrefix )
{
  // If you have session-dependant settings, save them here.
  // If you have application wide settings, you have to create your own KConfig,
  // see the Kate::Plugin docs for more information.
  Q_UNUSED( config );
  Q_UNUSED( groupPrefix );
}

// kate: space-indent on; indent-width 2; replace-tabs on;

