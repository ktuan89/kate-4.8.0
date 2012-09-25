#ifndef _KateFileTreeDialog_h_
#define _KateFileTreeDialog_h_


#include <kdialog.h>

#include <QString>

class QListView;
class QTreeView;
class KLineEdit;


class KateFileTreeDialog: public KDialog {
    Q_OBJECT
    public:
        KateFileTreeDialog(QWidget *parent);
        virtual ~KateFileTreeDialog();
    private:
        KLineEdit *m_inputLine;
};

#endif // _KateFileTreeDialog_h_
