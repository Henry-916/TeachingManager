#ifndef DATABASE_H
#define DATABASE_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QMap>

class Database : public QObject
{
    Q_OBJECT

public:
    bool hasAdmin() const;
    bool isCurrentUserAdmin(int userId);

    static Database& getInstance();
    bool connect();
    bool isConnected() const;

    // 通用操作（针对单主键表）
    bool executeInsert(const QString& table, const QVariantMap& data);
    bool executeUpdate(const QString& table, int id, const QVariantMap& data);
    bool executeDelete(const QString& table, int id);
    QList<QMap<QString, QVariant>> executeSelect(const QString& table,
                                                 const QString& condition = "");

    // 复合主键表的特殊操作
    bool updateTeaching(int teacherId, int courseId, const QString& semester,
                        const QVariantMap& data);
    bool deleteTeaching(int teacherId, int courseId, const QString& semester);
    bool updateEnrollment(int studentId, int teacherId, int courseId,
                          const QString& semester, const QVariantMap& data);
    bool deleteEnrollment(int studentId, int teacherId, int courseId,
                          const QString& semester);

    // 特殊查询
    QList<QMap<QString, QVariant>> getTeachings();
    QList<QMap<QString, QVariant>> getEnrollments();
    QList<QMap<QString, QVariant>> getUsers();

    // 用户管理
    bool addUser(const QString& username, const QString& password, int role,
                 const QVariant& studentId, const QVariant& teacherId);
    bool updateUser(int userId, const QString& username, const QString& password,
                    int role, const QVariant& studentId, const QVariant& teacherId);
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
    bool enableForeignKeys();

    // 主键映射
    QMap<QString, QString> m_primaryKeys;
};

#endif // DATABASE_H
