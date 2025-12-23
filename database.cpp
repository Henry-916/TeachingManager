#include "database.h"
#include <QMessageBox>
#include <QApplication>
#include <QDebug>
#include <QDir>
#include <QSqlRecord>
#include <QElapsedTimer>
#include <QSettings>
#include <QFileInfo>

Database::Database(QObject *parent) : QObject(parent)
{
    // 初始化主键映射
    m_primaryKeys["students"] = "student_id";
    m_primaryKeys["teachers"] = "teacher_id";
    m_primaryKeys["courses"] = "course_id";
    m_primaryKeys["users"] = "user_id";
    m_primaryKeys["teachings"] = "id";
    m_primaryKeys["enrollments"] = "id";

    // 默认连接参数
    m_host = "localhost";
    m_database = "teaching_manager";
    m_username = "root";
    m_password = "123456";
    m_port = 3306;
}

Database& Database::getInstance()
{
    static Database instance;
    return instance;
}

bool Database::connect(const QString& host, const QString& database,
                       const QString& username, const QString& password, int port)
{
    // 保存连接参数
    m_host = host;
    m_database = database;
    m_username = username;
    m_password = password;
    m_port = port;

    // 1. 先尝试直接连接MySQL服务器（不指定数据库）
    QSqlDatabase tempDb = QSqlDatabase::addDatabase("QMYSQL", "temp_connection");
    tempDb.setHostName(m_host);
    tempDb.setPort(m_port);
    tempDb.setUserName(m_username);
    tempDb.setPassword(m_password);

    if (!tempDb.open()) {
        QMessageBox::critical(nullptr, "MySQL连接失败",
                              "无法连接MySQL服务器:\n" + tempDb.lastError().text());
        QSqlDatabase::removeDatabase("temp_connection");
        return false;
    }

    // 2. 检查数据库是否存在，不存在则创建
    QSqlQuery checkDbQuery(tempDb);
    QString checkDbSql = QString("CREATE DATABASE IF NOT EXISTS `%1` "
                                 "CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci").arg(m_database);

    if (!checkDbQuery.exec(checkDbSql)) {
        QMessageBox::critical(nullptr, "数据库错误",
                              "无法创建数据库:\n" + checkDbQuery.lastError().text());
        tempDb.close();
        QSqlDatabase::removeDatabase("temp_connection");
        return false;
    }

    // 3. 关闭临时连接
    tempDb.close();
    QSqlDatabase::removeDatabase("temp_connection");

    // 4. 重新连接指定数据库
    QSqlDatabase db = QSqlDatabase::addDatabase("QMYSQL");
    db.setHostName(m_host);
    db.setPort(m_port);
    db.setDatabaseName(m_database);
    db.setUserName(m_username);
    db.setPassword(m_password);

    if (!db.open()) {
        QMessageBox::critical(nullptr, "数据库连接失败",
                              "无法打开数据库:\n" + db.lastError().text());
        return false;
    }

    // 5. 创建表结构
    createTables();
    qDebug() << "MySQL数据库连接成功! 数据库:" << m_database;
    return true;
}

