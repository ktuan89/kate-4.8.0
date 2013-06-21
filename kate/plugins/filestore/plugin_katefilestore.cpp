#include "plugin_katefilestore.h"
#include "plugin_katefilestore.moc"

#include <kate/documentmanager.h>
#include <kate/application.h>
#include <ktexteditor/cursor.h>
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
#include <kservice.h>
#include <KConfig>
#include <KConfigGroup>

#include <QtAlgorithms>
#include <QList>
#include <QUrl>
#include <QStringList>
#include <QThread>
#include <QPair>
#include <QDir>
#include <QFile>
#include <QFileInfo>

#include <time.h>


K_PLUGIN_FACTORY(KatePluginFileStoreFactory, registerPlugin<KatePluginFileStore>();)
K_EXPORT_PLUGIN(KatePluginFileStoreFactory(KAboutData("katefilestore","katefilestore",ki18n("File Store"), "0.1", ki18n("Automatically store files"))) )

KatePluginFileStore::KatePluginFileStore( QObject* parent, const QList<QVariant>& )
    : Kate::Plugin( (Kate::Application*)parent, "kate-file-store-plugin" )
{

  KConfig *config = new KConfig("katefilestorerc", KConfig::SimpleConfig);
  KConfigGroup *cg = new KConfigGroup(config, "kfs");
  for (int i = 0; i < 200; ++i) {
    QString url = cg->readEntry(QString("fileurl:%1").arg(i), "");
    int time = cg->readEntry(QString("savetime:%1").arg(i), 0);
    if (url != "") {
      urlToLastSavingTime[url] = time;
    }
  }

  connect(Kate::documentManager(), SIGNAL(documentCreated(KTextEditor::Document *)),
          this, SLOT(saveDocument(KTextEditor::Document *)));
}

KatePluginFileStore::~KatePluginFileStore()
{
  QList<QPair<int, QString> > list;
  for (QMap<QString, int>::const_iterator i = urlToLastSavingTime.constBegin();
       i != urlToLastSavingTime.constEnd();
       ++i) {
    list.append(qMakePair(i.value(), i.key()));
  }
  qSort(list);

  KConfig *config = new KConfig("katefilestorerc", KConfig::SimpleConfig);
  KConfigGroup *cg = new KConfigGroup(config, "kfs");
  for (int i = list.count() - 1, j = 0; i >= 0 && i >= list.count() - 200; --i, ++j) {
    cg->writeEntry(QString("fileurl:%1").arg(j), list[i].second);
    cg->writeEntry(QString("savetime:%1").arg(j), list[i].first);
  }
  config->sync();
}

void KatePluginFileStore::slotNameChanged(KTextEditor::Document *doc) {
  KUrl url = KUrl(doc->url());
  QString filepath = url.prettyUrl();
  if (filepath != "") {
    int cur_time = time(0);
    if (filepath.startsWith("file:///Users/anhk/www/") &&
        (!urlToLastSavingTime.contains(filepath) || urlToLastSavingTime[filepath] + 86400 < cur_time)) {
      urlToLastSavingTime[filepath] = cur_time;
      QThread *thread = new QThread(this);
      KatePluginFileStoreWorker *worker = new KatePluginFileStoreWorker(filepath);
      connect(thread, SIGNAL(started()), worker, SLOT(doWork()));
      connect(worker, SIGNAL(finished()), thread, SLOT(quit()));
      connect(worker, SIGNAL(finished()), worker, SLOT(deleteLater()));
      connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
      worker->moveToThread(thread);
      thread->start();
    }
  }
}

void KatePluginFileStore::saveDocument(KTextEditor::Document *doc) {
  connect(doc, SIGNAL(documentNameChanged(KTextEditor::Document*)),
          this, SLOT(slotNameChanged(KTextEditor::Document*)));
}

KatePluginFileStoreWorker::KatePluginFileStoreWorker(QString filename): m_filename(filename) {}

void KatePluginFileStoreWorker::doWork() {
  m_filename = m_filename.mid(QString("file://").length());
  QString new_file = "/Users/anhk/www_local/" +
    m_filename.mid(QString("/Users/anhk/www/").length());

  QFileInfo file_info(new_file);
  QString path_ = file_info.path();
  QDir dir;
  dir.mkpath(path_);

  QFile backupFile(new_file);
  if (backupFile.exists()) backupFile.remove();

  QFile::copy(m_filename, new_file);

  emit finished();
}

Kate::PluginView *KatePluginFileStore::createView( Kate::MainWindow *mainWindow )
{
  return new KatePluginFileStoreView( mainWindow );
}


KatePluginFileStoreView::KatePluginFileStoreView( Kate::MainWindow *mainWin )
    : Kate::PluginView( mainWin )
    , m_mw(mainWin)
{
  m_toolview = mainWin->createToolView ("kate_plugin_filestore", Kate::MainWindow::Left, SmallIcon("utilities-terminal"), i18n("FileStore"));
}

KatePluginFileStoreView::~KatePluginFileStoreView()
{
  delete m_toolview;
}

void KatePluginFileStoreView::readSessionConfig( KConfigBase* config, const QString& groupPrefix )
{
  Q_UNUSED( config );
  Q_UNUSED( groupPrefix );
}

void KatePluginFileStoreView::writeSessionConfig( KConfigBase* config, const QString& groupPrefix )
{
  Q_UNUSED( config );
  Q_UNUSED( groupPrefix );
}

// kate: space-indent on; indent-width 2; replace-tabs on;

