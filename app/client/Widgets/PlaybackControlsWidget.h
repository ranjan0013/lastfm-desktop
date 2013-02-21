/*
   Copyright 2005-2009 Last.fm Ltd.
      - Primarily authored by Max Howell, Jono Cole and Doug Mansell

   This file is part of the Last.fm Desktop Application Suite.

   lastfm-desktop is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   lastfm-desktop is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with lastfm-desktop.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef PLAYBACKCONTROLS_H
#define PLAYBACKCONTROLS_H

#include <QAction>
#include <QFrame>
#include <QPointer>

#include <lastfm/RadioStation.h>
#include <lastfm/Track.h>

namespace unicorn { class Session; }

namespace Ui { class PlaybackControlsWidget; }

class QMovie;

class PlaybackControlsWidget : public QFrame
{
    Q_OBJECT

public:
    explicit PlaybackControlsWidget(QWidget* parent = 0);
    ~PlaybackControlsWidget();

public:
    Q_PROPERTY( bool scrobbleTrack READ scrobbleTrack WRITE setScrobbleTrack )

    bool scrobbleTrack() { return m_scrobbleTrack; }
    void setScrobbleTrack( bool scrobbleTrack );

    void addToMenu( class QMenu& menu, QAction* before = 0 );

private slots:
    void onSessionChanged( const unicorn::Session& session );

    void onActionsChanged();
    void onSpace();
    void onPlayClicked( bool checked );
    void onSkipClicked();
    void onLoveClicked( bool loved );
    void onLoveTriggered( bool loved );
    void onBanClicked();
    void onBanFinished();

    void onTuningIn( const RadioStation& station );
    void onTrackStarted( const Track& track, const Track& oldTrack );
    void onError( int error , const QVariant& errorData );
    void onStopped();

    void onTick( qint64 );

private:
    Ui::PlaybackControlsWidget *ui;

    QPointer<QAction> m_playAction;

    bool m_scrobbleTrack;
};

#endif // PLAYBACKCONTROLS_H
