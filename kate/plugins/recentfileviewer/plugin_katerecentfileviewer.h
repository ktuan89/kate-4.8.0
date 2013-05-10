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

#ifndef _PLUGIN_KATE_RECENTFILEVIEWER_H_
#define _PLUGIN_KATE_RECENTFILEVIEWER_H_

#include <kate/mainwindow.h>
#include <kate/plugin.h>
#include <kxmlguiclient.h>
#include <KUrl>

#include <QListWidget>
#include <QListWidgetItem>
#include <QString>

class KatePluginRecentFileViewer : public Kate::Plugin
{
  Q_OBJECT

  public:
    explicit KatePluginRecentFileViewer( QObject* parent = 0, const QList<QVariant>& = QList<QVariant>() );
    virtual ~KatePluginRecentFileViewer();

    Kate::PluginView *createView( Kate::MainWindow *mainWindow );
};

class KatePluginRecentFileViewerView : public Kate::PluginView, public Kate::XMLGUIClient
{
    Q_OBJECT

  public:
    KatePluginRecentFileViewerView( Kate::MainWindow *mainWindow );
    ~KatePluginRecentFileViewerView();

    virtual void readSessionConfig( KConfigBase* config, const QString& groupPrefix );
    virtual void writeSessionConfig( KConfigBase* config, const QString& groupPrefix );

  public slots:
    void slotShow1();
    void slotShow2();
    void slotShow3();
    void slotShow4();
    void slotShow5();
    void slotShow6();
    void slotShow7();
    void slotShow8();
    void slotShow9();
    void slotSelectItem(QListWidgetItem * item);
    void slotViewChanged();
    void slotDocumentDeleted(KTextEditor::Document *doc);

  private:
    void readConfig();
    void writeConfig();
    void showRecentDoc(int i);
    void deleteDoc(KTextEditor::Document *doc);

    Kate::MainWindow *m_mw;
    QList<KTextEditor::Document*> m_docList;
    QWidget *m_toolview;
    QListWidget *m_fileslist;

    bool m_viewChangedRunning;
};

class RecentFileViewerListItem : public QListWidgetItem {
  public:
    RecentFileViewerListItem(QString str, KTextEditor::Document *doc);
    RecentFileViewerListItem(QString str, QString url, QString doc_name);
    KTextEditor::Document *getKtDoc();
    KUrl getDocUrl();
    QString getDocUrlStr();
    QString getDocName();

  private:
    KTextEditor::Document *kt_doc;
    QString m_url, m_doc_name;
};


#endif

// kate: space-indent on; indent-width 2; replace-tabs on;

