/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 5.9.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *centralwidget;
    QLabel *label;
    QLabel *label_temp;
    QLabel *label_press;
    QLabel *label_xang;
    QLabel *label_yang;
    QLabel *label_ip;
    QLabel *label_wifi;
    QLabel *label_eth0;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName(QStringLiteral("MainWindow"));
        MainWindow->resize(480, 320);
        MainWindow->setStyleSheet(QStringLiteral(""));
        centralwidget = new QWidget(MainWindow);
        centralwidget->setObjectName(QStringLiteral("centralwidget"));
        label = new QLabel(centralwidget);
        label->setObjectName(QStringLiteral("label"));
        label->setGeometry(QRect(0, 0, 480, 320));
        label->setStyleSheet(QLatin1String("border-image: url(:/Frame 12(2).png);\n"
""));
        label_temp = new QLabel(centralwidget);
        label_temp->setObjectName(QStringLiteral("label_temp"));
        label_temp->setGeometry(QRect(100, 43, 51, 20));
        QFont font;
        font.setPointSize(15);
        label_temp->setFont(font);
        label_temp->setStyleSheet(QStringLiteral("color: rgb(255, 255, 255);"));
        label_temp->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignVCenter);
        label_press = new QLabel(centralwidget);
        label_press->setObjectName(QStringLiteral("label_press"));
        label_press->setGeometry(QRect(155, 43, 90, 20));
        label_press->setFont(font);
        label_press->setStyleSheet(QStringLiteral("color: rgb(255, 255, 255);"));
        label_press->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignVCenter);
        label_xang = new QLabel(centralwidget);
        label_xang->setObjectName(QStringLiteral("label_xang"));
        label_xang->setGeometry(QRect(300, 80, 161, 51));
        QFont font1;
        font1.setPointSize(30);
        font1.setBold(true);
        font1.setWeight(75);
        label_xang->setFont(font1);
        label_xang->setStyleSheet(QStringLiteral("color: rgb(254, 245, 0);"));
        label_xang->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignVCenter);
        label_yang = new QLabel(centralwidget);
        label_yang->setObjectName(QStringLiteral("label_yang"));
        label_yang->setGeometry(QRect(300, 140, 161, 41));
        label_yang->setFont(font1);
        label_yang->setStyleSheet(QLatin1String("color: rgb(254, 245, 0);\n"
""));
        label_yang->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignVCenter);
        label_ip = new QLabel(centralwidget);
        label_ip->setObjectName(QStringLiteral("label_ip"));
        label_ip->setGeometry(QRect(280, 183, 141, 40));
        label_ip->setFont(font);
        label_ip->setStyleSheet(QStringLiteral("color: rgb(255, 255, 255);"));
        label_wifi = new QLabel(centralwidget);
        label_wifi->setObjectName(QStringLiteral("label_wifi"));
        label_wifi->setGeometry(QRect(380, 43, 20, 20));
        label_wifi->setStyleSheet(QStringLiteral(""));
        label_eth0 = new QLabel(centralwidget);
        label_eth0->setObjectName(QStringLiteral("label_eth0"));
        label_eth0->setGeometry(QRect(360, 43, 20, 20));
        label_eth0->setStyleSheet(QStringLiteral(""));
        MainWindow->setCentralWidget(centralwidget);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QApplication::translate("MainWindow", "MainWindow", Q_NULLPTR));
        label->setText(QString());
        label_temp->setText(QApplication::translate("MainWindow", "28.1\342\204\203", Q_NULLPTR));
        label_press->setText(QApplication::translate("MainWindow", "101.1Kpa", Q_NULLPTR));
        label_xang->setText(QString());
        label_yang->setText(QString());
        label_ip->setText(QApplication::translate("MainWindow", "192.168.2.218", Q_NULLPTR));
        label_wifi->setText(QString());
        label_eth0->setText(QString());
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
