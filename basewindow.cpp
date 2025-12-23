#include "basewindow.h"
#include <QMessageBox>
#include <QHeaderView>
#include <QDebug>
#include <QHBoxLayout>
#include <QLabel>
#include <QSpacerItem>
#include <QStyle>
#include <QFont>
#include <QLineEdit>

BaseWindow::BaseWindow(const User &user, QWidget *parent)
    : QMainWindow(parent)
    , db(Database::getInstance())
    , m_currentUser(user)
    , topBarWidget(nullptr)
    , logoutButton(nullptr)
{
    // 设置窗口基本属性
    setMinimumSize(800, 600);
    setWindowTitle(QString("教学管理系统 - %1 [%2]")
                       .arg(m_currentUser.getUsername())
                       .arg(m_currentUser.getRoleString()));
}

BaseWindow::~BaseWindow()
{
    // 基础窗口的清理工作
}

void BaseWindow::setupTopBar()
{
    // 创建顶部栏容器
    topBarWidget = new QWidget(this);
    topBarWidget->setFixedHeight(50);

    // 创建水平布局
    QHBoxLayout* topLayout = new QHBoxLayout(topBarWidget);
    topLayout->setContentsMargins(10, 0, 10, 0);
    topLayout->setSpacing(10);

    // 左侧：用户信息标签
    QLabel* userInfoLabel = new QLabel(topBarWidget);
    userInfoLabel->setText(QString("当前用户: %1 (%2)")
                               .arg(m_currentUser.getUsername())
                               .arg(m_currentUser.getRoleString()));
    userInfoLabel->setStyleSheet(
        "font-size: 14px;"
        "font-weight: bold;"
        "color: #f0f0f0;"
        );

    // 弹簧（将按钮推到右侧）
    QSpacerItem* spacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

    // 右侧：退出登录按钮
    logoutButton = new QPushButton("退出登录", topBarWidget);
    logoutButton->setFixedSize(100, 30);
    logoutButton->setStyleSheet(
        "QPushButton {"
        "    background-color: #DC143C;"
        "    color: white;"
        "    border: none;"
        "    border-radius: 4px;"
        "    font-size: 13px;"
        "    font-weight: bold;"
        "    padding: 5px 10px;"
        "}"
        "QPushButton:hover {"
        "    background-color: #FF0000;"
        "}"
        "QPushButton:pressed {"
        "    background-color: #b71c1c;"
        "}"
        );

    // 添加到布局
    topLayout->addWidget(userInfoLabel);
    topLayout->addItem(spacer);
    topLayout->addWidget(logoutButton);

    // 连接信号槽
    connect(logoutButton, &QPushButton::clicked, this, &BaseWindow::onLogoutClicked);
}

void BaseWindow::onLogoutClicked()
{
    int result = QMessageBox::question(this, "退出登录",
                                       "确定要退出登录吗？",
                                       QMessageBox::Yes | QMessageBox::No,
                                       QMessageBox::No);

    if (result == QMessageBox::Yes) {
        qDebug() << "用户" << m_currentUser.getUsername() << "退出登录";

        // 发出退出信号
        emit logoutRequested();

        // 关闭当前窗口
        this->close();
    }
}

void BaseWindow::setupTable(QTableWidget* table, const QStringList& headers)
{
    table->setColumnCount(headers.size());
    table->setHorizontalHeaderLabels(headers);
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    table->setAlternatingRowColors(true);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
}

void BaseWindow::loadTableData(QTableWidget* table, const QList<QMap<QString, QVariant>>& data)
{
    table->setRowCount(data.size());

    for (int i = 0; i < data.size(); i++) {
        const auto& rowData = data[i];

        // 获取当前行的列值列表（按数据库查询的字段顺序）
        QList<QVariant> values;
        for (auto it = rowData.begin(); it != rowData.end(); ++it) {
            values.append(it.value());
        }

        // 按照列顺序填充数据
        for (int col = 0; col < qMin(values.size(), table->columnCount()); col++) {
            table->setItem(i, col, new QTableWidgetItem(values[col].toString()));
        }
    }
}

void BaseWindow::changePassword(const QString& currentPassword, const QString& newPassword,
                                const QString& confirmPassword)
{
    // 验证输入
    if (currentPassword.isEmpty()) {
        QMessageBox::warning(this, "输入错误", "请输入当前密码");
        return;
    }

    if (newPassword.isEmpty()) {
        QMessageBox::warning(this, "输入错误", "请输入新密码");
        return;
    }

    if (confirmPassword.isEmpty()) {
        QMessageBox::warning(this, "输入错误", "请确认新密码");
        return;
    }

    if (newPassword != confirmPassword) {
        QMessageBox::warning(this, "修改失败", "新密码和确认密码不一致");
        return;
    }

    // 验证当前密码
    int userId = m_currentUser.getId();
    int role = static_cast<int>(m_currentUser.getRole());
    int dummyUserId;

    if (!db.validateUser(m_currentUser.getUsername(), currentPassword, role, dummyUserId)) {
        QMessageBox::warning(this, "修改失败", "当前密码不正确");
        return;
    }

    // 更新密码
    if (db.updateUser(userId, m_currentUser.getUsername(), newPassword, role)) {
        QMessageBox::information(this, "修改成功", "密码修改成功，请重新登录");
        emit logoutRequested();
    } else {
        QMessageBox::warning(this, "修改失败", "密码修改失败，请稍后重试");
    }
}

// basewindow.cpp - 修改createPasswordChangeGroup函数
QGroupBox* BaseWindow::createPasswordChangeGroup()
{
    passwordGroup = new QGroupBox("修改密码");
    QGridLayout *passwordLayout = new QGridLayout(passwordGroup);

    passwordLayout->addWidget(new QLabel("当前密码:"), 0, 0);
    currentPasswordEdit = new QLineEdit();
    currentPasswordEdit->setEchoMode(QLineEdit::Password);
    currentPasswordEdit->setPlaceholderText("请输入当前密码");
    passwordLayout->addWidget(currentPasswordEdit, 0, 1);

    passwordLayout->addWidget(new QLabel("新密码:"), 1, 0);
    newPasswordEdit = new QLineEdit();
    newPasswordEdit->setEchoMode(QLineEdit::Password);
    newPasswordEdit->setPlaceholderText("请输入新密码（至少6位）");
    passwordLayout->addWidget(newPasswordEdit, 1, 1);

    passwordLayout->addWidget(new QLabel("确认新密码:"), 2, 0);
    confirmPasswordEdit = new QLineEdit();
    confirmPasswordEdit->setEchoMode(QLineEdit::Password);
    confirmPasswordEdit->setPlaceholderText("请再次输入新密码");
    passwordLayout->addWidget(confirmPasswordEdit, 2, 1);

    changePasswordButton = new QPushButton("修改密码");
    changePasswordButton->setStyleSheet("background-color: #228B22; color: white; padding: 5px;");
    passwordLayout->addWidget(changePasswordButton, 3, 0, 1, 2);

    return passwordGroup;
}

void BaseWindow::setupCommonTable(QTableWidget* table, const QStringList& headers)
{
    setupTable(table, headers);
    table->setAlternatingRowColors(true);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
}
