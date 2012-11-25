/*  This file is part of the KDE libraries and the Kate part.
 *
 *  Copyright (C) 2003 Anders Lund <anders.lund@lund.tdcadsl.dk>
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

//BEGIN includes
#include "katenewcompletion.h"
#include "kateview.h"
#include "kateconfig.h"
#include "katedocument.h"
#include "kateglobal.h"

#include <ktexteditor/variableinterface.h>
#include <ktexteditor/movingrange.h>

#include <kconfig.h>
#include <kdialog.h>
#include <kpluginfactory.h>
#include <klocale.h>
#include <kaction.h>
#include <kactioncollection.h>
#include <knotification.h>
#include <kparts/part.h>
#include <kiconloader.h>
#include <kpagedialog.h>
#include <kpagewidgetmodel.h>
#include <ktoggleaction.h>
#include <kconfiggroup.h>
#include <kcolorscheme.h>
#include <kaboutdata.h>

#include <QtCore/QRegExp>
#include <QtCore/QString>
#include <QtCore/QSet>
#include <QtGui/QSpinBox>
#include <QtGui/QLabel>
#include <QtGui/QLayout>

#include <kvbox.h>
#include <QtGui/QCheckBox>

#include <kdebug.h>
//END

//BEGIN KateNewCompletionModel
KateNewCompletionModel::KateNewCompletionModel( QObject *parent )
  : CodeCompletionModel( parent ), m_automatic(false)
{
  setHasGroups(false);
}

KateNewCompletionModel::~KateNewCompletionModel()
{
}

void KateNewCompletionModel::saveMatches( KTextEditor::View* view,
                        const KTextEditor::Range& range)
{
  m_matches = allMatches( view, range );
  m_matches.sort();
}

QVariant KateNewCompletionModel::data(const QModelIndex& index, int role) const
{
  if( role == InheritanceDepth )
    return 10000; //Very high value, so the word-completion group and items are shown behind any other groups/items if there is multiple

  if( !index.parent().isValid() ) {
    //It is the group header
    switch ( role )
    {
      case Qt::DisplayRole:
        return i18n("Customized Completion");
      case GroupRole:
        return Qt::DisplayRole;
    }
  }

  if( index.column() == KTextEditor::CodeCompletionModel::Name && role == Qt::DisplayRole )
    return m_matches.at( index.row() );

  if( index.column() == KTextEditor::CodeCompletionModel::Icon && role == Qt::DecorationRole ) {
    static QIcon icon(KIcon("insert-text").pixmap(QSize(16, 16)));
    return icon;
  }

  return QVariant();
}

QModelIndex KateNewCompletionModel::parent(const QModelIndex& index) const
{
  if(index.internalId())
    return createIndex(0, 0, 0);
  else
    return QModelIndex();
}

QModelIndex KateNewCompletionModel::index(int row, int column, const QModelIndex& parent) const
{
  if( !parent.isValid()) {
    if(row == 0)
      return createIndex(row, column, 0);
    else
      return QModelIndex();

  }else if(parent.parent().isValid())
    return QModelIndex();


  if (row < 0 || row >= m_matches.count() || column < 0 || column >= ColumnCount )
    return QModelIndex();

  return createIndex(row, column, 1);
}

int KateNewCompletionModel::rowCount ( const QModelIndex & parent ) const
{
  if( !parent.isValid() && !m_matches.isEmpty() )
    return 1; //One root node to define the custom group
  else if(parent.parent().isValid())
    return 0; //Completion-items have no children
  else
    return m_matches.count();
}

bool KateNewCompletionModel::shouldStartCompletion(KTextEditor::View* view, const QString &insertedText, bool userInsertion, const KTextEditor::Cursor &position)
{
    if (!userInsertion) return false;
    if(insertedText.isEmpty())
        return false;

    KateView *v = qobject_cast<KateView*> (view);

    QString text = view->document()->line(position.line()).left(position.column());
    static const QRegExp ktuan_new_class("((new \\w*)|(gen\\w*)|(get\\w*))$");
    if (ktuan_new_class.indexIn(text) >= 0) return true;
    return false;
}

bool KateNewCompletionModel::shouldAbortCompletion(KTextEditor::View* view, const KTextEditor::Range &range, const QString &currentCompletion) {

    if (m_automatic) {
      if (currentCompletion.length()<3) return true;
    }
    static const QRegExp ktuan_new_class("((new \\w*)|(gen\\w*)|(get\\w*))$");
    return !ktuan_new_class.exactMatch(currentCompletion);
    // return CodeCompletionModelControllerInterface4::shouldAbortCompletion(view,range,currentCompletion);
}

void KateNewCompletionModel::completionInvoked(KTextEditor::View* view, const KTextEditor::Range& range, InvocationType it)
{
  /**
   * auto invoke...
   */
  m_automatic=false;
  if (it==AutomaticInvocation) {
      m_automatic=true;
      KateView *v = qobject_cast<KateView*> (view);

      if (range.columnWidth() >= 3)
        saveMatches( view, range );
      else
        m_matches.clear();

      // done here...
      return;
  }

  // normal case ;)
  saveMatches( view, range );
}