void Database::createTables()
{
    // MySQL建表语句（注意语法差异）
    QStringList tableQueries = {
        // 用户表
        "CREATE TABLE IF NOT EXISTS users ("
        "user_id INT PRIMARY KEY AUTO_INCREMENT, "
        "account VARCHAR(50) UNIQUE NOT NULL, "
        "password VARCHAR(100) NOT NULL, "
        "role INT NOT NULL DEFAULT 0"
        ") ENGINE=InnoDB DEFAULT CHARSET=utf8mb4",

        // 学生表
        "CREATE TABLE IF NOT EXISTS students ("
        "student_id INT PRIMARY KEY, "
        "name VARCHAR(100) NOT NULL, "
        "age INT, "
        "credits INT DEFAULT 0"
        ") ENGINE=InnoDB DEFAULT CHARSET=utf8mb4",

        // 教师表
        "CREATE TABLE IF NOT EXISTS teachers ("
        "teacher_id INT PRIMARY KEY, "
        "name VARCHAR(100) NOT NULL, "
        "age INT"
        ") ENGINE=InnoDB DEFAULT CHARSET=utf8mb4",

        // 课程表
        "CREATE TABLE IF NOT EXISTS courses ("
        "course_id INT PRIMARY KEY, "
        "name VARCHAR(200) NOT NULL, "
        "credit DECIMAL(4,1), "
        "semester VARCHAR(20)"
        ") ENGINE=InnoDB DEFAULT CHARSET=utf8mb4",

        // 授课表 - 添加自增主键
        "CREATE TABLE IF NOT EXISTS teachings ("
        "id INT PRIMARY KEY AUTO_INCREMENT, "
        "teacher_id INT, "
        "course_id INT, "
        "class_time VARCHAR(50), "
        "classroom VARCHAR(50), "
        "UNIQUE KEY uk_teacher_course (teacher_id, course_id), "
        "FOREIGN KEY (teacher_id) REFERENCES teachers(teacher_id) ON DELETE CASCADE, "
        "FOREIGN KEY (course_id) REFERENCES courses(course_id) ON DELETE CASCADE"
        ") ENGINE=InnoDB DEFAULT CHARSET=utf8mb4",

        // 选课表 - 添加自增主键
        "CREATE TABLE IF NOT EXISTS enrollments ("
        "id INT PRIMARY KEY AUTO_INCREMENT, "
        "student_id INT, "
        "course_id INT, "
        "score DECIMAL(4,1) DEFAULT 0, "
        "UNIQUE KEY uk_student_course (student_id, course_id), "
        "FOREIGN KEY (student_id) REFERENCES students(student_id) ON DELETE CASCADE, "
        "FOREIGN KEY (course_id) REFERENCES courses(course_id) ON DELETE CASCADE"
        ") ENGINE=InnoDB DEFAULT CHARSET=utf8mb4"
    };

    // 执行建表语句
    QSqlQuery query;
    for (const auto& queryStr : tableQueries) {
        if (!query.exec(queryStr)) {
            qWarning() << "创建表失败:" << query.lastError().text()
                << "\nSQL:" << queryStr;
        }
    }

    // MySQL触发器语法不同
    QStringList triggerQueries = {
        // 防止更新产生多个管理员
        "CREATE TRIGGER IF NOT EXISTS single_admin_update "
        "BEFORE UPDATE ON users "
        "FOR EACH ROW "
        "BEGIN "
        "    IF NEW.role = 2 AND OLD.role != 2 AND "
        "       (SELECT COUNT(*) FROM users WHERE role = 2 AND user_id != OLD.user_id) > 0 THEN "
        "        SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT = '只能有一个管理员'; "
        "    END IF; "
        "END",

        // 插入学生时自动创建用户账户
        "CREATE TRIGGER IF NOT EXISTS create_student_user_trigger "
        "AFTER INSERT ON students "
        "FOR EACH ROW "
        "BEGIN "
        "    INSERT IGNORE INTO users (account, password, role) "
        "    VALUES (NEW.student_id, '123456', 0); "
        "END",

        // 插入教师时自动创建用户账户
        "CREATE TRIGGER IF NOT EXISTS create_teacher_user_trigger "
        "AFTER INSERT ON teachers "
        "FOR EACH ROW "
        "BEGIN "
        "    INSERT IGNORE INTO users (account, password, role) "
        "    VALUES (NEW.teacher_id, '123456', 1); "
        "END"
    };

    // 执行触发器语句
    for (const auto& queryStr : triggerQueries) {
        if (!query.exec(queryStr)) {
            qWarning() << "创建触发器失败:" << query.lastError().text();
        }
    }
}

void Database::saveDatabaseConfig()
{
    QSettings settings("TeachingSystem", "TeachingManager");
    settings.setValue("Database/Host", m_host);
    settings.setValue("Database/Name", m_database);
    settings.setValue("Database/Username", m_username);
    settings.setValue("Database/Password", m_password);
    settings.setValue("Database/Port", m_port);
}

void Database::loadDatabaseConfig()
{
    QSettings settings("TeachingSystem", "TeachingManager");
    m_host = settings.value("Database/Host", "localhost").toString();
    m_database = settings.value("Database/Name", "teaching_manager").toString();
    m_username = settings.value("Database/Username", "root").toString();
    m_password = settings.value("Database/Password", "").toString();
    m_port = settings.value("Database/Port", 3306).toInt();
}

bool Database::isConnected() const
{
    return QSqlDatabase::database().isOpen();
}

// 通用CRUD操作
bool Database::executeInsert(const QString& table, const QVariantMap& data)
{
    if (data.isEmpty()) return false;

    QStringList fields, placeholders;
    for (auto it = data.begin(); it != data.end(); ++it) {
        fields << QString("`%1`").arg(it.key());  // MySQL使用反引号
        placeholders << ":" + it.key();
    }

    QString sql = QString("INSERT INTO `%1` (%2) VALUES (%3)")
                      .arg(table)
                      .arg(fields.join(", "))
                      .arg(placeholders.join(", "));

    QSqlQuery query;
    query.prepare(sql);

    for (auto it = data.begin(); it != data.end(); ++it) {
        query.bindValue(":" + it.key(), it.value());
    }

    return query.exec();
}

