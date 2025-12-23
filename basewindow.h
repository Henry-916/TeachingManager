#ifndef BASEWINDOW_H
#define BASEWINDOW_H

#include <QMainWindow>
#include <QTableWidget>
#include <QLabel>
#include <QPushButton>
#include <QHBoxLayout>
#include <QSpacerItem>
#include <QGroupBox>
#include "User.h"
#include "database.h"

class BaseWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit BaseWindow(const User &user, QWidget *parent = nullptr);
    virtual ~BaseWindow();

signals:
    void logoutRequested();

protected:
    // 公共UI设置函数
    void setupTopBar();  // 新增：设置顶部栏
    void setupTable(QTableWidget* table, const QStringList& headers);
    void loadTableData(QTableWidget* table, const QList<QMap<QString, QVariant>>& data);

    // 虚函数，子类需要实现
    virtual void setupUI() = 0;  // 纯虚函数，必须实现
    virtual void loadData() {}   // 非纯虚函数，有默认实现（空）

    // 公共组件
    Database& db;
    User m_currentUser;

    // 公共UI组件
    QWidget* topBarWidget;      // 新增：顶部栏容器
    QPushButton* logoutButton;  // 新增：退出登录按钮
    QGroupBox* createPasswordChangeGroup();

    void setupCommonTable(QTableWidget* table, const QStringList& headers);
    void loadCommonTableData(QTableWidget* table, const QList<QMap<QString, QVariant>>& data);
    void changePassword(const QString& currentPassword, const QString& newPassword,
                        const QString& confirmPassword);

    // 密码修改相关组件
    QGroupBox* passwordGroup = nullptr;
    QLineEdit* currentPasswordEdit = nullptr;
    QLineEdit* newPasswordEdit = nullptr;
    QLineEdit* confirmPasswordEdit = nullptr;
    QPushButton* changePasswordButton = nullptr;

protected slots:
    virtual void onLogoutClicked();

private:
    // 禁止复制和赋值
    BaseWindow(const BaseWindow&) = delete;
    BaseWindow& operator=(const BaseWindow&) = delete;
};

#endif // BASEWINDOW_H