// Scan throughout the entire document for possible completions,
// ignoring any dublets
const QStringList KateNewCompletionModel::allMatches( KTextEditor::View *view, const KTextEditor::Range &range ) const
{
  KTextEditor::Document *doc = view->document();
  QString match_str = doc->text(range);
  QString s, m;
  QSet<QString> seen;
  QStringList l;

  int i( 0 );
  int pos( 0 );

  QRegExp class_rek("([A-Z]\\w*<( |\\w|,|<|>)+>)");
  if (match_str.startsWith("new ")) {
    QString class_name = match_str.mid(4);
    for (i = 0; i < doc->lines(); ++i) {
      QString s = doc->line(i);
      QString m;
      pos = 0;
      while (pos >= 0) {
        pos = class_rek.indexIn(s, pos);
        if ( pos >= 0 )
        {
          // typing in the middle of a word
          if ( ! ( i == range.start().line() && pos >= range.start().column() && pos <= range.end().column()) )
          {
            m = class_rek.cap( 1 );
            if ( ! seen.contains( m ) && m.startsWith(class_name)) {
              seen.insert( m );
              m = "new " + m;
              l << m;
            }
          }
          pos += class_rek.matchedLength();
        }
      }
    }
  }

  // convert yieldXXX and Ent::load('XXX') to genXXX and getXXX
  if (match_str.startsWith("gen") || match_str.startsWith("get")) {
    QString x = match_str.mid(3);
    class_rek = QRegExp("(yield|Ent::load\\(\\\'|Ent::load\\(\\\")([A-Z]\\w*)");
    for (i = 0; i < doc->lines(); ++i) {
      QString s = doc->line(i);
      QString m;
      pos = 0;
      while (pos >= 0) {
        pos = class_rek.indexIn(s, pos);
        if ( pos >= 0 )
        {
          // typing in the middle of a word
          if ( ! ( i == range.start().line() && pos >= range.start().column() && pos <= range.end().column()) )
          {
            m = class_rek.cap( 2 );
            if ( ! seen.contains( m ) && m.startsWith(x)) {
              seen.insert( m );
              l << ("gen" + m);
              l << ("get" + m);
            }
          }
          pos += class_rek.matchedLength();
        }
      }
    }
  }

  return l;
}

KTextEditor::CodeCompletionModelControllerInterface3::MatchReaction KateNewCompletionModel::matchingItem(const QModelIndex& /*matched*/)
{
  return HideListIfAutomaticInvocation;
}

bool KateNewCompletionModel::shouldHideItemsWithEqualNames() const
{
  // We don't want word-completion items if the same items
  // are available through more sophisticated completion models
  return true;
}

// Return the range containing the word left of the cursor
KTextEditor::Range KateNewCompletionModel::completionRange(KTextEditor::View* view, const KTextEditor::Cursor &position)
{
  int line = position.line();
  int col = position.column();

  KTextEditor::Document *doc = view->document();

  // ktuan java case: new List<Integer>
  // yieldXXX, Ent::load('XXX, genXXX, getXXX
  {
    QString text = view->document()->line(position.line()).left(position.column());
    const static QRegExp ktuan_new_class("((new \\w*)|(gen\\w*)|(get\\w*))$");
    int pos = ktuan_new_class.indexIn(text);
    if (pos >= 0) {
      return KTextEditor::Range( KTextEditor::Cursor( line, pos ), position );
    }
  }

  return KTextEditor::Range( KTextEditor::Cursor( line, col ), position );
}
//END KateNewCompletionModel

#include "katenewcompletion.moc"
// kate: space-indent on; indent-width 2; replace-tabs on; mixed-indent off;