bool Database::executeUpdate(const QString& table, int id, const QVariantMap& data)
{
    if (data.isEmpty()) return false;

    QStringList updates;
    for (auto it = data.begin(); it != data.end(); ++it) {
        updates << QString("`%1` = :%2").arg(it.key()).arg(it.key());
    }

    QString idField = m_primaryKeys.value(table, "id");

    QString sql = QString("UPDATE `%1` SET %2 WHERE `%3` = :id")
                      .arg(table)
                      .arg(updates.join(", "))
                      .arg(idField);

    QSqlQuery query;
    query.prepare(sql);
    query.bindValue(":id", id);

    for (auto it = data.begin(); it != data.end(); ++it) {
        query.bindValue(":" + it.key(), it.value());
    }

    return query.exec();
}

bool Database::executeDelete(const QString& table, int id)
{
    // 获取主键字段名
    QString idField = m_primaryKeys.value(table, "id");

    QString sql = QString("DELETE FROM %1 WHERE %2 = :id")
                      .arg(table)
                      .arg(idField);

    QSqlQuery query;
    query.prepare(sql);
    query.bindValue(":id", id);

    return query.exec();
}

QList<QMap<QString, QVariant>> Database::executeSelect(const QString& table,
                                                       const QString& condition)
{
    QList<QMap<QString, QVariant>> result;

    QString fields;
    QString orderBy;

    if (table == "students") {
        fields = "student_id, name, age, credits";
        orderBy = "student_id";
    } else if (table == "teachers") {
        fields = "teacher_id, name, age";
        orderBy = "teacher_id";
    } else if (table == "courses") {
        fields = "course_id, name, credit, semester";  // 添加 semester 字段
        orderBy = "course_id";
    } else if (table == "users") {
        fields = "user_id, account, password, role";
        orderBy = "user_id";
    } else {
        fields = "*";
        orderBy = "";
    }

    QString sql = QString("SELECT %1 FROM %2").arg(fields).arg(table);
    if (!condition.isEmpty()) {
        sql += " WHERE " + condition;
    }

    // 添加排序，确保数据顺序一致
    if (!orderBy.isEmpty()) {
        sql += " ORDER BY " + orderBy;
    }

    QSqlQuery query(sql);

    while (query.next()) {
        QMap<QString, QVariant> row;
        QSqlRecord record = query.record();
        for (int i = 0; i < record.count(); i++) {
            row[record.fieldName(i)] = query.value(i);
        }
        result.append(row);
    }

    return result;
}

// 特殊查询
QList<QMap<QString, QVariant>> Database::getTeachings()
{
    QList<QMap<QString, QVariant>> result;

    // 表头顺序：{"教师工号", "教师姓名", "课程ID", "课程名称", "学期", "上课时间", "教室"}
    QString sql = "SELECT "
                  "t.teacher_id, "           // 教师工号 - 列0
                  "te.name as teacher_name, " // 教师姓名 - 列1
                  "t.course_id, "            // 课程ID - 列2
                  "c.name as course_name, "  // 课程名称 - 列3
                  "c.semester, "             // 学期 - 列4
                  "t.class_time, "           // 上课时间 - 列5
                  "t.classroom "             // 教室 - 列6
                  "FROM teachings t "
                  "LEFT JOIN teachers te ON t.teacher_id = te.teacher_id "
                  "LEFT JOIN courses c ON t.course_id = c.course_id "
                  // 排序规则：先按照学期逆序，然后按照课程ID顺序，再按照教师ID顺序
                  "ORDER BY c.semester DESC, t.course_id ASC, t.teacher_id ASC";

    QSqlQuery query(sql);

    while (query.next()) {
        QMap<QString, QVariant> row;
        QSqlRecord record = query.record();
        for (int i = 0; i < record.count(); i++) {
            row[record.fieldName(i)] = query.value(i);
        }
        result.append(row);
    }

    return result;
}

