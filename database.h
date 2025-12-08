#ifndef DATABASE_H
#define DATABASE_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>

class Database : public QObject
{
    Q_OBJECT

public:
    // 获取单例实例
    static Database& instance();

    // 连接到数据库
    bool connect();

    // 断开数据库连接
    void disconnect();

    // 检查数据库是否已连接
    bool isConnected() const { return m_isConnected; }

    // 获取数据库对象
    QSqlDatabase getDatabase() const {
        if (!m_isConnected || !m_db.isOpen()) {
            return QSqlDatabase();
        }
        return m_db;
    }

    // 执行查询（包装函数）
    bool executeQuery(const QString& queryStr);

    // 用户表操作
    bool addUser(const QString& username, const QString& password, int role,
                 int student_id = -1, int teacher_id = -1);
    bool authenticateUser(const QString& username, const QString& password, int role);
    QList<QList<QVariant>> getUsers();
    bool deleteUser(int userId);

    // 学生表操作
    bool addStudent(int student_id, const QString& name, int age, int credits = 0);
    bool updateStudent(int student_id, const QString& name, int age, int credits);
    bool deleteStudent(int student_id);
    QList<QList<QVariant>> getStudents();

    // 教师表操作
    bool addTeacher(int teacher_id, const QString& name, int age);
    bool updateTeacher(int teacher_id, const QString& name, int age);
    bool deleteTeacher(int teacher_id);
    QList<QList<QVariant>> getTeachers();

    // 课程表操作
    bool addCourse(int course_id, const QString& name, float credit);
    bool updateCourse(int course_id, const QString& name, float credit);
    bool deleteCourse(int course_id);
    QList<QList<QVariant>> getCourses();

    // 授课表操作
    bool addTeaching(int teacher_id, int course_id, const QString& semester,
                     const QString& class_time, const QString& classroom);
    bool deleteTeaching(int teacher_id, int course_id, const QString& semester);
    QList<QList<QVariant>> getTeachings();

    // 选课成绩表操作
    bool addEnrollment(int student_id, int teacher_id, int course_id,
                       const QString& semester, float score = 0.0);
    bool updateEnrollmentScore(int student_id, int teacher_id, int course_id,
                               const QString& semester, float score);
    bool deleteEnrollment(int student_id, int teacher_id, int course_id,
                          const QString& semester);
    QList<QList<QVariant>> getEnrollments();

private:
    // 私有构造函数确保单例
    explicit Database(QObject *parent = nullptr);

    // 禁止复制和赋值
    Database(const Database&) = delete;
    Database& operator=(const Database&) = delete;

    // 创建数据库表
    void createTables();

    // 创建默认管理员账户
    void createDefaultAdmin();

    // 启用外键约束
    bool enableForeignKeys();

private:
    bool m_isConnected;
    QString m_dbPath;
    QSqlDatabase m_db;  // 数据库连接对象
};

#endif // DATABASE_H
