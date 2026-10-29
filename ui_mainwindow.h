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
    QLabel *label_net;

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
        label->setStyleSheet(QLatin1String("image: url(:/ArcMap-II.png);\n"
""));
        label_temp = new QLabel(centralwidget);
        label_temp->setObjectName(QStringLiteral("label_temp"));
        label_temp->setGeometry(QRect(350, 30, 91, 41));
        QFont font;
        font.setPointSize(7);
        label_temp->setFont(font);
        label_temp->setStyleSheet(QStringLiteral("color: rgb(255, 255, 255);"));
        label_temp->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignVCenter);
        label_press = new QLabel(centralwidget);
        label_press->setObjectName(QStringLiteral("label_press"));
        label_press->setGeometry(QRect(500, 30, 321, 41));
        label_press->setFont(font);
        label_press->setStyleSheet(QStringLiteral("color: rgb(255, 255, 255);"));
        label_press->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignVCenter);
        label_xang = new QLabel(centralwidget);
        label_xang->setObjectName(QStringLiteral("label_xang"));
        label_xang->setGeometry(QRect(145, 170, 311, 91));
        QFont font1;
        font1.setPointSize(12);
        font1.setBold(true);
        font1.setWeight(75);
        label_xang->setFont(font1);
        label_xang->setStyleSheet(QStringLiteral("color: rgb(254,245, 0);"));
        label_xang->setAlignment(Qt::AlignCenter);
        label_yang = new QLabel(centralwidget);
        label_yang->setObjectName(QStringLiteral("label_yang"));
        label_yang->setGeometry(QRect(489, 170, 311, 91));
        label_yang->setFont(font1);
        label_yang->setStyleSheet(QLatin1String("color: rgb(254,245, 0);\n"
""));
        label_yang->setAlignment(Qt::AlignCenter);
        label_ip = new QLabel(centralwidget);
        label_ip->setObjectName(QStringLiteral("label_ip"));
        label_ip->setGeometry(QRect(430, 310, 351, 41));
        QFont font2;
        font2.setPointSize(8);
        label_ip->setFont(font2);
        label_ip->setStyleSheet(QStringLiteral("color: rgb(255, 255, 255);"));
        label_net = new QLabel(centralwidget);
        label_net->setObjectName(QStringLiteral("label_net"));
        label_net->setGeometry(QRect(300, 300, 48, 48));
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
        label_xang->setText(QApplication::translate("MainWindow", "-0.5", Q_NULLPTR));
        label_yang->setText(QApplication::translate("MainWindow", "-0.3", Q_NULLPTR));
        label_ip->setText(QApplication::translate("MainWindow", "192.168.2.218", Q_NULLPTR));
        label_net->setText(QString());
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