QList<QMap<QString, QVariant>> Database::getEnrollments()
{
    QList<QMap<QString, QVariant>> result;

    // 表头顺序：{"学生学号", "学生姓名", "课程ID", "课程名称", "学期", "成绩"}
    QString sql = "SELECT "
                  "e.student_id, "           // 学生学号 - 列0
                  "s.name as student_name, " // 学生姓名 - 列1
                  "e.course_id, "            // 课程ID - 列2
                  "c.name as course_name, "  // 课程名称 - 列3
                  "c.semester, "             // 学期 - 列4
                  "e.score "                 // 成绩 - 列5
                  "FROM enrollments e "
                  "LEFT JOIN students s ON e.student_id = s.student_id "
                  "LEFT JOIN courses c ON e.course_id = c.course_id "
                  // 排序规则：先按照学期逆序，然后按照课程ID顺序，再按照学生ID顺序
                  "ORDER BY c.semester DESC, e.course_id ASC, e.student_id ASC";

    QSqlQuery query(sql);

    while (query.next()) {
        QMap<QString, QVariant> row;
        QSqlRecord record = query.record();
        for (int i = 0; i < record.count(); i++) {
            row[record.fieldName(i)] = query.value(i);
        }
        result.append(row);
    }

    return result;
}

QList<QMap<QString, QVariant>> Database::getUsers()
{
    QList<QMap<QString, QVariant>> result;

    QString sql = "SELECT user_id, account, password, role FROM users ORDER BY user_id";

    QSqlQuery query(sql);

    while (query.next()) {
        QMap<QString, QVariant> row;
        QSqlRecord record = query.record();
        for (int i = 0; i < record.count(); i++) {
            row[record.fieldName(i)] = query.value(i);
        }
        result.append(row);
    }

    return result;
}

// 用户管理
bool Database::addUser(const QString& account, const QString& password, int role)
{
    // 检查是否已存在管理员（应用层检查，提供友好提示）
    if (role == 2) { // 管理员角色
        QSqlQuery checkAdminQuery("SELECT COUNT(*) FROM users WHERE role = 2");
        if (checkAdminQuery.exec() && checkAdminQuery.next()) {
            if (checkAdminQuery.value(0).toInt() > 0) {
                qWarning() << "添加用户失败：系统中已存在管理员，不能创建新的管理员";
                return false;
            }
        }
    }

    // 检查用户名是否已存在
    QSqlQuery checkUserQuery;
    checkUserQuery.prepare("SELECT COUNT(*) FROM users WHERE account = ?");
    checkUserQuery.addBindValue(account);
    if (checkUserQuery.exec() && checkUserQuery.next() && checkUserQuery.value(0).toInt() > 0) {
        qWarning() << "添加用户失败：账号已存在：" << account;
        return false;
    }

    // 对于学生和教师，验证对应的ID是否存在
    if (role == 0) { // 学生
        bool ok;
        int studentId = account.toInt(&ok);
        if (!ok) {
            qWarning() << "添加用户失败：学生账号必须是数字";
            return false;
        }

        QSqlQuery checkStudentQuery;
        checkStudentQuery.prepare("SELECT COUNT(*) FROM students WHERE student_id = ?");
        checkStudentQuery.addBindValue(studentId);
        if (checkStudentQuery.exec() && checkStudentQuery.next() &&
            checkStudentQuery.value(0).toInt() == 0) {
            qWarning() << "添加用户失败：学生表中不存在该学生ID:" << studentId;
            return false;
        }
    } else if (role == 1) { // 教师
        bool ok;
        int teacherId = account.toInt(&ok);
        if (!ok) {
            qWarning() << "添加用户失败：教师账号必须是数字";
            return false;
        }

        QSqlQuery checkTeacherQuery;
        checkTeacherQuery.prepare("SELECT COUNT(*) FROM teachers WHERE teacher_id = ?");
        checkTeacherQuery.addBindValue(teacherId);
        if (checkTeacherQuery.exec() && checkTeacherQuery.next() &&
            checkTeacherQuery.value(0).toInt() == 0) {
            qWarning() << "添加用户失败：教师表中不存在该教师ID:" << teacherId;
            return false;
        }
    }

    QVariantMap data;
    data["account"] = account;
    data["password"] = password;
    data["role"] = role;

    bool success = executeInsert("users", data);
    if (!success) {
        qWarning() << "添加用户失败：" << QSqlDatabase::database().lastError().text();
    }
    return success;
}

