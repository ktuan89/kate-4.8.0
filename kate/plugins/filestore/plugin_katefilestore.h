#ifndef _PLUGIN_KATE_FILESTORE_H_
#define _PLUGIN_KATE_FILESTORE_H_

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

#include <QWidget>
#include <QList>
#include <QString>
#include <QMap>

class KatePluginFileStore : public Kate::Plugin
{
  Q_OBJECT

  public:
    explicit KatePluginFileStore( QObject* parent = 0, const QList<QVariant>& = QList<QVariant>() );
    virtual ~KatePluginFileStore();

    Kate::PluginView *createView( Kate::MainWindow *mainWindow );

  public Q_SLOTS:
    void saveDocument(KTextEditor::Document *doc);
    void slotNameChanged(KTextEditor::Document *doc);

  private:
    QMap<QString, int> urlToLastSavingTime;
};

class KatePluginFileStoreView : public Kate::PluginView
{
    Q_OBJECT

  public:
    KatePluginFileStoreView( Kate::MainWindow *mainWindow );
    ~KatePluginFileStoreView();

    virtual void readSessionConfig( KConfigBase* config, const QString& groupPrefix );
    virtual void writeSessionConfig( KConfigBase* config, const QString& groupPrefix );

  private:
    Kate::MainWindow *m_mw;
    QWidget *m_toolview;

signals:
  void updateAlphaBetaMoveMode(bool b);
};

class KatePluginFileStoreWorker : public QObject {
  Q_OBJECT

  public:
    KatePluginFileStoreWorker(QString filename);

  public Q_SLOTS:
    void doWork();

  Q_SIGNALS:
    void finished();

  private:
    QString m_filename;
};

#endif

// kate: space-indent on; indent-width 2; replace-tabs on;

