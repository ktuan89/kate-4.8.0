#ifndef _KateFileBrowserDialog_h_
#define _KateFileBrowserDialog_h_


#include <kdialog.h>
#include "katefilebrowser.h"

#include <klineedit.h>

#include <QString>
#include <QListWidget>

class QListView;
class QTreeView;

class KateFileBrowserDialog: public KDialog {
    Q_OBJECT
    public:
        KateFileBrowserDialog(QWidget *parent, KateFileBrowser *browser);
        ~KateFileBrowserDialog();
        void cdTo(QString relative_path);
    protected:
        bool eventFilter(QObject *obj, QEvent *event);
    public:
        KLineEdit *m_inputLine;
        KateFileBrowser *m_browser;
        QListWidget *m_fileslist;
};

#endif // _KateFileBrowserDialog_h_
