#ifndef STUDENTWINDOW_H
#define STUDENTWINDOW_H

#include "basewindow.h"
#include <QTableWidget>
#include <QLabel>
#include <QGroupBox>
#include <QLineEdit>
#include <QPushButton>

class StudentWindow : public BaseWindow
{
    Q_OBJECT

public:
    explicit StudentWindow(const User &user, QWidget *parent = nullptr);
    ~StudentWindow();

private slots:
    void onChangePassword();

private:
    void setupUI() override;
    void loadData() override;

    void loadStudentInfo();
    void loadEnrollments();

    int m_studentId;

    // UI组件
    QTabWidget* tabWidget;
    QLabel* infoLabel;
    QTableWidget* infoTable;
    QTableWidget* myEnrollmentsTable;
};

#endif // STUDENTWINDOW_H