bool Database::updateUser(int userId, const QString& account, const QString& password,
                          int role)
{
    // 先获取用户当前的角色
    QSqlQuery getCurrentRoleQuery;
    getCurrentRoleQuery.prepare("SELECT role FROM users WHERE user_id = ?");
    getCurrentRoleQuery.addBindValue(userId);
    int currentRole = -1;
    if (getCurrentRoleQuery.exec() && getCurrentRoleQuery.next()) {
        currentRole = getCurrentRoleQuery.value(0).toInt();
    } else {
        qWarning() << "更新用户失败：找不到用户ID:" << userId;
        return false;
    }

    // 检查是否要设置为管理员（应用层检查）
    if (role == 2 && currentRole != 2) {
        QSqlQuery checkAdminQuery("SELECT COUNT(*) FROM users WHERE role = 2");
        if (checkAdminQuery.exec() && checkAdminQuery.next()) {
            if (checkAdminQuery.value(0).toInt() > 0) {
                qWarning() << "更新用户失败：系统中已存在管理员，不能设置新的管理员";
                return false;
            }
        }
    }

    // 检查账号是否已存在（排除当前用户）
    QSqlQuery checkUserQuery;
    checkUserQuery.prepare("SELECT COUNT(*) FROM users WHERE account = ? AND user_id != ?");
    checkUserQuery.addBindValue(account);
    checkUserQuery.addBindValue(userId);
    if (checkUserQuery.exec() && checkUserQuery.next() && checkUserQuery.value(0).toInt() > 0) {
        qWarning() << "更新用户失败：账号已存在：" << account;
        return false;
    }

    // 对于学生和教师，验证对应的ID是否存在
    if (role == 0) { // 学生
        bool ok;
        int studentId = account.toInt(&ok);
        if (!ok) {
            qWarning() << "更新用户失败：学生账号必须是数字";
            return false;
        }

        QSqlQuery checkStudentQuery;
        checkStudentQuery.prepare("SELECT COUNT(*) FROM students WHERE student_id = ?");
        checkStudentQuery.addBindValue(studentId);
        if (checkStudentQuery.exec() && checkStudentQuery.next() &&
            checkStudentQuery.value(0).toInt() == 0) {
            qWarning() << "更新用户失败：学生表中不存在该学生ID:" << studentId;
            return false;
        }
    } else if (role == 1) { // 教师
        bool ok;
        int teacherId = account.toInt(&ok);
        if (!ok) {
            qWarning() << "更新用户失败：教师账号必须是数字";
            return false;
        }

        QSqlQuery checkTeacherQuery;
        checkTeacherQuery.prepare("SELECT COUNT(*) FROM teachers WHERE teacher_id = ?");
        checkTeacherQuery.addBindValue(teacherId);
        if (checkTeacherQuery.exec() && checkTeacherQuery.next() &&
            checkTeacherQuery.value(0).toInt() == 0) {
            qWarning() << "更新用户失败：教师表中不存在该教师ID:" << teacherId;
            return false;
        }
    }

    QVariantMap data;
    data["account"] = account;
    data["password"] = password;
    data["role"] = role;

    bool success = executeUpdate("users", userId, data);
    if (!success) {
        qWarning() << "更新用户失败：" << QSqlDatabase::database().lastError().text();
    }
    return success;
}

bool Database::deleteUser(int userId)
{
    // 先检查要删除的用户是否是管理员
    QSqlQuery checkRoleQuery;
    checkRoleQuery.prepare("SELECT role FROM users WHERE user_id = ?");
    checkRoleQuery.addBindValue(userId);
    if (checkRoleQuery.exec() && checkRoleQuery.next()) {
        int role = checkRoleQuery.value(0).toInt();
        if (role == 2) {
            qWarning() << "删除用户失败：不能删除管理员账户";
            return false;
        }
    }

    return executeDelete("users", userId);
}

bool Database::checkUsernameExists(const QString& account, int excludeUserId)
{
    QString condition = QString("account = '%1'").arg(account);
    if (excludeUserId >= 0) {
        condition += QString(" AND user_id != %1").arg(excludeUserId);
    }

    auto users = executeSelect("users", condition);
    return !users.isEmpty();
}

bool Database::validateUser(const QString& account, const QString& password,
                            int role, int& userId)
{
    QString condition = QString("account = '%1' AND password = '%2' AND role = %3")
    .arg(account).arg(password).arg(role);

    auto users = executeSelect("users", condition);

    if (!users.isEmpty()) {
        userId = users.first()["user_id"].toInt();
        return true;
    }

    return false;
}

