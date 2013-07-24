/* This file is part of the KDE project
   Copyright (C) 2001 Christoph Cullmann <cullmann@kde.org>
   Copyright (C) 2001 Joseph Wenninger <jowenn@kde.org>
   Copyright (C) 2001 Anders Lund <anders.lund@lund.tdcadsl.dk>

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

#ifndef KATE_VIEWSPACE_H
#define KATE_VIEWSPACE_H

#include "katemain.h"

#include <KTextEditor/View>
#include <KTextEditor/Document>
#include <KTextEditor/ModificationInterface>

#include <QWidget>
#include <QList>
#include <QPixmap>
#include <QLabel>
#include <QEvent>
#include <QResizeEvent>
#include <KStatusBar>
#include <KVBox>
#include <QCheckBox>
#include <QString>
#include <QWebView>

class KConfigBase;
class KSqueezedTextLabel;
class KateViewManager;
class KateViewSpace;
class QStackedWidget;

class KateVSStatusBar : public KStatusBar
{
    Q_OBJECT

  public:
    KateVSStatusBar ( KateViewSpace *parent = 0L);
    virtual ~KateVSStatusBar ();

    bool canSwitch() {
      return m_switchCheckbox->isChecked();
    }

    void updateIndex(int i) {
      m_switchCheckbox->setText(
        QString("%1  ").arg(i + 1)
      );
    }

    void updateAlphaBetaMoveMode(bool b) {
      if (b) m_alphabetaMode->setText("AB mode");
      else m_alphabetaMode->setText("");
    }

    /**
     * stuff to update the statusbar on view changes
     */
  public Q_SLOTS:
    void updateStatus ();

    void viewModeChanged ( KTextEditor::View *view );

    void cursorPositionChanged ( KTextEditor::View *view );

    void selectionChanged (KTextEditor::View *view);

    void modifiedChanged();

    void documentNameChanged ();

    void documentConfigChanged ();

    void informationMessage (KTextEditor::View *view, const QString &message);

    void cursorPositionItemVisibilityChanged(bool visible);
    void charactersCountItemVisibilityChanged(bool visible);
    void insertModeItemVisibilityChanged(bool visible);
    void selectModeItemVisibilityChanged(bool visible);
    void encodingItemVisibilityChanged(bool visible);
    void documentNameItemVisibilityChanged(bool visible);

  protected:
    virtual bool eventFilter (QObject*, QEvent *);
    virtual void showMenu ();

  private:
    QCheckBox* m_switchCheckbox;
    QLabel* m_lineColLabel;
    QLabel* m_charsLabel;
    QLabel* m_modifiedLabel;
    QLabel* m_insertModeLabel;
    QLabel* m_selectModeLabel;
    QLabel* m_encodingLabel;
    QLabel* m_alphabetaMode;
    KSqueezedTextLabel* m_fileNameLabel;
    QPixmap m_modPm, m_modDiscPm, m_modmodPm;
    class KateViewSpace *m_viewSpace;
};

class KateViewSpace : public QWidget
{
    friend class KateViewManager;
    friend class KateVSStatusBar;

    Q_OBJECT

  public:
    explicit KateViewSpace(KateViewManager *, QWidget* parent = 0, const char* name = 0);
    ~KateViewSpace();
    bool isActiveSpace();
    void setActive(bool b, bool showled = false);
    QStackedWidget* stack;
    KVBox *vbox;
    void addView(KTextEditor::View* v, bool show = true);
    void removeView(KTextEditor::View* v);

    bool showView(KTextEditor::View *view)
    {
      return showView(view->document());
    }
    bool showView(KTextEditor::Document *document);

    KTextEditor::View* currentView();
    int viewCount() const
    {
      return mViewList.count();
    }

    void saveConfig (KConfigBase* config, int myIndex, const QString& viewConfGrp);
    void restoreConfig ( KateViewManager *viewMan, const KConfigBase* config, const QString &group );

    bool canSwitch() {
      return mStatusBar->canSwitch();
    }

    void updateIndex(int i) {
      mStatusBar->updateIndex(i);
    }

public Q_SLOTS:
  void updateAlphaBetaMoveMode(bool b) {
    mStatusBar->updateAlphaBetaMoveMode(b);
  }

  protected:
    void resizeEvent(QResizeEvent *event);

  private Q_SLOTS:
    void statusBarToggled ();
    void addSearchDockObject();
    void slotRunCommand(const QString &command);
    void slotFocusSearchDock();

  private:
    QWebView *m_webview;
    QWidget *m_searchDock;
    bool mIsActiveSpace;
    KateVSStatusBar* mStatusBar;
    /// This list is necessary to only save the order of the accessed views.
    /// The order is important. The least recently viewed view is always the
    /// last entry in the list, i.e. mViewList.last()
    /// mViewList.count() == stack.count() is always true!
    QList<KTextEditor::View*> mViewList;
    KateViewManager *m_viewManager;
    QString m_group;
    int m_kt_debug;
};

class KateSearchDockJSObject : public QObject {
  Q_OBJECT

public:

  KateSearchDockJSObject(KateViewManager *viewManager);

  Q_INVOKABLE void runJSCommand(const QString &command);

private:
  KateViewManager *m_viewManager;
};

#endif
// kate: space-indent on; indent-width 2; replace-tabs on;
