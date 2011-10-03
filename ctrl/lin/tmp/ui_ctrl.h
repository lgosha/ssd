/********************************************************************************
** Form generated from reading UI file 'ctrl.ui'
**
** Created: Fri Sep 30 10:55:29 2011
**      by: Qt User Interface Compiler version 4.6.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_CTRL_H
#define UI_CTRL_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QComboBox>
#include <QtGui/QGroupBox>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QMainWindow>
#include <QtGui/QPushButton>
#include <QtGui/QStatusBar>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_ctrlClass
{
public:
    QWidget *centralWidget;
    QGroupBox *groupBox;
    QPushButton *m_pStart_button;
    QPushButton *m_pStop_button;
    QPushButton *m_pRestart_button;
    QGroupBox *groupBox_2;
    QComboBox *m_pApp_combo;
    QLabel *m_pAppStatus_label;
    QGroupBox *groupBox_3;
    QComboBox *m_pVersion_combo;
    QLabel *label_2;
    QComboBox *m_pFile_combo;
    QLabel *label_3;
    QPushButton *m_pChange_button;
    QPushButton *m_pApply_button;
    QLabel *m_pStatus_label;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *ctrlClass)
    {
        if (ctrlClass->objectName().isEmpty())
            ctrlClass->setObjectName(QString::fromUtf8("ctrlClass"));
        ctrlClass->resize(528, 281);
        QSizePolicy sizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(ctrlClass->sizePolicy().hasHeightForWidth());
        ctrlClass->setSizePolicy(sizePolicy);
        centralWidget = new QWidget(ctrlClass);
        centralWidget->setObjectName(QString::fromUtf8("centralWidget"));
        groupBox = new QGroupBox(centralWidget);
        groupBox->setObjectName(QString::fromUtf8("groupBox"));
        groupBox->setGeometry(QRect(10, 70, 141, 151));
        m_pStart_button = new QPushButton(groupBox);
        m_pStart_button->setObjectName(QString::fromUtf8("m_pStart_button"));
        m_pStart_button->setGeometry(QRect(20, 30, 75, 23));
        m_pStop_button = new QPushButton(groupBox);
        m_pStop_button->setObjectName(QString::fromUtf8("m_pStop_button"));
        m_pStop_button->setGeometry(QRect(20, 70, 75, 23));
        m_pRestart_button = new QPushButton(groupBox);
        m_pRestart_button->setObjectName(QString::fromUtf8("m_pRestart_button"));
        m_pRestart_button->setGeometry(QRect(20, 110, 75, 23));
        groupBox_2 = new QGroupBox(centralWidget);
        groupBox_2->setObjectName(QString::fromUtf8("groupBox_2"));
        groupBox_2->setGeometry(QRect(10, 10, 501, 51));
        m_pApp_combo = new QComboBox(groupBox_2);
        m_pApp_combo->setObjectName(QString::fromUtf8("m_pApp_combo"));
        m_pApp_combo->setGeometry(QRect(10, 20, 101, 21));
        m_pAppStatus_label = new QLabel(groupBox_2);
        m_pAppStatus_label->setObjectName(QString::fromUtf8("m_pAppStatus_label"));
        m_pAppStatus_label->setGeometry(QRect(130, 20, 421, 16));
        groupBox_3 = new QGroupBox(centralWidget);
        groupBox_3->setObjectName(QString::fromUtf8("groupBox_3"));
        groupBox_3->setGeometry(QRect(160, 70, 351, 151));
        m_pVersion_combo = new QComboBox(groupBox_3);
        m_pVersion_combo->setObjectName(QString::fromUtf8("m_pVersion_combo"));
        m_pVersion_combo->setGeometry(QRect(10, 40, 161, 22));
        label_2 = new QLabel(groupBox_3);
        label_2->setObjectName(QString::fromUtf8("label_2"));
        label_2->setGeometry(QRect(10, 20, 46, 14));
        m_pFile_combo = new QComboBox(groupBox_3);
        m_pFile_combo->setObjectName(QString::fromUtf8("m_pFile_combo"));
        m_pFile_combo->setGeometry(QRect(180, 40, 161, 22));
        label_3 = new QLabel(groupBox_3);
        label_3->setObjectName(QString::fromUtf8("label_3"));
        label_3->setGeometry(QRect(180, 20, 46, 14));
        m_pChange_button = new QPushButton(groupBox_3);
        m_pChange_button->setObjectName(QString::fromUtf8("m_pChange_button"));
        m_pChange_button->setGeometry(QRect(10, 120, 161, 23));
        m_pChange_button->setCheckable(true);
        m_pApply_button = new QPushButton(groupBox_3);
        m_pApply_button->setObjectName(QString::fromUtf8("m_pApply_button"));
        m_pApply_button->setGeometry(QRect(180, 120, 161, 23));
        m_pStatus_label = new QLabel(centralWidget);
        m_pStatus_label->setObjectName(QString::fromUtf8("m_pStatus_label"));
        m_pStatus_label->setGeometry(QRect(20, 230, 491, 31));
        ctrlClass->setCentralWidget(centralWidget);
        statusBar = new QStatusBar(ctrlClass);
        statusBar->setObjectName(QString::fromUtf8("statusBar"));
        ctrlClass->setStatusBar(statusBar);

        retranslateUi(ctrlClass);

        QMetaObject::connectSlotsByName(ctrlClass);
    } // setupUi

    void retranslateUi(QMainWindow *ctrlClass)
    {
        ctrlClass->setWindowTitle(QApplication::translate("ctrlClass", "ctrl", 0, QApplication::UnicodeUTF8));
        groupBox->setTitle(QApplication::translate("ctrlClass", "Control", 0, QApplication::UnicodeUTF8));
        m_pStart_button->setText(QApplication::translate("ctrlClass", "Start", 0, QApplication::UnicodeUTF8));
        m_pStop_button->setText(QApplication::translate("ctrlClass", "Stop", 0, QApplication::UnicodeUTF8));
        m_pRestart_button->setText(QApplication::translate("ctrlClass", "Restart", 0, QApplication::UnicodeUTF8));
        groupBox_2->setTitle(QApplication::translate("ctrlClass", "Status", 0, QApplication::UnicodeUTF8));
        groupBox_3->setTitle(QApplication::translate("ctrlClass", "Configuration", 0, QApplication::UnicodeUTF8));
        label_2->setText(QApplication::translate("ctrlClass", "Version", 0, QApplication::UnicodeUTF8));
        label_3->setText(QApplication::translate("ctrlClass", "File", 0, QApplication::UnicodeUTF8));
        m_pChange_button->setText(QApplication::translate("ctrlClass", "Change", 0, QApplication::UnicodeUTF8));
        m_pApply_button->setText(QApplication::translate("ctrlClass", "Apply", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class ctrlClass: public Ui_ctrlClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_CTRL_H
