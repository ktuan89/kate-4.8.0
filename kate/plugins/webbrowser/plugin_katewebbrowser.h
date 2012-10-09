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

#ifndef _PLUGIN_KATE_WEBBROWSER_H_
#define _PLUGIN_KATE_WEBBROWSER_H_

#include <kate/mainwindow.h>
#include <kate/plugin.h>
#include <kxmlguiclient.h>

#include <QWebView>
#include <QSplitter>
#include <QList>
#include <QNetworkCookieJar>
#include <QNetworkAccessManager>
#include <QString>
#include <QNetworkReply>
#include <QSslError>

#include <map>
using namespace std;

class KatePluginWebBrowser : public Kate::Plugin
{
  Q_OBJECT

  public:
    explicit KatePluginWebBrowser( QObject* parent = 0, const QList<QVariant>& = QList<QVariant>() );
    virtual ~KatePluginWebBrowser();

    Kate::PluginView *createView( Kate::MainWindow *mainWindow );
};

class KatePluginWebBrowserView : public Kate::PluginView, public Kate::XMLGUIClient
{
    Q_OBJECT

  public:
    KatePluginWebBrowserView( Kate::MainWindow *mainWindow );
    ~KatePluginWebBrowserView();

    virtual void readSessionConfig( KConfigBase* config, const QString& groupPrefix );
    virtual void writeSessionConfig( KConfigBase* config, const QString& groupPrefix );

  public slots:
    void runCommand(const QString &command);
    void sslErrorHandler(QNetworkReply* qnr, const QList<QSslError> & errlist);

  private:
    Kate::MainWindow *m_mw;
    QWidget *m_toolview;
    QList<QWebView*> m_webviews;
    QSplitter *m_splitter;
    QNetworkCookieJar *m_cookieJar;
    QNetworkAccessManager *m_netManager;
};

#endif

// kate: space-indent on; indent-width 2; replace-tabs on;

