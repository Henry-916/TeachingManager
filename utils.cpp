#include "utils.h"
#include <QHeaderView>
#include <QMessageBox>

void Utils::setupTable(QTableWidget* table, const QStringList& headers)
{
    table->setColumnCount(headers.size());
    table->setHorizontalHeaderLabels(headers);
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    table->setAlternatingRowColors(true);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
}

void Utils::loadTableData(QTableWidget* table, const QList<QMap<QString, QVariant>>& data)
{
    table->setRowCount(data.size());

    for (int i = 0; i < data.size(); i++) {
        int col = 0;
        for (const auto& value : data[i]) {
            table->setItem(i, col++, new QTableWidgetItem(value.toString()));
        }
    }
}

QString Utils::trim(const QString& str)
{
    return str.trimmed();
}

bool Utils::isValidNumber(const QString& str)
{
    bool ok;
    str.toDouble(&ok);
    return ok;
}

bool Utils::checkPermission(bool hasPermission, const QString& actionName, QWidget* parent)
{
    if (!hasPermission && parent) {
        QMessageBox::warning(parent, "权限不足",
                             QString("您没有%1的权限").arg(actionName));
        return false;
    }
    return hasPermission;
}
