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

#include "plugin_katerecursivepart.h"
#include "plugin_katerecursivepart.moc"

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
#include <kservice.h>

#include <QUrl>
#include <QStringList>
#include <QTimer>

#include <algorithm>
#include <cmath>

using namespace std;

K_PLUGIN_FACTORY(KatePluginRecursivePartFactory, registerPlugin<KatePluginRecursivePart>();)
K_EXPORT_PLUGIN(KatePluginRecursivePartFactory(KAboutData("katerecursivepart","katerecursivepart",ki18n("Recursive Part"), "0.1", ki18n("Kate in Kate"))) )

KatePluginRecursivePart::KatePluginRecursivePart( QObject* parent, const QList<QVariant>& )
    : Kate::Plugin( (Kate::Application*)parent, "kate-recursive-part-plugin" )
{
}

KatePluginRecursivePart::~KatePluginRecursivePart()
{
}

Kate::PluginView *KatePluginRecursivePart::createView( Kate::MainWindow *mainWindow )
{
  return new KatePluginRecursivePartView( mainWindow );
}


KatePluginRecursivePartView::KatePluginRecursivePartView( Kate::MainWindow *mainWin )
    : Kate::PluginView( mainWin )
    , Kate::XMLGUIClient(KatePluginRecursivePartFactory::componentData())
    , m_mw(mainWin)
{
  m_toolview = mainWin->createToolView ("kate_plugin_recursivepart", Kate::MainWindow::Left, SmallIcon("utilities-terminal"), i18n("RecursivePart"));

  KPluginFactory *factory = KPluginLoader("katepart").factory();
  // KService::Ptr service = KService::serviceByDesktopPath("katepart.desktop");

  if (!factory) return;

  m_part = static_cast<KTextEditor::Document *>(factory->create<QObject>(m_toolview, m_toolview));

  if (!m_part) return;

  mainWindow()->guiFactory()->addClient(m_part);
  m_toolview->setFocusProxy(m_part->widget());
  m_part->widget()->show();

  KTextEditor::View *view = m_part->activeView();
  connect(view, SIGNAL(focusIn(KTextEditor::View*)), this, SLOT(slotGetFocus(KTextEditor::View*)));
  connect(view, SIGNAL(focusOut(KTextEditor::View*)), this, SLOT(slotLostFocus(KTextEditor::View*)));

  connect(this, SIGNAL(updateAlphaBetaMoveMode(bool)), view, SLOT(updateAlphaBetaMoveMode(bool)));
  emit updateAlphaBetaMoveMode(true);

  connect(
    mainWin, SIGNAL(signalRunCommand(const QString &)),
    this, SLOT(runCommand(const QString &))
  );

  QTimer *timer = new QTimer(this);
  connect(timer, SIGNAL(timeout()), this, SLOT(slotTimeout()));
  timer->setSingleShot(false);
  timer->setInterval(4222);
  timer->start();
}

KatePluginRecursivePartView::~KatePluginRecursivePartView()
{
  mainWindow()->guiFactory()->removeClient(m_part);
  if (m_part) delete m_part;
  delete m_toolview;
}

void KatePluginRecursivePartView::runCommand(const QString &command) {
  if (command.startsWith("RP set ")) {
    m_part->activeView()->document()->setText(command.mid(7));
  }
  else if (command.startsWith("RP get")) {
    QString content = m_part->activeView()->document()->text();
    content.replace("\n", "~");
    mainWindow()->runJSCommand("recursivePartGetText \'" + content + "\'");
  }
}

void KatePluginRecursivePartView::slotTimeout() {
  mainWindow()->runJSCommand("recursivePartTimeout");
}

void KatePluginRecursivePartView::slotGetFocus(KTextEditor::View *view) {
  // int db_area = KDebug::registerArea("ktuan-debug");
  // kDebug(db_area) << "get focus";
  mainWindow()->guiFactory()->removeClient(mainWindow()->activeView());
  mainWindow()->guiFactory()->addClient(m_part);
}

void KatePluginRecursivePartView::slotLostFocus(KTextEditor::View *view) {
  // int db_area = KDebug::registerArea("ktuan-debug");
  // kDebug(db_area) << "lost focus";
  mainWindow()->guiFactory()->removeClient(m_part);
  mainWindow()->guiFactory()->addClient(mainWindow()->activeView());
}

void KatePluginRecursivePartView::readSessionConfig( KConfigBase* config, const QString& groupPrefix )
{
  // If you have session-dependant settings, load them here.
  // If you have application wide settings, you have to read your own KConfig,
  // see the Kate::Plugin docs for more information.
  Q_UNUSED( config );
  Q_UNUSED( groupPrefix );
}

void KatePluginRecursivePartView::writeSessionConfig( KConfigBase* config, const QString& groupPrefix )
{
  // If you have session-dependant settings, save them here.
  // If you have application wide settings, you have to create your own KConfig,
  // see the Kate::Plugin docs for more information.
  Q_UNUSED( config );
  Q_UNUSED( groupPrefix );
}

// kate: space-indent on; indent-width 2; replace-tabs on;

