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
    QLabel *label_2;
    QLabel *label_3;
    QLabel *label_4;
    QLabel *label_5;
    QLabel *label_6;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName(QStringLiteral("MainWindow"));
        MainWindow->resize(960, 360);
        MainWindow->setStyleSheet(QStringLiteral(""));
        centralwidget = new QWidget(MainWindow);
        centralwidget->setObjectName(QStringLiteral("centralwidget"));
        label = new QLabel(centralwidget);
        label->setObjectName(QStringLiteral("label"));
        label->setGeometry(QRect(0, 0, 960, 376));
        label->setStyleSheet(QLatin1String("background-color: rgb(0, 0,255);\n"
""));
        label_temp = new QLabel(centralwidget);
        label_temp->setObjectName(QStringLiteral("label_temp"));
        label_temp->setGeometry(QRect(210, 40, 131, 41));
        QFont font;
        font.setPointSize(8);
        label_temp->setFont(font);
        label_temp->setStyleSheet(QStringLiteral("color: rgb(255, 255, 255);"));
        label_temp->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignVCenter);
        label_press = new QLabel(centralwidget);
        label_press->setObjectName(QStringLiteral("label_press"));
        label_press->setGeometry(QRect(420, 40, 321, 41));
        label_press->setFont(font);
        label_press->setStyleSheet(QStringLiteral("color: rgb(255, 255, 255);"));
        label_press->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignVCenter);
        label_xang = new QLabel(centralwidget);
        label_xang->setObjectName(QStringLiteral("label_xang"));
        label_xang->setGeometry(QRect(530, 100, 391, 61));
        QFont font1;
        font1.setPointSize(8);
        font1.setBold(true);
        font1.setWeight(75);
        label_xang->setFont(font1);
        label_xang->setStyleSheet(QStringLiteral("color: rgb(255,255, 255);"));
        label_xang->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignVCenter);
        label_yang = new QLabel(centralwidget);
        label_yang->setObjectName(QStringLiteral("label_yang"));
        label_yang->setGeometry(QRect(530, 170, 391, 61));
        label_yang->setFont(font1);
        label_yang->setStyleSheet(QLatin1String("color: rgb(255, 255, 255);\n"
""));
        label_yang->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignVCenter);
        label_ip = new QLabel(centralwidget);
        label_ip->setObjectName(QStringLiteral("label_ip"));
        label_ip->setGeometry(QRect(300, 240, 351, 61));
        label_ip->setFont(font);
        label_ip->setStyleSheet(QStringLiteral("color: rgb(255, 255, 255);"));
        label_wifi = new QLabel(centralwidget);
        label_wifi->setObjectName(QStringLiteral("label_wifi"));
        label_wifi->setEnabled(true);
        label_wifi->setGeometry(QRect(830, 50, 20, 20));
        label_wifi->setStyleSheet(QStringLiteral(""));
        label_eth0 = new QLabel(centralwidget);
        label_eth0->setObjectName(QStringLiteral("label_eth0"));
        label_eth0->setEnabled(true);
        label_eth0->setGeometry(QRect(750, 50, 20, 20));
        label_eth0->setStyleSheet(QStringLiteral(""));
        label_2 = new QLabel(centralwidget);
        label_2->setObjectName(QStringLiteral("label_2"));
        label_2->setGeometry(QRect(220, 240, 54, 61));
        label_2->setFont(font);
        label_2->setStyleSheet(QStringLiteral("color: rgb(255, 255, 255);"));
        label_2->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
        label_3 = new QLabel(centralwidget);
        label_3->setObjectName(QStringLiteral("label_3"));
        label_3->setGeometry(QRect(440, 100, 54, 61));
        label_3->setFont(font);
        label_3->setStyleSheet(QStringLiteral("color: rgb(255, 255, 255);"));
        label_3->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
        label_4 = new QLabel(centralwidget);
        label_4->setObjectName(QStringLiteral("label_4"));
        label_4->setGeometry(QRect(440, 170, 54, 61));
        label_4->setFont(font);
        label_4->setStyleSheet(QStringLiteral("color: rgb(255, 255, 255);"));
        label_4->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
        label_5 = new QLabel(centralwidget);
        label_5->setObjectName(QStringLiteral("label_5"));
        label_5->setGeometry(QRect(113, 40, 81, 40));
        label_5->setFont(font);
        label_5->setStyleSheet(QStringLiteral("color: rgb(255, 255, 255);"));
        label_5->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
        label_6 = new QLabel(centralwidget);
        label_6->setObjectName(QStringLiteral("label_6"));
        label_6->setGeometry(QRect(350, 40, 61, 40));
        label_6->setFont(font);
        label_6->setStyleSheet(QStringLiteral("color: rgb(255, 255, 255);"));
        label_6->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
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
        label_xang->setText(QApplication::translate("MainWindow", "1", Q_NULLPTR));
        label_yang->setText(QApplication::translate("MainWindow", "2", Q_NULLPTR));
        label_ip->setText(QApplication::translate("MainWindow", "192.168.2.218", Q_NULLPTR));
        label_wifi->setText(QString());
        label_eth0->setText(QString());
        label_2->setText(QApplication::translate("MainWindow", "IP:", Q_NULLPTR));
        label_3->setText(QApplication::translate("MainWindow", "X:", Q_NULLPTR));
        label_4->setText(QApplication::translate("MainWindow", "Y:", Q_NULLPTR));
        label_5->setText(QApplication::translate("MainWindow", "T:", Q_NULLPTR));
        label_6->setText(QApplication::translate("MainWindow", "P:", Q_NULLPTR));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
