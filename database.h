#ifndef DATABASE_H
#define DATABASE_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QMap>
#include <QSettings>

class Database : public QObject
{
    Q_OBJECT

public:
    static Database& getInstance();
    bool connect(const QString& host = "localhost",
                 const QString& database = "teaching_manager",
                 const QString& username = "root",
                 const QString& password = "123456",
                 int port = 3306);
    bool isConnected() const;

    // 数据库配置
    void saveDatabaseConfig();
    void loadDatabaseConfig();

    // 通用操作
    bool executeInsert(const QString& table, const QVariantMap& data);
    bool executeUpdate(const QString& table, int id, const QVariantMap& data);
    bool executeDelete(const QString& table, int id);
    QList<QMap<QString, QVariant>> executeSelect(const QString& table,
                                                 const QString& condition = "");

    // 复合主键表的特殊操作
    bool updateTeaching(int teacherId, int courseId, const QVariantMap& data);
    bool deleteTeaching(int teacherId, int courseId);
    bool updateEnrollment(int studentId, int courseId, const QVariantMap& data);
    bool deleteEnrollment(int studentId, int courseId);

    // 特殊查询
    QList<QMap<QString, QVariant>> getTeachings();
    QList<QMap<QString, QVariant>> getEnrollments();
    QList<QMap<QString, QVariant>> getUsers();

    // 用户管理
    bool addUser(const QString& account, const QString& password, int role);
    bool updateUser(int userId, const QString& account, const QString& password, int role);
    bool deleteUser(int userId);
    bool checkUsernameExists(const QString& username, int excludeUserId = -1);

    // SQL执行
    QString executeSQL(const QString& sql);

    // 登录验证
    bool validateUser(const QString& username, const QString& password,
                      int role, int& userId);

private:
    explicit Database(QObject *parent = nullptr);
    Database(const Database&) = delete;
    Database& operator=(const Database&) = delete;

    void createTables();
    bool createDatabaseIfNotExists();

    // 数据库连接信息
    QString m_host;
    QString m_database;
    QString m_username;
    QString m_password;
    int m_port;

    // 主键映射
    QMap<QString, QString> m_primaryKeys;
};

#endif // DATABASE_H
