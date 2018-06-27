﻿#include <QPainter>
#include <QKeyEvent>
#include <QDebug>
#include <QFileInfo>
#ifdef WIN32 
#include <Windows.h>
#endif
#ifdef Q_OS_MACOS || Q_OS_MAC32 || Q_OS_MAC64
#include <ApplicationServices/ApplicationServices.h>
#endif
#include "lockpage.h"
#include "ui_lockpage.h"
#include "goopal.h"
#include "datamgr.h"
#include "debug_log.h"
#include "commondialog.h"
#include "macro.h"

LockPage::LockPage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::LockPage)
{
    DLOG_QT_WALLET_FUNCTION_BEGIN;

    ui->setupUi(this);

    chances = 5;

    setAutoFillBackground(true);
    QPalette palette;
    palette.setBrush(QPalette::Background, QBrush(QPixmap(DataMgr::getDataMgr()->getWorkPath() + "pic2/bg.png")));
    setPalette(palette);

    ui->logoLabel->setPixmap(QPixmap(DataMgr::getDataMgr()->getWorkPath() + "pic2/Achainwallet.png"));

	QString language = DataMgr::getInstance()->getLanguage();
	if (language == "English")
    {
        ui->lockedLabel->setPixmap(QPixmap(DataMgr::getDataMgr()->getWorkPath() + "pic2/locked_En.png"));
        ui->lockedLabel->setGeometry(454,310,72,13);
    }
    else if( language == "Simplified Chinese")
    {
        ui->lockedLabel->setPixmap(QPixmap(DataMgr::getDataMgr()->getWorkPath() + "pic2/locked.png"));
    }

    ui->pwdLineEdit->setStyleSheet("color:white;background:transparent;border-width:0;border-style:outset;");
    ui->pwdLineEdit->setFocus();
    ui->pwdLineEdit->setContextMenuPolicy(Qt::NoContextMenu);
    ui->pwdLineEdit->setAttribute(Qt::WA_InputMethodEnabled, false);
    ui->pwdLineEdit->setAttribute(Qt::WA_MacShowFocusRect, false);

    ui->unlockBtn->setStyleSheet(QString("QToolButton{background-image:url(%1pic2/arrow.png);background-repeat: repeat-xy;background-position: center;background-attachment: fixed;background-clip: padding;border-style: flat;}"
                                 "QToolButton:hover{background-image:url(%2pic2/arrow_hover.png);background-repeat: repeat-xy;background-position: center;background-attachment: fixed;background-clip: padding;border-style: flat;}").arg(DataMgr::getDataMgr()->getWorkPath()).arg(DataMgr::getDataMgr()->getWorkPath()));

    ui->minBtn->setStyleSheet(QString("QToolButton{background-image:url(%1pic2/minimize2.png);background-repeat: repeat-xy;background-position: center;background-attachment: fixed;background-clip: padding;border-style: flat;}"
                              "QToolButton:hover{background-image:url(%2pic2/minimize_hover.png);background-repeat: repeat-xy;background-position: center;background-attachment: fixed;background-clip: padding;border-style: flat;}").arg(DataMgr::getDataMgr()->getWorkPath()).arg(DataMgr::getDataMgr()->getWorkPath()));
    ui->closeBtn->setStyleSheet(QString("QToolButton{background-image:url(%1pic2/close2.png);background-repeat: repeat-xy;background-position: center;background-attachment: fixed;background-clip: padding;border-style: flat;}"
                                "QToolButton:hover{background-image:url(%2pic2/close_hover.png);background-repeat: repeat-xy;background-position: center;background-attachment: fixed;background-clip: padding;border-style: flat;}").arg(DataMgr::getDataMgr()->getWorkPath()).arg(DataMgr::getDataMgr()->getWorkPath()));

	ui->versionLabel->setText("Achain Wallet Lite " ACHAIN_WALLET_VERSION_STR);

#ifdef WIN32
    if( GetKeyState(VK_CAPITAL) )  //zxlwin
    {
        ui->tipLabel->setText( tr("Caps lock opened!") );
    }
#endif // WIN32

#ifdef Q_OS_MACOS || Q_OS_MAC32 || Q_OS_MAC64
    bool wasCapsLockDown = CGEventSourceKeyState(kCGEventSourceStateHIDSystemState, 57);
    if (wasCapsLockDown)
        ui->tipLabel->setText(tr("Caps lock opened!"));
#endif

    timerForLockTime = new QTimer(this);
    connect(timerForLockTime, SIGNAL(timeout()), this, SLOT(checkLock()));
    timerForLockTime->start(1000);

    checkLock();

	connect(DataMgr::getInstance(), &DataMgr::onWalletUnlock, this, &LockPage::walletUnlock);

    DLOG_QT_WALLET_FUNCTION_END;
}

LockPage::~LockPage()
{
    DLOG_QT_WALLET_FUNCTION_BEGIN;
    delete ui;

    DLOG_QT_WALLET_FUNCTION_END;
}

