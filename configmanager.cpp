#include "configmanager.h"
#include <QDir>
#include <QCoreApplication>
#include <QDebug>
#include <QMessageBox>

// DatabaseConfigDialog 实现
DatabaseConfigDialog::DatabaseConfigDialog(const DatabaseConfig& currentConfig, QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("数据库配置");
    setFixedSize(400, 300);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(10);

    // 表单布局
    QGridLayout *formLayout = new QGridLayout();
    formLayout->setSpacing(8);

    // 主机
    formLayout->addWidget(new QLabel("主机地址:"), 0, 0);
    hostEdit = new QLineEdit(currentConfig.host);
    hostEdit->setPlaceholderText("例如: localhost 或 127.0.0.1");
    formLayout->addWidget(hostEdit, 0, 1);

    // 端口
    formLayout->addWidget(new QLabel("端口号:"), 1, 0);
    portSpinBox = new QSpinBox();
    portSpinBox->setRange(1, 65535);
    portSpinBox->setValue(currentConfig.port);
    portSpinBox->setToolTip("MySQL默认端口为3306");
    formLayout->addWidget(portSpinBox, 1, 1);

    // 用户名
    formLayout->addWidget(new QLabel("用户名:"), 2, 0);
    usernameEdit = new QLineEdit(currentConfig.username);
    usernameEdit->setPlaceholderText("MySQL用户名");
    formLayout->addWidget(usernameEdit, 2, 1);

    // 密码
    formLayout->addWidget(new QLabel("密码:"), 3, 0);
    passwordEdit = new QLineEdit();
    passwordEdit->setEchoMode(QLineEdit::Password);
    passwordEdit->setPlaceholderText("MySQL密码");
    formLayout->addWidget(passwordEdit, 3, 1);

    mainLayout->addLayout(formLayout);

    // 按钮
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    QPushButton *okButton = new QPushButton("确定");
    QPushButton *cancelButton = new QPushButton("取消");
    QPushButton *testButton = new QPushButton("测试连接");

    buttonLayout->addWidget(testButton);
    buttonLayout->addStretch();
    buttonLayout->addWidget(okButton);
    buttonLayout->addWidget(cancelButton);

    mainLayout->addLayout(buttonLayout);

    // 连接信号槽
    connect(okButton, &QPushButton::clicked, this, &QDialog::accept);
    connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);
    connect(testButton, &QPushButton::clicked, [this]() {
        DatabaseConfig testConfig = this->getConfig();
        // 这里可以添加测试连接的功能
        QMessageBox::information(this, "测试连接",
                                 QString("将测试连接:\n主机: %1\n端口: %2\n用户名: %3\n数据库: teaching_manager")
                                     .arg(testConfig.host)
                                     .arg(testConfig.port)
                                     .arg(testConfig.username));
    });
}

DatabaseConfig DatabaseConfigDialog::getConfig() const
{
    DatabaseConfig config;
    config.host = hostEdit->text().trimmed();
    config.port = portSpinBox->value();
    config.username = usernameEdit->text().trimmed();
    config.password = passwordEdit->text();
    config.database = "teaching_manager";  // 固定
    return config;
}

// ConfigManager 实现
ConfigManager& ConfigManager::getInstance()
{
    static ConfigManager instance;
    return instance;
}

ConfigManager::ConfigManager(QObject *parent)
    : QObject(parent)
{
    // 设置配置文件路径
    QString appDir = QCoreApplication::applicationDirPath();
    m_configFile = appDir + "/config.ini";
    qDebug() << "配置文件路径:" << m_configFile;
}

bool ConfigManager::hasConfig() const
{
    QSettings settings(m_configFile, QSettings::IniFormat);
    return settings.contains("Database/Host");
}

DatabaseConfig ConfigManager::loadDatabaseConfig()
{
    DatabaseConfig config;

    QSettings settings(m_configFile, QSettings::IniFormat);

    // 从配置文件加载，如果不存在则使用默认值
    config.host = settings.value("Database/Host", "localhost").toString();
    config.database = settings.value("Database/Database", "teaching_manager").toString();
    config.username = settings.value("Database/Username", "root").toString();
    config.password = settings.value("Database/Password", "123456").toString();
    config.port = settings.value("Database/Port", 3306).toInt();

    return config;
}

void ConfigManager::saveDatabaseConfig(const DatabaseConfig& config)
{
    QSettings settings(m_configFile, QSettings::IniFormat);

    settings.setValue("Database/Host", config.host);
    settings.setValue("Database/Database", config.database);
    settings.setValue("Database/Username", config.username);
    settings.setValue("Database/Password", config.password);
    settings.setValue("Database/Port", config.port);

    settings.sync(); // 立即写入磁盘

    qDebug() << "数据库配置已保存到配置文件:";
    qDebug() << "  主机:" << config.host;
    qDebug() << "  端口:" << config.port;
    qDebug() << "  用户名:" << config.username;
    qDebug() << "  数据库:" << config.database;
}

bool ConfigManager::showConfigDialog(DatabaseConfig& config)
{
    DatabaseConfigDialog dialog(config);

    if (dialog.exec() == QDialog::Accepted) {
        config = dialog.getConfig();

        // 验证配置是否有效
        if (config.host.isEmpty() || config.username.isEmpty()) {
            QMessageBox::warning(nullptr, "配置错误",
                                 "主机地址和用户名不能为空！");
            return false;
        }

        return true;
    }

    return false; // 用户取消了对话框
}
