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
#include "katexhpcompletion.h"
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

//BEGIN KateXHPCompletionModel
KateXHPCompletionModel::KateXHPCompletionModel( QObject *parent )
  : CodeCompletionModel( parent ), m_automatic(false)
{
  setHasGroups(false);
}

KateXHPCompletionModel::~KateXHPCompletionModel()
{
}

void KateXHPCompletionModel::saveMatches( KTextEditor::View* view,
                        const KTextEditor::Range& range)
{
  m_matches = allMatches( view, range );
  m_matches.sort();
}

QVariant KateXHPCompletionModel::data(const QModelIndex& index, int role) const
{
  if( role == InheritanceDepth )
    return 10000; //Very high value, so the word-completion group and items are shown behind any other groups/items if there is multiple

  if( !index.parent().isValid() ) {
    //It is the group header
    switch ( role )
    {
      case Qt::DisplayRole:
        return i18n("Auto XHP");
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

QModelIndex KateXHPCompletionModel::parent(const QModelIndex& index) const
{
  if(index.internalId())
    return createIndex(0, 0, 0);
  else
    return QModelIndex();
}

QModelIndex KateXHPCompletionModel::index(int row, int column, const QModelIndex& parent) const
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

int KateXHPCompletionModel::rowCount ( const QModelIndex & parent ) const
{
  if( !parent.isValid() && !m_matches.isEmpty() )
    return 1; //One root node to define the custom group
  else if(parent.parent().isValid())
    return 0; //Completion-items have no children
  else
    return m_matches.count();
}

bool KateXHPCompletionModel::shouldStartCompletion(KTextEditor::View* view, const QString &insertedText, bool userInsertion, const KTextEditor::Cursor &position)
{
    if (!userInsertion) return false;
    if(insertedText.isEmpty())
        return false;

    KateView *v = qobject_cast<KateView*> (view);

    QString text = view->document()->line(position.line()).left(position.column());
    static const QRegExp ktuan_new_class("((</|<| :)([a-z]|[0-9]|:|-)+)$");
    if (ktuan_new_class.indexIn(text) >= 0) return true;
    return false;
}

bool KateXHPCompletionModel::shouldAbortCompletion(KTextEditor::View* view, const KTextEditor::Range &range, const QString &currentCompletion) {

    if (m_automatic) {
      if (currentCompletion.length()<1) return true;
    }
    static const QRegExp ktuan_new_class("([a-z]|[0-9]|:|-)+$");
    return !ktuan_new_class.exactMatch(currentCompletion);// &&
}

void KateXHPCompletionModel::completionInvoked(KTextEditor::View* view, const KTextEditor::Range& range, InvocationType it)
{
  /**
   * auto invoke...
   */
  m_automatic=false;
  if (it==AutomaticInvocation) {
      m_automatic=true;
      KateView *v = qobject_cast<KateView*> (view);

      if (range.columnWidth() >= 1)
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
const QStringList KateXHPCompletionModel::allMatches( KTextEditor::View *view, const KTextEditor::Range &range ) const
{
  KTextEditor::Document *doc = view->document();
  QString match_str = doc->text(range);
  QString s, m;
  QSet<QString> seen;
  QStringList l;

  int i( 0 );
  int pos( 0 );

  QRegExp re( "\\b(" + match_str + "([a-z]|[0-9]|:|-)*)" );

  while( i < doc->lines() )
  {
      s = doc->line( i );
      pos = 0;
      while ( pos >= 0 )
      {
        pos = re.indexIn( s, pos );
        if ( pos >= 0 )
        {
          // typing in the middle of a word
          if ( ! ( i == range.start().line() && pos == range.start().column() ) )
          {
            m = re.cap( 1 );
            if ( ! seen.contains( m ) ) {
              seen.insert( m );
              l << m;
            }
          }
          pos += re.matchedLength();
        }
      }
    i++;
  }
  return l;
}

KTextEditor::CodeCompletionModelControllerInterface3::MatchReaction KateXHPCompletionModel::matchingItem(const QModelIndex& /*matched*/)
{
  return HideListIfAutomaticInvocation;
}

bool KateXHPCompletionModel::shouldHideItemsWithEqualNames() const
{
  // We don't want word-completion items if the same items
  // are available through more sophisticated completion models
  return true;
}

// Return the range containing the word left of the cursor
KTextEditor::Range KateXHPCompletionModel::completionRange(KTextEditor::View* view, const KTextEditor::Cursor &position)
{
  int line = position.line();
  int col = position.column();

  KTextEditor::Document *doc = view->document();

  int col2 = position.column();
  while (col2 > 0) {
    QChar c = ( doc->character( KTextEditor::Cursor( line, col2-1 ) ) );
    if (c.isLower() || c.isNumber() || c == '-' || c == ':') {
      col2--;
      continue;
    }

    break;
  }

  {
    QChar c = ( doc->character( KTextEditor::Cursor( line, col2 ) ) );
    if (c == ':') {
      return KTextEditor::Range( KTextEditor::Cursor( line, col2 + 1 ), position );
    } else if (col2 > 0) {
      c = ( doc->character( KTextEditor::Cursor( line, col2 - 1 ) ) );
      if (c == '<' || c == '/') {
        return KTextEditor::Range( KTextEditor::Cursor( line, col2 ), position );
      }
    }
  }

  return KTextEditor::Range( KTextEditor::Cursor( line, col ), position );
}
//END KateXHPCompletionModel

#include "katexhpcompletion.moc"
// kate: space-indent on; indent-width 2; replace-tabs on; mixed-indent off;
