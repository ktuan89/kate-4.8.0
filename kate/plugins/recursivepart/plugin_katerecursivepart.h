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

#ifndef _PLUGIN_KATE_RECURSIVEPART_H_
#define _PLUGIN_KATE_RECURSIVEPART_H_

#include <kate/mainwindow.h>
#include <kate/plugin.h>
#include <kxmlguiclient.h>

#include <ktexteditor/view.h>
#include <ktexteditor/document.h>

#include <kparts/part.h>
#include <kpluginloader.h>
#include <kaboutdata.h>
#include <kpluginfactory.h>
#include <klibloader.h>

#include <QWebView>
#include <QSplitter>
#include <QList>
#include <QNetworkCookieJar>
#include <QNetworkAccessManager>
#include <QString>
#include <QNetworkReply>
#include <QSslError>
#include <QTimer>

#include <map>
using namespace std;

class KatePluginRecursivePart : public Kate::Plugin
{
  Q_OBJECT

  public:
    explicit KatePluginRecursivePart( QObject* parent = 0, const QList<QVariant>& = QList<QVariant>() );
    virtual ~KatePluginRecursivePart();

    Kate::PluginView *createView( Kate::MainWindow *mainWindow );
};

class KatePluginRecursivePartView : public Kate::PluginView, public Kate::XMLGUIClient
{
    Q_OBJECT

  public:
    KatePluginRecursivePartView( Kate::MainWindow *mainWindow );
    ~KatePluginRecursivePartView();

    virtual void readSessionConfig( KConfigBase* config, const QString& groupPrefix );
    virtual void writeSessionConfig( KConfigBase* config, const QString& groupPrefix );

  private:
    Kate::MainWindow *m_mw;
    QWidget *m_toolview;
    KTextEditor::Document *m_part;
    QTimer *m_timer;

public slots:
  void slotGetFocus(KTextEditor::View*);
  void slotLostFocus(KTextEditor::View*);
  void runCommand(const QString &);
  void slotTimeout();
signals:
  void updateAlphaBetaMoveMode(bool b);
};

#endif

// kate: space-indent on; indent-width 2; replace-tabs on;

