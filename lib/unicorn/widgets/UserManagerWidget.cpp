#include "UserManagerWidget.h"

#include "lib/unicorn/dialogs/LoginContinueDialog.h"
#include "lib/unicorn/dialogs/LoginDialog.h"
#include "lib/unicorn/dialogs/WelcomeDialog.h"
#include "lib/unicorn/LoginProcess.h"
#include "lib/unicorn/QMessageBoxBuilder.h"
#include "lib/unicorn/UnicornApplication.h"
#include "lib/unicorn/UnicornSettings.h"
#include "lib/unicorn/UnicornSession.h"

#include <lastfm/User>

#include <QApplication>
#include <QButtonGroup>
#include <QDebug>
#include <QDialogButtonBox>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QMouseEvent>
#include <QPushButton>
#include <QTimer>
#include <QVBoxLayout>

using lastfm::UserDetails;
UserRadioButton::UserRadioButton( const User& user )
                :m_userName( user.name() )
{
    setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Fixed );

    if( user == User()) setChecked( true );

    QHBoxLayout* l = new QHBoxLayout();
    l->setContentsMargins( 20, 0, 0, 0 );
    l->addWidget( m_image = new QLabel(), 0, Qt::AlignTop);
    m_image->setFrameShape( QFrame::Box );
    m_image->setFrameShadow( QFrame::Plain );
    unsigned int framewidth = m_image->lineWidth() * 2;
    m_image->setMinimumSize( QSize( 35+framewidth, 35+ framewidth ));
    m_image->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::MinimumExpanding );
    
    installEventFilter( this );
    if( user.imageUrl( lastfm::Medium ).isEmpty()) {
        QNetworkReply* reply = UserDetails::getInfo( user.name() );
        connect( reply, SIGNAL(finished()), SLOT( onUserDetailsFetched()));
    } else {
        QNetworkReply* reply = lastfm::nam()->get( QNetworkRequest( user.imageUrl(lastfm::Medium)));
        connect( reply, SIGNAL(finished()), SLOT( onImageLoaded()));
    }
    QGridLayout* gl = new QGridLayout();
    gl->setSpacing( 0 );
    gl->addWidget( m_name = new QLabel(user.name()), 0, 0, Qt::AlignTop );
    gl->addWidget( m_realName = new QLabel(), 0, 1, Qt::AlignTop );
    gl->addWidget( m_loggedIn = new QLabel(), 1, 0, 1, 2, Qt::AlignTop );
    m_realName->setObjectName( "realname" );
    l->addLayout( gl );
    l->addStretch();
    QPushButton* remove;
    l->addWidget( remove = new QPushButton( tr("Remove") ));
    connect( remove, SIGNAL(clicked()), SLOT(removeMe()));
    QVBoxLayout* vl = new QVBoxLayout(this);
    vl->addLayout( l );
    remove->setFocusPolicy( Qt::NoFocus );

    m_name->setAutoFillBackground( true );
    m_realName->setAutoFillBackground( true );
    m_loggedIn->setAutoFillBackground( true );
}

void 
UserRadioButton::onUserDetailsFetched()
{
    QNetworkReply* reply = qobject_cast<QNetworkReply*>( sender());
    Q_ASSERT( reply );

    UserDetails user( reply );

    m_name->setText( user.name());
    if( !user.realName().isEmpty())
        m_realName->setText( QString( " (%1)" ).arg(user.realName()));
    
    if( user == User())
        m_loggedIn->setText( "(" + tr( "currently logged in" ) + ")" );

    reply = lastfm::nam()->get( QNetworkRequest( user.imageUrl(lastfm::Medium)));
    connect( reply, SIGNAL(finished()), SLOT( onImageLoaded()));

    m_userName = user.name();
}

void 
UserRadioButton::onImageLoaded()
{
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    if( !reply ) return;

    QPixmap pm;
    pm.loadFromData( reply->readAll());
    m_image->setPixmap( pm.scaledToWidth( 35, Qt::SmoothTransformation ));
}

bool 
UserRadioButton::eventFilter( QObject* obj, QEvent* event )
{
    QMouseEvent* mouseEvent = dynamic_cast<QMouseEvent*>( event );
    if( !mouseEvent ) return false;

    int l,t,r,b;
    getContentsMargins( &l, &t, &r, &b );
    QMouseEvent mEvent( mouseEvent->type(), QPoint( l, height()/2 ), mouseEvent->button(), mouseEvent->buttons(), mouseEvent->modifiers());
    
    // Remove the event filter so that the fake mouse event can actually
    // be passed to the radio button and then set the eventFilter again.
    removeEventFilter( this );
    if( this )
        QCoreApplication::sendEvent( this, &mEvent );
    installEventFilter( this );

    return true;
}

void
UserRadioButton::removeMe()
{
    int result =
        QMessageBoxBuilder( this ).setTitle( tr( "Removing %1" ).arg( m_name->text() ) )
                                  .setText( tr( "Are you sure you want to remove this user? All user settings will be lost and you will need to re authenticate in order to scrobble in the future." ))
                                  .setIcon( QMessageBox::Question )
                                  .setButtons( QMessageBox::Yes | QMessageBox::No )
                                  .exec();
    if( result != QMessageBox::Yes ) return;

    unicorn::Settings us;
    us.remove( m_name->text() );

    if ( us.userRoster().count() == 0 )
    {
        us.setValue( "FirstRunWizardCompleted", false );
        qobject_cast<unicorn::Application*>( qApp )->restart();
    }

    if( isChecked()) {
        foreach (UserRadioButton* b, parentWidget()->findChildren<UserRadioButton*>()) {
            if( b->user() == User().name()) {
                b->click();
            }
        }
    }
    deleteLater();
}


