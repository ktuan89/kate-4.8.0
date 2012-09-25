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
#ifndef _KateFastOpen_h_
#define _KateFastOpen_h_

#include <kate/plugin.h>
#include <kate/application.h>
#include <kate/documentmanager.h>
#include <kate/mainwindow.h>

#include <kdialog.h>

#include <qsortfilterproxymodel.h>
#include <QString>

class QListView;
class QTreeView;
class KLineEdit;

namespace KTextEditor {
    class Document;
}

class PluginKateFastOpen : public Kate::Plugin {
  Q_OBJECT
  public:
    explicit PluginKateFastOpen( QObject* parent = 0, const QList<QVariant>& = QList<QVariant>());
    virtual ~PluginKateFastOpen();
    virtual Kate::PluginView *createView (Kate::MainWindow *mainWindow);
};

class PluginViewKateFastOpen: public Kate::PluginView, public Kate::XMLGUIClient {
  Q_OBJECT
  public:
    PluginViewKateFastOpen(Kate::MainWindow *mainwindow);
    virtual ~PluginViewKateFastOpen();
  private Q_SLOTS:
    void slotFastOpen();
    void slotFastOpenBuffer();
  private:
    void open(QString filename);
};


class PluginViewKateFastOpenDialog: public KDialog {
    Q_OBJECT
    public:
        PluginViewKateFastOpenDialog(QWidget *parent);
        virtual ~PluginViewKateFastOpenDialog();
        static QString document(QWidget *parent);
    private:
        KLineEdit *m_inputLine;
};


#endif // _KateFastOpen_h_

