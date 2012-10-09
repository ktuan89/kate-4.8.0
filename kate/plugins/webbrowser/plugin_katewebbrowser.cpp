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

#include "plugin_katewebbrowser.h"
#include "plugin_katewebbrowser.moc"

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

#include <QUrl>
#include <QStringList>
#include <QWebSettings>
#include <QSslConfiguration>
#include <QSslCertificate>

#include <algorithm>
#include <cmath>
using namespace std;

K_PLUGIN_FACTORY(KatePluginWebBrowserFactory, registerPlugin<KatePluginWebBrowser>();)
K_EXPORT_PLUGIN(KatePluginWebBrowserFactory(KAboutData("katewebbrowser","katewebbrowser",ki18n("Web Browser"), "0.1", ki18n("Plugin to load web pages"))) )

KatePluginWebBrowser::KatePluginWebBrowser( QObject* parent, const QList<QVariant>& )
    : Kate::Plugin( (Kate::Application*)parent, "kate-web-browser-plugin" )
{
}

KatePluginWebBrowser::~KatePluginWebBrowser()
{
}

Kate::PluginView *KatePluginWebBrowser::createView( Kate::MainWindow *mainWindow )
{
  return new KatePluginWebBrowserView( mainWindow );
}


KatePluginWebBrowserView::KatePluginWebBrowserView( Kate::MainWindow *mainWin )
    : Kate::PluginView( mainWin )
    , Kate::XMLGUIClient(KatePluginWebBrowserFactory::componentData())
    , m_mw(mainWin)
{
  m_toolview = mainWin->createToolView ("kate_plugin_webbrowser", Kate::MainWindow::Left, SmallIcon("utilities-terminal"), i18n("WebBrowser"));

  m_splitter = new QSplitter(m_toolview);
  m_splitter->setOrientation(Qt::Vertical);

  QWebSettings::globalSettings()->setAttribute(QWebSettings::DeveloperExtrasEnabled, true);
  // ktuan code
  QSslConfiguration sslCfg = QSslConfiguration::defaultConfiguration();
  QList<QSslCertificate> ca_list = sslCfg.caCertificates();
  QList<QSslCertificate> ca_new = QSslCertificate::fromData("CaCertificates");
  ca_list += ca_new;

  sslCfg.setCaCertificates(ca_list);
  sslCfg.setProtocol(QSsl::AnyProtocol);
  QSslConfiguration::setDefaultConfiguration(sslCfg);
  // end

  m_cookieJar = new QNetworkCookieJar();
  m_netManager = new QNetworkAccessManager();
  m_netManager->setCookieJar(m_cookieJar);

  connect(m_netManager, SIGNAL(sslErrors(QNetworkReply*, const QList<QSslError> & )),
          this, SLOT(sslErrorHandler(QNetworkReply*, const QList<QSslError> & )));

  mainWindow()->guiFactory()->addClient( this );
  connect(
    mainWin, SIGNAL(signalRunCommand(const QString &)),
    this, SLOT(runCommand(const QString &)));
}

KatePluginWebBrowserView::~KatePluginWebBrowserView()
{
  mainWindow()->guiFactory()->removeClient( this );
  foreach (QWebView *webview, m_webviews) {
    delete webview;
  }
  delete m_splitter;
  delete m_toolview;
}

void KatePluginWebBrowserView::runCommand(const QString &command)
{
  if (command.startsWith("WB add")) {
    QWebView *webview = new QWebView(m_splitter);
    m_webviews.append(webview);
    webview->page()->setNetworkAccessManager(m_netManager);
  } else if (command.startsWith("WB remove")) {
    if (m_webviews.count() > 0) {
      delete m_webviews.back();
      m_webviews.pop_back();
    }
  } else if (command.startsWith("WB set")) {
    QStringList list = command.split(QRegExp("\\s+"));
    bool ok;
    int id = list.at(2).toInt(&ok);
    m_webviews[id-1]->load(QUrl(list.at(3)));
  } else if (command.startsWith("WB refresh")) {
    bool ok;
    int id = command.mid(11).toInt(&ok);
    m_webviews[id-1]->triggerPageAction(QWebPage::Reload);
  }
}

void KatePluginWebBrowserView::sslErrorHandler(QNetworkReply* qnr, const QList<QSslError> & errlist) {
  qnr->ignoreSslErrors();
}

void KatePluginWebBrowserView::readSessionConfig( KConfigBase* config, const QString& groupPrefix )
{
  // If you have session-dependant settings, load them here.
  // If you have application wide settings, you have to read your own KConfig,
  // see the Kate::Plugin docs for more information.
  Q_UNUSED( config );
  Q_UNUSED( groupPrefix );
}

void KatePluginWebBrowserView::writeSessionConfig( KConfigBase* config, const QString& groupPrefix )
{
  // If you have session-dependant settings, save them here.
  // If you have application wide settings, you have to create your own KConfig,
  // see the Kate::Plugin docs for more information.
  Q_UNUSED( config );
  Q_UNUSED( groupPrefix );
}

// kate: space-indent on; indent-width 2; replace-tabs on;

