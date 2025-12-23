#ifndef CONFIGMANAGER_H
#define CONFIGMANAGER_H

#include <QObject>
#include <QSettings>
#include <QString>
#include <QDialog>
#include <QLineEdit>
#include <QSpinBox>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>

struct DatabaseConfig {
    QString host = "localhost";
    QString database = "teaching_manager";
    QString username = "root";
    QString password = "123456";
    int port = 3306;
};

// 数据库配置对话框（内部类）
class DatabaseConfigDialog : public QDialog
{
    Q_OBJECT

public:
    explicit DatabaseConfigDialog(const DatabaseConfig& currentConfig, QWidget *parent = nullptr);
    DatabaseConfig getConfig() const;

private:
    QLineEdit *hostEdit;
    QSpinBox *portSpinBox;
    QLineEdit *usernameEdit;
    QLineEdit *passwordEdit;
};

class ConfigManager : public QObject
{
    Q_OBJECT

public:
    static ConfigManager& getInstance();

    DatabaseConfig loadDatabaseConfig();
    void saveDatabaseConfig(const DatabaseConfig& config);

    // 显示配置对话框
    bool showConfigDialog(DatabaseConfig& config);

    // 检查配置是否存在
    bool hasConfig() const;

private:
    explicit ConfigManager(QObject *parent = nullptr);
    ConfigManager(const ConfigManager&) = delete;
    ConfigManager& operator=(const ConfigManager&) = delete;

    QString m_configFile;
};

#endif // CONFIGMANAGER_H
