/* This file is part of the KDE project
   Copyright (C) 2001 Christoph Cullmann <cullmann@kde.org>
   Copyright (C) 2002 Joseph Wenninger <jowenn@kde.org>
   Copyright (C) 2002 Anders Lund <anders.lund@lund.tdcadsl.dk>
   Copyright (C) 2007 Anders Lund <anders@alweb.dk>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "kateconsole.h"
#include "kateconsole.moc"

#include <kicon.h>
#include <kiconloader.h>
#include <ktexteditor/document.h>
#include <ktexteditor/view.h>

#include <kde_terminal_interface.h>
#include <kshell.h>
#include <kparts/part.h>
#include <kaction.h>
#include <kactioncollection.h>
#include <KDialog>

#include <kurl.h>
#include <klibloader.h>
#include <klocale.h>
#include <kdebug.h>
#include <kmessagebox.h>
//Added by qt3to4:
#include <QShowEvent>
#include <QLabel>

#include <QCheckBox>
#include <QVBoxLayout>

#include <kpluginloader.h>
#include <kaboutdata.h>
#include <kpluginfactory.h>
#include <kauthorized.h>

#include <QDesktopWidget>
#include <QDesktopServices>
#include <qcoreapplication.h>

K_PLUGIN_FACTORY(KateKonsoleFactory, registerPlugin<KateKonsolePlugin>();)
K_EXPORT_PLUGIN(KateKonsoleFactory(KAboutData("katekonsole","katekonsoleplugin",ki18n("Konsole"), "0.1", ki18n("Embedded Konsole"), KAboutData::License_LGPL_V2)) )

KateKonsolePlugin::KateKonsolePlugin( QObject* parent, const QList<QVariant>& ):
    Kate::Plugin ( (Kate::Application*)parent )
{
  m_previousEditorEnv=qgetenv("EDITOR");
  if (!KAuthorized::authorizeKAction("shell_access"))
  {
    KMessageBox::sorry(0, i18n ("You do not have enough karma to access a shell or terminal emulation"));
  }
}

KateKonsolePlugin::~KateKonsolePlugin()
{
  ::setenv( "EDITOR", m_previousEditorEnv.data(), 1 );
}

Kate::PluginView *KateKonsolePlugin::createView (Kate::MainWindow *mainWindow)
{
  KateKonsolePluginView *view = new KateKonsolePluginView (this, mainWindow);
  return view;
}

Kate::PluginConfigPage *KateKonsolePlugin::configPage (uint number, QWidget *parent, const char *name)
{
  Q_UNUSED(name)
  if (number != 0)
    return 0;
  return new KateKonsoleConfigPage(parent, this);
}

QString KateKonsolePlugin::configPageName (uint number) const
{
  if (number != 0) return QString();
  return i18n("Terminal");
}

QString KateKonsolePlugin::configPageFullName (uint number) const
{
  if (number != 0) return QString();
  return i18n("Terminal Settings");
}

KIcon KateKonsolePlugin::configPageIcon (uint number) const
{
  if (number != 0) return KIcon();
  return KIcon("utilities-terminal");
}

void KateKonsolePlugin::readConfig()
{
  foreach ( KateKonsolePluginView *view, mViews )
    view->readConfig();
}

KateKonsolePluginView::KateKonsolePluginView (KateKonsolePlugin* plugin, Kate::MainWindow *mainWindow)
    : Kate::PluginView (mainWindow),m_plugin(plugin)
{
  // init console
  QWidget *toolview = mainWindow->createToolView ("kate_private_plugin_katekonsoleplugin", Kate::MainWindow::Bottom, SmallIcon("utilities-terminal"), i18n("Terminal"));
  m_console = new KateConsole(m_plugin, mainWindow, toolview);

  // register this view
  m_plugin->mViews.append ( this );
}

KateKonsolePluginView::~KateKonsolePluginView ()
{
  // unregister this view
  m_plugin->mViews.removeAll (this);

  // cleanup, kill toolview + console
  QWidget *toolview = m_console->parentWidget();
  delete m_console;
  delete toolview;
}

void KateKonsolePluginView::readConfig()
{
  m_console->readConfig();
}

KateConsole::KateConsole (KateKonsolePlugin* plugin, Kate::MainWindow *mw, QWidget *parent)
    : KVBox (parent), Kate::XMLGUIClient(KateKonsoleFactory::componentData())
    , m_part (0)
    , m_mw (mw)
    , m_toolView (parent)
    , m_plugin(plugin)
    , m_kt_open_immediately(0)
    , m_kt_comm_temp("tbgs ###")
{
  QAction* a = actionCollection()->addAction("katekonsole_tools_pipe_to_terminal");
  a->setIcon(KIcon("utilities-terminal"));
  a->setText(i18nc("@action", "&Pipe to Terminal"));
  connect(a, SIGNAL(triggered()), this, SLOT(slotPipeToConsole()));

  a = actionCollection()->addAction("katekonsole_tools_sync");
  a->setText(i18nc("@action", "S&ynchronize Terminal with Current Document"));
  connect(a, SIGNAL(triggered()), this, SLOT(slotManualSync()));

  a = actionCollection()->addAction("katekonsole_tools_toggle_focus");
  a->setIcon(KIcon("utilities-terminal"));
  a->setText(i18nc("@action", "&Focus Terminal"));
  connect(a, SIGNAL(triggered()), this, SLOT(slotToggleFocus()));

  a = actionCollection()->addAction("katekonsole_tools_arc_build");
  a->setText(i18nc("@action", "Arc Build"));
  connect(a, SIGNAL(triggered()), this, SLOT(slotArcBuild()));

  a = actionCollection()->addAction("katekonsole_tools_gcc_compile");
  a->setText(i18nc("@action", "G++ compile"));
  connect(a, SIGNAL(triggered()), this, SLOT(slotGccCompile()));

  a = actionCollection()->addAction("katekonsole_tools_run_with_input");
  a->setText(i18nc("@action", "Run with input"));
  connect(a, SIGNAL(triggered()), this, SLOT(slotRunWithInput()));

  a = actionCollection()->addAction("katekonsole_pick_file");
  a->setText(i18nc("@action", "Pick file"));
  connect(a, SIGNAL(triggered()), this, SLOT(slotPickFile()));

  connect(
    mainWindow(), SIGNAL(signalRunCommand(const QString &)),
    this, SLOT(runCommand(const QString &)));

  m_mw->guiFactory()->addClient (this);

  readConfig();
}

KateConsole::~KateConsole ()
{
  m_mw->guiFactory()->removeClient (this);
  if (m_part)
    disconnect ( m_part, SIGNAL(destroyed()), this, SLOT(slotDestroyed()) );
}

void KateConsole::loadConsoleIfNeeded()
{
  if (m_part) return;

  if (!window() || !parentWidget()) return;
  if (!window() || !isVisibleTo(window())) return;

  KPluginFactory *factory = KPluginLoader("libkonsolepart").factory();

  if (!factory) return;

  m_part = static_cast<KParts::ReadOnlyPart *>(factory->create<QObject>(this, this));

  if (!m_part) return;

  // start the terminal
  qobject_cast<TerminalInterface*>(m_part)->showShellInDir( QString() );

  KGlobal::locale()->insertCatalog("konsole");

  setFocusProxy(m_part->widget());
  m_part->widget()->show();

  connect ( m_part, SIGNAL(destroyed()), this, SLOT(slotDestroyed()) );
  connect ( m_part, SIGNAL(receiveLine(const QString&)), this, SLOT(receiveLine(const QString&)));

  slotSync();
}

void KateConsole::slotDestroyed ()
{
  m_part = 0;

  // hide the dockwidget
  if (parentWidget())
  {
    m_mw->hideToolView (m_toolView);
    m_mw->centralWidget()->setFocus ();
  }
}

void KateConsole::showEvent(QShowEvent *)
{
  if (m_part) return;

  loadConsoleIfNeeded();
}

void KateConsole::cd (const KUrl &url)
{
  QString path = url.path();
  if (path.startsWith("/Users/anhk/www/")) {
    path = "~/www/" + path.mid(16);
    sendInput("cd " + path + '\n');
  } else {
    sendInput("cd " + KShell::quoteArg(path) + '\n');
  }
}

void KateConsole::sendInput( const QString& text )
{
  loadConsoleIfNeeded();

  if (!m_part) return;

  TerminalInterface *t = qobject_cast<TerminalInterface *>(m_part);

  if (!t) return;

  t->sendInput (text);
}

void KateConsole::runCommand(const QString & command) {
  if (command.startsWith("OP ")) {
    QString filepath = command.mid(3);
    mainWindow()->openUrl(KUrl(filepath));
  } else if (command.startsWith("OPEN ")) {
    m_kt_open_immediately = 3;
    sendInput(command.mid(5));
  } else if (command.startsWith("VIEW ")) {
    QString url = command.mid(5);
    QDesktopServices::openUrl(url);
  } else if (command.startsWith("SC ")) {
    m_kt_comm_temp = command.mid(3);
  } else if (!command.startsWith("WB") && !command.startsWith("RP")) {
    sendInput(command);
  }
}

void KateConsole::receiveLine(const QString & line) {
  // ktuan
  // int db_area = KDebug::registerArea("ktuan-debug");
  // kDebug(db_area) << "\'" << line << "\'";
  if (line.startsWith("www") || line.startsWith("fbcode") ||
      line.startsWith("dataswarm") || line.startsWith("source")) {
    if (m_kt_open_immediately > 0) {
      m_kt_open_immediately--;
      QString filename = line;
      int i = filename.indexOf(":");
      if (i != -1) {
        filename = filename.mid(0, i);
      }
      mainWindow()->openUrl( KUrl("/Users/anhk/" + filename));
    }
    emit signalReceiveLine(line);
    m_searchList.prepend(line);
    while (m_searchList.count() > 200) m_searchList.removeLast();
  } else if (line.startsWith("KTENDEND")) {
    m_kt_open_immediately = 0;
  } else {
    if (line.startsWith("#\tmodified:   ") ||
        line.startsWith("#\tnew file:   ")
    ) {
      emit signalReceiveLine(line.mid(14));
    }
  }
}

void KateConsole::slotPickFile() {
  QString filename = PluginKonsoleDialog::document(mainWindow()->window(), m_searchList, this);
  if (filename.startsWith("www") || filename.startsWith("fbcode") ||
      filename.startsWith("dataswarm") || filename.startsWith("source")) {
    int i = filename.indexOf(":");
    if (i != -1) {
      filename = filename.mid(0, i);
    }
    mainWindow()->openUrl( KUrl("/Users/anhk/" + filename));
  } else {
    filename = filename.trimmed();
    QString filetoopen = filename;
    if (filename.startsWith("flib") ||
        filename.startsWith("html") ||
        filename.startsWith("lib") ||
        filename.startsWith("thrift")) {
      filetoopen = "/Users/anhk/www/" + filename;
      mainWindow()->openUrl(KUrl(filetoopen));
    }
  }
}

void KateConsole::slotArcBuild()
{
  sendInput("arc build\n");
}

void KateConsole::slotGccCompile() {
  QString name = m_mw->activeView()->document()->documentName();
  if (name.endsWith(".cpp") || name.endsWith(".java")) {
    m_lastfilename = name;
  } else {
    name = m_lastfilename;
  }
  if (name.endsWith(".cpp")) sendInput("g++ " + name + "\n");
  else if (name.endsWith(".java")) sendInput("javac " + name + "\n");
}

void KateConsole::slotRunWithInput() {
  QString name = m_mw->activeView()->document()->documentName();
  if (name.endsWith(".cpp") || name.endsWith(".java")) {
    m_lastfilename = name;
  } else {
    name = m_lastfilename;
  }
  if (name.endsWith(".java")) {
    QString classname = name.left(name.length() - 5);
    sendInput("java " + classname + " <in.txt\n");
  } else sendInput("./a.out <in.txt\n");
}

void KateConsole::slotPipeToConsole ()
{
  if (KMessageBox::warningContinueCancel
      (m_mw->window()
       , i18n ("Do you really want to pipe the text to the console? This will execute any contained commands with your user rights.")
       , i18n ("Pipe to Terminal?")
       , KGuiItem(i18n("Pipe to Terminal")), KStandardGuiItem::cancel(), "Pipe To Terminal Warning") != KMessageBox::Continue)
    return;

  KTextEditor::View *v = m_mw->activeView();

  if (!v)
    return;

  if (v->selection())
    sendInput (v->selectionText());
  else
    sendInput (v->document()->text());
}

void KateConsole::slotSync()
{
  if (m_mw->activeView() ) {
    if ( m_mw->activeView()->document()->url().isValid()
      && m_mw->activeView()->document()->url().isLocalFile() ) {
      cd(KUrl( m_mw->activeView()->document()->url().directory() ));
    } else if ( !m_mw->activeView()->document()->url().isEmpty() ) {
      sendInput( "### " + i18n("Sorry, can not cd into '%1'", m_mw->activeView()->document()->url().directory() ) + '\n' );
    }
  }
}

void KateConsole::slotManualSync()
{
  slotSync();
  if ( ! m_part || ! m_part->widget()->isVisible() )
    m_mw->showToolView( parentWidget() );
}
void KateConsole::slotToggleFocus()
{
  QAction *action = actionCollection()->action("katekonsole_tools_toggle_focus");
  if ( ! m_part ) {
    m_mw->showToolView( parentWidget() );
    action->setText( i18n("Defocus Terminal") );
    return; // this shows and focuses the konsole
  }

  if ( ! m_part ) return;

  if (m_part->widget()->hasFocus()) {
    if (m_mw->activeView())
      m_mw->activeView()->setFocus();
      action->setText( i18n("Focus Terminal") );
  } else {
    // show the view if it is hidden
    if (parentWidget()->isHidden())
      m_mw->showToolView( parentWidget() );
    else // should focus the widget too!
      m_part->widget()->setFocus( Qt::OtherFocusReason );
    action->setText( i18n("Defocus Terminal") );
  }
}

void KateConsole::readConfig()
{
  disconnect( m_mw, SIGNAL(viewChanged()), this, SLOT(slotSync()) );
  if ( KConfigGroup(KGlobal::config(), "Konsole").readEntry("AutoSyncronize", false) )
    connect( m_mw, SIGNAL(viewChanged()), SLOT(slotSync()) );


  if ( KConfigGroup(KGlobal::config(), "Konsole").readEntry("SetEditor", false) )
    ::setenv( "EDITOR", "kate -b",1);
  else
    ::setenv( "EDITOR", m_plugin->previousEditorEnv().data(), 1 );
}

QString PluginKonsoleDialog::document(QWidget *parent, const QLinkedList<QString> & rlist, KateConsole *console) {
    PluginKonsoleDialog dlg(parent, rlist, console);
    if (QDialog::Accepted==dlg.exec()) {
      if (dlg.m_fileslist->currentItem()) {
        return dlg.m_fileslist->currentItem()->text();
      }
    }
    return "";
}

PluginKonsoleDialog::PluginKonsoleDialog(QWidget *parent, const QLinkedList<QString> & rlist, KateConsole *console):
    KDialog(parent),
    m_console(console) {
    setModal(true);

    setButtons( KDialog::Ok | KDialog::Cancel);
    setButtonGuiItem( KDialog::User1 , KGuiItem("Switch to") );
    showButtonSeparator(true);
    setCaption(i18n("Document Fast Open"));

    QWidget * mainwidget = new QWidget(this);
    setMainWidget(mainwidget);

    QVBoxLayout *layout=new QVBoxLayout(mainwidget);
    layout->setSpacing(spacingHint());

    m_inputLine = new KLineEdit(mainwidget);
    m_inputLine->setFocus(Qt::OtherFocusReason);
    layout->addWidget(m_inputLine);

    m_fileslist = new QListWidget(mainwidget);
    layout->addWidget(m_fileslist);

    foreach (QString result, rlist) {
      QListWidgetItem *item = new QListWidgetItem(result);
      m_fileslist->addItem(item);
    }

    if (m_fileslist->count() > 0) {
      m_fileslist->setCurrentRow(0);
    }

    m_inputLine->installEventFilter(this);
    m_fileslist->installEventFilter(this);

    connect(console, SIGNAL(signalReceiveLine(const QString & )),
            this, SLOT(receiveLine(const QString & )));

    QDesktopWidget *desktop=new QDesktopWidget();
    setMinimumWidth(desktop->screenGeometry(parent).width()/2);
    delete desktop;
}

void PluginKonsoleDialog::receiveLine(const QString & line) {
  m_fileslist->addItem(new QListWidgetItem(line));
}

bool PluginKonsoleDialog::eventFilter(QObject *obj, QEvent *event) {
    if (event->type()==QEvent::KeyPress) {
        QKeyEvent *keyEvent=static_cast<QKeyEvent*>(event);
        if (keyEvent->key() == Qt::Key_K && keyEvent->modifiers() == Qt::META) {
          QCoreApplication::sendEvent(m_fileslist, new QKeyEvent(QEvent::KeyPress, Qt::Key_Down, Qt::NoModifier));
          return true;
        }
        if (keyEvent->key() == Qt::Key_I && keyEvent->modifiers() == Qt::META) {
          QCoreApplication::sendEvent(m_fileslist, new QKeyEvent(QEvent::KeyPress, Qt::Key_Up, Qt::NoModifier));
          return true;
        }
        if (obj==m_inputLine) {
            if ( (keyEvent->key()==Qt::Key_Up) || (keyEvent->key()==Qt::Key_Down) || (keyEvent->key()==Qt::Key_PageUp) || (keyEvent->key()==Qt::Key_PageDown)) {
                QCoreApplication::sendEvent(m_fileslist,event);
                return true;
            }
            if (keyEvent->key() == Qt::Key_Tab) {
              m_fileslist->clear();
              if (m_inputLine->text() == "") {
                m_console->sendInput("cat ~/www/.git/COMMIT_EDITMSG\n");
              } else {
                QString s = m_console->m_kt_comm_temp;
                s.replace("###", m_inputLine->text());
                m_console->sendInput(s + "\n");
              }
              return true;
            }
        } else {
            if ( (keyEvent->key()!=Qt::Key_Up) && (keyEvent->key()!=Qt::Key_Down) && (keyEvent->key()!=Qt::Key_Tab) && (keyEvent->key()!=Qt::Key_Backtab) && (keyEvent->key()!=Qt::Key_PageUp) && (keyEvent->key()!=Qt::Key_PageDown)) {
                QCoreApplication::sendEvent(m_inputLine,event);
                return true;
            }
        }
    }
    return KDialog::eventFilter(obj,event);
}

PluginKonsoleDialog::~PluginKonsoleDialog() {
  m_fileslist->clear();
  // delete m_fileslist;
}

KateKonsoleConfigPage::KateKonsoleConfigPage( QWidget* parent, KateKonsolePlugin *plugin )
  : Kate::PluginConfigPage( parent )
  , mPlugin( plugin )
{
  QVBoxLayout *lo = new QVBoxLayout( this );
  lo->setSpacing( KDialog::spacingHint() );

  cbAutoSyncronize = new QCheckBox( i18n("&Automatically synchronize the terminal with the current document when possible"), this );
  lo->addWidget( cbAutoSyncronize );
  cbSetEditor = new QCheckBox( i18n("Set &EDITOR environment variable to 'kate -b'"), this );
  lo->addWidget( cbSetEditor );
  QLabel *tmp = new QLabel(this);
  tmp->setText(i18n("Important: The document has to be closed to make the console application continue"));
  lo->addWidget(tmp);
  reset();
  lo->addStretch();
  connect( cbAutoSyncronize, SIGNAL(stateChanged(int)), SIGNAL(changed()) );
  connect( cbSetEditor, SIGNAL(stateChanged(int)), SIGNAL(changed()) );
}

void KateKonsoleConfigPage::apply()
{
  KConfigGroup config(KGlobal::config(), "Konsole");
  config.writeEntry("AutoSyncronize", cbAutoSyncronize->isChecked());
  config.writeEntry("SetEditor", cbSetEditor->isChecked());
  config.sync();
  mPlugin->readConfig();
}

void KateKonsoleConfigPage::reset()
{
  KConfigGroup config(KGlobal::config(), "Konsole");
  cbAutoSyncronize->setChecked(config.readEntry("AutoSyncronize", false));
  cbSetEditor->setChecked(config.readEntry("SetEditor", false));
}

// kate: space-indent on; indent-width 2; replace-tabs on;

