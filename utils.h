#ifndef UTILS_H
#define UTILS_H

#include <QTableWidget>
#include <QList>
#include <QMap>
#include <QVariant>

namespace Utils {
// 表格工具函数
void setupTable(QTableWidget* table, const QStringList& headers);
void loadTableData(QTableWidget* table, const QList<QMap<QString, QVariant>>& data);

// 字符串工具函数
QString trim(const QString& str);
bool isValidNumber(const QString& str);

// 权限检查辅助函数
bool checkPermission(bool hasPermission, const QString& actionName, QWidget* parent = nullptr);
}

#endif // UTILS_H
