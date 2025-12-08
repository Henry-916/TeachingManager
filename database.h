#ifndef DATABASE_H
#define DATABASE_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>

class Database : public QObject
{
    Q_OBJECT

public:
    static Database& getInstance();
    bool connect();
    bool isConnected() const;
    void disconnect();

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
    explicit Database(QObject *parent = nullptr);
    Database(const Database&) = delete;
    Database& operator=(const Database&) = delete;

    void createTables();  // 创建数据库表
    bool enableForeignKeys();  // 启用外键约束
};

#endif // DATABASE_H
