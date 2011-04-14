#ifndef WINDOWMAIN_H
#define WINDOWMAIN_H

#include <lastfm/RadioStation>

#include "lib/unicorn/UnicornMainWindow.h"

namespace Ui { class WindowMain; }

class WindowMain : public unicorn::MainWindow
{
    Q_OBJECT

public:
    explicit WindowMain();
    ~WindowMain();

signals:
    void aboutToHide();

private slots:
    void onStartClicked();
    void onRadioTick( qint64 tick );
    void onTrackSpooled( const Track& track );
    void onTuningIn( const RadioStation& station );
    void onError( int error , const QVariant& errorData );
    void onStopped();

    void onGotRecentStations();
    void onGotEvents();

    void onLibraryClicked();
    void onMixClicked();
    void onRecomendedClicked();

    void onLoveClicked( bool loved );
    void onLoveTriggered( bool loved );
    void onBanClicked();
    void onBanFinished();
    void onSkipClicked();
    void onInfoClicked();
    void onPlayClicked( bool checked );
    void onPlayTriggered( bool checked );

    void onBackClicked();
    void onNowPlayingClicked();

    void onStationEditTextChanged( const QString& text );

    void onEditClicked();
    void onFilterClicked();

    void onActionsChanged();

    void onSwitch();

    void onSpace();

private:
    void addWinThumbBarButtons( QList<QAction*>& );
    void createActions();

private:
    Ui::WindowMain* ui;

    QAction* m_loveAction;
    QAction* m_banAction;
    QAction* m_playAction;
    QAction* m_skipAction;
};

#endif // WINDOWMAIN_H