const QString 
UserRadioButton::user() const
{
    return m_userName;
}


UserManagerWidget::UserManagerWidget( QWidget* parent )
            :QWidget( parent )
            , m_buttonGroup( new QButtonGroup( this ) )
            , m_loginProcess( 0 )
            , m_lcd( 0 )
{
    m_lcd = new LoginContinueDialog( this );
    connect( m_lcd, SIGNAL( accepted()), SLOT( onUserAdded()));

    QVBoxLayout* layout = new QVBoxLayout( this );
    layout->setSpacing( 10 );
    layout->addWidget( new QLabel( tr( "Users authenticated with this application" )));

    layout->addWidget( ui.groupBox = new QGroupBox() );

    ui.usersLayout = new QVBoxLayout( ui.groupBox );

    ui.groupBox->setTitle( tr( "Log me in as:" ));

    ui.usersLayout->addStretch( 1 );
    ui.groupBox->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::MinimumExpanding );

    QHBoxLayout* buttonLayout = new QHBoxLayout();

    ui.addUserButton = new QPushButton( tr( "Add new user" ), this );

    buttonLayout->addWidget( ui.addUserButton );
    buttonLayout->addStretch( 1 );

    ui.usersLayout->addLayout( buttonLayout );

    QList<lastfm::User> roster = unicorn::Settings().userRoster();

    foreach( lastfm::User u, roster ) {
        UserRadioButton* b = new UserRadioButton( u );
        add( b, false );
    }

    connect( ui.addUserButton, SIGNAL( clicked() ), SLOT( onAddUserClicked() ) );
    connect( m_buttonGroup, SIGNAL( buttonClicked( int ) ), this, SIGNAL( userChanged() ) );

    setTabOrders();
}


UserManagerWidget::~UserManagerWidget()
{
}

void
UserManagerWidget::onAddUserClicked()
{
    LoginDialog* ld = new LoginDialog( this );
    ld->setWindowFlags( Qt::Sheet );
    ld->open();
    connect( ld, SIGNAL( accepted()), SLOT( onLoginDialogAccepted()));
}


void
UserManagerWidget::onLoginDialogAccepted()
{
    LoginDialog* ld = qobject_cast<LoginDialog*>(sender());
    Q_ASSERT( ld );

    if ( m_loginProcess )
    {
        delete m_loginProcess;
        m_loginProcess = 0;
    }

    m_loginProcess = new unicorn::LoginProcess( this );

    ld->deleteLater();


    connect( m_loginProcess, SIGNAL( gotSession( unicorn::Session& ) ),
             this, SLOT( onGotSession( unicorn::Session& ) ) );

    m_loginProcess->authenticate();


    m_lcd->setWindowFlags( Qt::Sheet );
    m_lcd->open();
}


void
UserManagerWidget::onGotSession( unicorn::Session& s )
{
    Q_UNUSED( s )
    m_lcd->accept();
}

void 
UserManagerWidget::onUserAdded()
{
    Q_ASSERT( m_loginProcess );

    const unicorn::Session& s = m_loginProcess->session();

    bool alreadyAdded = false;
    foreach ( UserRadioButton* b, findChildren<UserRadioButton*>() )
    {
        if ( s.username() == b->user() )
        {
            alreadyAdded = true;
            break;
        }
    }

    if ( !alreadyAdded )
    {
        if ( s.isValid() )
        {
            User user( s.username());
            UserRadioButton* urb = new UserRadioButton( user );

            add( urb );
            if( ui.groupBox->layout()->count() <= 1 ) urb->click();
    
            setTabOrders();

            WelcomeDialog( user ).exec();
        }
        else
        {
            m_loginProcess->cancel();
            m_loginProcess->showError();
        }
    }
    else
    {
        QMessageBoxBuilder( this )
                    .setIcon( QMessageBox::Information )
                    .setTitle( tr( "User already added" ) )
                    .setText( tr( "This user has already been added." ) )
                    .exec();

    }
}

void 
UserManagerWidget::add( UserRadioButton* urb, bool announce )
{
    ui.usersLayout->insertWidget( ui.usersLayout->count() - 2, urb );
    m_buttonGroup->addButton( urb );

    if( announce )
        emit rosterUpdated();

    connect( urb, SIGNAL( destroyed(QObject*)), SIGNAL( rosterUpdated()));
}

void
UserManagerWidget::setTabOrders()
{
    if( m_buttonGroup->buttons().count() )
    {
        if( m_buttonGroup->checkedButton() )
        {
            qDebug() << "button: " << qobject_cast<UserRadioButton *>( m_buttonGroup->checkedButton() )->user();
            setTabOrder( m_buttonGroup->checkedButton(), ui.addUserButton );
        }
    }
 }

QAbstractButton*
UserManagerWidget::checkedButton() const
{
    return m_buttonGroup->checkedButton();
}
