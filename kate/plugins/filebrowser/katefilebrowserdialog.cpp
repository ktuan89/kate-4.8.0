#include "katefilebrowserdialog.h"

#include "katefilebrowser.h"

#include <kactioncollection.h>
#include <kaction.h>
#include <kdebug.h>
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
#include <KDirOperator>
#include <QRegExp>



KateFileBrowserDialog::KateFileBrowserDialog(QWidget *parent, KateFileBrowser *browser):
    KDialog(parent), m_browser(browser) {
    setModal(true);

    setButtons(KDialog::Cancel);
    setButtonGuiItem( KDialog::User1 , KGuiItem("Switch to") );
    showButtonSeparator(true);
    setCaption(m_browser->dirOperator()->url().prettyUrl());

    QWidget * mainwidget = new QWidget(this);
    setMainWidget(mainwidget);

    QVBoxLayout *layout=new QVBoxLayout(mainwidget);
    layout->setSpacing(spacingHint());

    m_inputLine = new KLineEdit(mainwidget);
    m_inputLine->setFocus(Qt::OtherFocusReason);
    layout->addWidget(m_inputLine);

    m_fileslist = new QListWidget(mainwidget);
    layout->addWidget(m_fileslist);

    m_inputLine->installEventFilter(this);
    m_fileslist->installEventFilter(this);

    QDesktopWidget *desktop=new QDesktopWidget();
    setMinimumWidth(desktop->screenGeometry(parent).width()/2);
    delete desktop;
}

QString lcp(QString a, QString b) {
  int len = 0;
  while (len < a.length() && len < b.length() && a[len] == b[len]) ++len;
  return a.left(len);
}

void KateFileBrowserDialog::cdTo(QString relative_path) {
  KUrl newurl = m_browser->dirOperator()->url();
  newurl.cd(relative_path);
  m_browser->setDir(newurl);
  setCaption(newurl.prettyUrl());
}

bool KateFileBrowserDialog::eventFilter(QObject *obj, QEvent *event) {
    if (event->type()==QEvent::KeyPress) {
        QKeyEvent *keyEvent=static_cast<QKeyEvent*>(event);
        if (keyEvent->key() == Qt::Key_Tab) {
          bool none = true;
          QString res = "", last_name = "";
          QString cur = m_inputLine->text();
          m_fileslist->clear();
          foreach (const KFileItem& item, m_browser->dirOperator()->dirLister()->items()) {
            QString name = item.name();
            if (name.startsWith(cur)) {
              m_fileslist->addItem(new QListWidgetItem(name));
              if (item.isDir()) last_name = name;
              if (none) { res = name; none = false; }
              else res = lcp(res, name);
            }
          }
          if (res == "") {
            foreach (const KFileItem& item, m_browser->dirOperator()->dirLister()->items()) {
              QString name = item.name();
              if (name.toLower().startsWith(cur.toLower())) {
                m_fileslist->addItem(new QListWidgetItem(name));
                if (item.isDir()) last_name = name;
                if (none) { res = name; none = false; }
                else res = lcp(res, name);
              }
            }
          }
          if (res != "" && res != cur) {
            m_inputLine->setText(res);
          }
          if (m_fileslist->count() == 1 && last_name != "") {
            cdTo(last_name);
            m_inputLine->setText("");
          }
          return true;
        } else if (keyEvent->key() == Qt::Key_QuoteLeft) {
          cdTo("..");
          return true;
        } else if (keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Apostrophe) {
          QString cur = m_inputLine->text();
          if (cur.startsWith("/") || cur.startsWith("~")) {
            m_browser->setDir(cur);
            m_inputLine->setText("");
            return true;
          }
          foreach (const KFileItem& item, m_browser->dirOperator()->dirLister()->items()) {
            QString name = item.name();
            if (name == cur) {
              if (item.isDir()) {
                cdTo(name);
                m_inputLine->setText("");
              } else if (item.isFile()) {
                m_browser->mainWindow()->openUrl(item.url());
                if (keyEvent->key() == Qt::Key_Return) {
                  accept();
                  return true;
                }
                m_inputLine->setText("");
              }
            }
          }
          return true;
        } else if (keyEvent->key() == Qt::Key_BracketLeft) {
          QString cur = m_inputLine->text();
          if (cur != "") {
            static const QRegExp file_name_reg("([a-z]|[0-9]|[A-Z]|_|-)+\\.[a-z]+$");
            if (file_name_reg.exactMatch(cur)) {
              m_browser->mainWindow()->openUrl(
                KUrl(m_browser->dirOperator()->url().prettyUrl(KUrl::AddTrailingSlash) +
                  cur)
                                       );
              accept();
              return true;
            } else {
              m_browser->dirOperator()->mkdir(cur);
            }
          }
          m_inputLine->setText("");
          return true;
        }
    }
    return KDialog::eventFilter(obj,event);
}

KateFileBrowserDialog::~KateFileBrowserDialog() {
}