QString Database::executeSQL(const QString& sql)
{
    QElapsedTimer timer;
    timer.start();

    QStringList sqlStatements;
    QString currentStatement;

    for (int i = 0; i < sql.length(); i++) {
        QChar ch = sql[i];
        currentStatement += ch;

        if (ch == ';' && (i == 0 || sql[i-1] != '\'')) {
            sqlStatements.append(currentStatement.trimmed());
            currentStatement.clear();
        }
    }

    if (!currentStatement.trimmed().isEmpty()) {
        sqlStatements.append(currentStatement.trimmed());
    }

    int successCount = 0;
    int failCount = 0;
    QString allResults;

    for (int i = 0; i < sqlStatements.size(); i++) {
        QString statement = sqlStatements[i].trimmed();

        if (statement.isEmpty() || statement.startsWith("--")) {
            continue;
        }

        allResults += QString("SQL: %1\n").arg(statement);

        QSqlQuery query;
        bool success = query.exec(statement);

        if (success) {
            if (query.isSelect()) {
                QString result;
                int rowCount = 0;

                QStringList headers;
                QSqlRecord record = query.record();
                for (int j = 0; j < record.count(); j++) {
                    headers.append(record.fieldName(j));
                }

                result += headers.join("\t") + "\n";
                result += QString("-").repeated(headers.join("").length() + headers.count() * 3) + "\n";

                while (query.next()) {
                    rowCount++;
                    for (int j = 0; j < record.count(); j++) {
                        result += query.value(j).toString() + "\t";
                    }
                    result += "\n";

                    if (rowCount > 1000) {
                        result += QString("\n... 已显示1000行，总共 %1 行\n").arg(rowCount);
                        while (query.next()) { rowCount++; }
                        break;
                    }
                }

                result += QString("\n共查询到 %1 行数据\n").arg(rowCount);
                allResults += "结果:\n" + result;
            } else {
                int affectedRows = query.numRowsAffected();
                if (affectedRows >= 0) {
                    allResults += QString("执行成功，影响 %1 行\n").arg(affectedRows);
                } else {
                    allResults += "执行成功\n";
                }
            }
            successCount++;
        } else {
            allResults += QString("执行失败: %1\n").arg(query.lastError().text());
            failCount++;
        }

        allResults += "\n";
    }

    qint64 elapsed = timer.elapsed();
    allResults += QString("\n执行完成: %1 成功, %2 失败 | 耗时: %3 毫秒")
                      .arg(successCount).arg(failCount).arg(elapsed);

    return allResults;
}

// 对于复合主键的表，提供专门的函数
bool Database::updateTeaching(int teacherId, int courseId, const QVariantMap& data)
{
    if (data.isEmpty()) return false;

    QStringList updates;
    for (auto it = data.begin(); it != data.end(); ++it) {
        updates << it.key() + " = :" + it.key();
    }

    QString sql = QString("UPDATE teachings SET %1 WHERE teacher_id = :teacher_id "
                          "AND course_id = :course_id")
                      .arg(updates.join(", "));

    QSqlQuery query;
    query.prepare(sql);
    query.bindValue(":teacher_id", teacherId);
    query.bindValue(":course_id", courseId);

    for (auto it = data.begin(); it != data.end(); ++it) {
        query.bindValue(":" + it.key(), it.value());
    }

    return query.exec();
}


bool Database::deleteTeaching(int teacherId, int courseId)
{
    QString sql = "DELETE FROM teachings WHERE teacher_id = :teacher_id "
                  "AND course_id = :course_id";

    QSqlQuery query;
    query.prepare(sql);
    query.bindValue(":teacher_id", teacherId);
    query.bindValue(":course_id", courseId);

    return query.exec();
}

bool Database::updateEnrollment(int studentId, int courseId, const QVariantMap& data)
{
    if (data.isEmpty()) return false;

    QStringList updates;
    for (auto it = data.begin(); it != data.end(); ++it) {
        updates << it.key() + " = :" + it.key();
    }

    QString sql = QString("UPDATE enrollments SET %1 WHERE student_id = :student_id "
                          "AND course_id = :course_id")
                      .arg(updates.join(", "));

    QSqlQuery query;
    query.prepare(sql);
    query.bindValue(":student_id", studentId);
    query.bindValue(":course_id", courseId);

    for (auto it = data.begin(); it != data.end(); ++it) {
        query.bindValue(":" + it.key(), it.value());
    }

    return query.exec();
}

bool Database::deleteEnrollment(int studentId, int courseId)
{
    QString sql = "DELETE FROM enrollments WHERE student_id = :student_id "
                  "AND course_id = :course_id";

    QSqlQuery query;
    query.prepare(sql);
    query.bindValue(":student_id", studentId);
    query.bindValue(":course_id", courseId);

    return query.exec();
}