void LockPage::on_unlockBtn_clicked()
{
    DLOG_QT_WALLET_FUNCTION_BEGIN;

    if( ui->pwdLineEdit->text().isEmpty()) {
        ui->tipLabel->setText( tr("Empty!") );
        return;
    }

    if( ui->pwdLineEdit->text().size() < 8) {
        ui->tipLabel->setText(tr("At least 8 letters!"));
        ui->pwdLineEdit->clear();
        return;
    }
    QString text(ui->pwdLineEdit->text());
    DataMgr::getInstance()->walletUnlock(text);
    qDebug() << "id_unlock_lockpage" ;
    ui->pwdLineEdit->setEnabled(false);

    DLOG_QT_WALLET_FUNCTION_END;
}

void LockPage::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    QPen pen;
    pen.setColor(QColor(255,255,255));
    pen.setStyle(Qt::SolidLine);
    pen.setWidth(1);
    painter.setPen(pen);
    painter.drawLine(QPoint(370,372),QPoint(596,372));
}


void LockPage::on_pwdLineEdit_returnPressed()
{
    DLOG_QT_WALLET_FUNCTION_BEGIN;

    on_unlockBtn_clicked();

    DLOG_QT_WALLET_FUNCTION_END;
}

void LockPage::on_minBtn_clicked()
{
    //if( Goopal::getInstance()->minimizeToTray)
	if (false) {
        emit tray();
    } else {
        emit minimum();
    }
}

void LockPage::on_closeBtn_clicked()
{
    //if( Goopal::getInstance()->minimizeToTray)
	if (false) {
        emit tray();
    } else {
        CommonDialog commonDialog(CommonDialog::OkAndCancel);
        commonDialog.setText( tr( "Close Achain Wallet Lite?"));
        bool choice = commonDialog.pop();

        if( choice) {
            emit closeGOP();
        } else {
            return;
        }
    }
}

void LockPage::on_pwdLineEdit_textChanged(const QString &arg1)
{
    if( !arg1.isEmpty())
    {
        ui->tipLabel->clear();
    }

    if( arg1.indexOf(' ') > -1)
    {
        ui->pwdLineEdit->setText( ui->pwdLineEdit->text().remove(' '));
    }
}

void LockPage::walletUnlock(bool result)
{
	ui->pwdLineEdit->clear();
	ui->pwdLineEdit->setEnabled(true);
	ui->pwdLineEdit->setFocus();

    if(result)
    {
		emit unlock();
	}
	else {
		ui->tipLabel->setText(tr("Wrong password!"));

		chances--;
		qDebug() << "chances : " << chances;
        if (chances < 1)
            checkLock();
	}
 
}

int LockPage::lockTime()
{
    QFileInfo fileInfo;

    if( fileInfo.exists())
    {
        QDateTime createdTime = fileInfo.lastModified();
        int deltaTime = QDateTime::currentDateTime().toTime_t() - createdTime.toTime_t();

        if( deltaTime < PWD_LOCK_TIME)
        {
            return PWD_LOCK_TIME - deltaTime;
        }
        else
        {
            return 0;
        }

    }
    else
    {
        return 0;
    }
}

void LockPage::checkLock()
{
    int timeLeft = lockTime();
    if( timeLeft > 0)
    {
        ui->pwdLineEdit->setEnabled(false);
        ui->unlockBtn->setEnabled(false);

        if( timeLeft > 3600)
        {
            int hours   = timeLeft / 3600;
            int minutes = (timeLeft - 3600 * hours) / 60;
            ui->tipLabel->setText( tr("Please try after ") + QString::number( hours) + tr(" hours ") +
                                   QString::number( minutes) + tr(" minutes ") + tr(".  "));
        }
        else if( timeLeft > 60)
        {
            int minutes = timeLeft / 60;
            ui->tipLabel->setText( tr("Please try after ") + QString::number( minutes) + tr(" minutes ") + tr(".  "));
        }
        else
        {
            ui->tipLabel->setText( tr("Please try after ") + QString::number( timeLeft) + tr(" seconds ") + tr(".  "));
        }

        ui->pwdLineEdit->clear();
        return;
    }
    else
    {
        ui->pwdLineEdit->setEnabled(true);
        ui->unlockBtn->setEnabled(true);
        if( ui->tipLabel->text().startsWith( tr("Please try after ")))
        {
            ui->tipLabel->clear();
        }
    }
}

void LockPage::keyPressEvent(QKeyEvent *e)
{
    QWidget::keyPressEvent(e);

    if(e->key() == Qt::Key_CapsLock)
    {
#ifdef WIN32
        if( GetKeyState(VK_CAPITAL) == -127 )  // 不知道为什么跟构造函数中同样的调用返回的short不一样  //zxlwin
        {
            ui->tipLabel->setText( tr("Caps lock opened!") );
        }
        else
        {
            ui->tipLabel->setText( tr("Caps lock closed!") );
        }
#endif // WIN32

#ifdef Q_OS_MACOS || Q_OS_MAC32 || Q_OS_MAC64
        ui->tipLabel->setText(tr("Caps lock opened!"));
#endif
    }
}

void LockPage::keyReleaseEvent(QKeyEvent *e)
{
    QWidget::keyReleaseEvent(e);

    if(e->key() == Qt::Key_CapsLock)
    {
#ifdef Q_OS_MACOS || Q_OS_MAC32 || Q_OS_MAC64
        ui->tipLabel->setText(tr("Caps lock closed!"));
#endif
    }
}
