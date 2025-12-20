# 教学管理系统 Teaching Manager

## 项目简介

教学管理系统是一个基于Qt和MySQL开发的桌面应用程序，用于学校教学管理。系统支持三种用户角色：学生、教师和管理员，提供课程管理、成绩管理、用户管理等功能。

## 功能特性

### 角色权限

#### 管理员
- 查看和管理所有学生、教师、课程信息
- 管理授课安排和选课记录
- 执行自定义SQL语句
- 用户账户管理
- 数据库配置管理

#### 教师
- 查看个人信息和授课安排
- 管理所授课程的学生成绩
- 修改个人密码

#### 学生
- 查看个人信息和学分
- 查看已选课程和成绩
- 浏览可选课程列表
- 修改个人密码

## 技术栈

- **前端框架**: Qt 6 (C++)
- **数据库**: MySQL 8.0
- **开发环境**: Qt Creator + qmake
- **操作系统**: Windows/Linux/macOS

## 构建与运行

### 环境要求

1. **Qt 6.5+** 开发环境（包含MySQL驱动模块）
2. **MySQL 8.0+** 数据库服务器
3. **MySQL Connector/C++** 驱动（libmysql.dll/libmysql.so）
4. **qmake** 构建工具

### 编译步骤

#### Windows环境
1. 打开Qt Creator
2. 导入项目文件 `TeachingManager.pro`
3. 配置MySQL库路径（在.pro文件中已预设）：
   ```qmake
   INCLUDEPATH += "C:\Program Files\MySQL\MySQL Server 8.0\include"
   LIBS += -L"C:\Program Files\MySQL\MySQL Server 8.0\lib" -llibmysql
   ```
4. 如果MySQL安装路径不同，请修改上述路径
5. 选择构建套件（如：Desktop Qt 6.x.x MinGW 64-bit）
6. 点击构建项目

#### 命令行构建
```bash
# 进入项目目录
cd TeachingManager

# 生成Makefile
qmake TeachingManager.pro

# 编译（Windows MinGW）
mingw32-make

# 编译（Linux/macOS）
make
```

### 数据库配置

1. 确保MySQL服务已启动
2. 运行初始化脚本创建数据库结构：
```bash
mysql -u root -p < init_test_data.sql
```
3. Windows用户也可以双击 `init_test_data.bat`（需在MySQL安装目录下）
4. 默认配置：
   - 主机: localhost
   - 端口: 3306
   - 数据库: teaching_manager
   - 用户名: root
   - 密码: 123456

### 运行程序

1. 首次运行会自动创建配置文件
2. 如需修改数据库连接配置，可在登录界面重新配置
3. 使用以下测试账户登录：

| 角色 | 用户名/ID | 密码 |
|------|-----------|------|
| 管理员 | admin | admin123 |
| 教师 | 2001 | 123456 |
| 学生 | 1001 | 123456 |

## 项目结构

```
TeachingManager/
├── TeachingManager.pro          # qmake项目文件
├── *.cpp/h                      # 源代码文件
├── logindialog.ui              # 登录界面UI文件
├── init_test_data.sql          # 数据库初始化脚本
├── init_test_data.bat          # Windows初始化脚本
└── README.md                   # 项目说明文档
```

## 数据库设计

系统包含以下核心表：

1. **users** - 用户账户表
2. **students** - 学生信息表
3. **teachers** - 教师信息表
4. **courses** - 课程信息表
5. **teachings** - 授课安排表
6. **enrollments** - 选课记录表

## 功能模块说明

### 1. 登录模块
- 支持学生、教师、管理员三种角色登录
- 自动根据角色显示相应的输入标签
- 密码加密存储和验证

### 2. 管理员模块
- **数据管理**: 对学生、教师、课程信息进行增删改查
- **授课管理**: 安排教师授课任务
- **成绩管理**: 查看和管理所有课程成绩
- **用户管理**: 管理用户账户和权限
- **SQL执行**: 执行自定义SQL语句

### 3. 教师模块
- **个人信息**: 查看教师基本信息
- **我的授课**: 查看所授课程安排
- **成绩管理**: 录入和修改学生成绩
- **密码修改**: 修改登录密码

### 4. 学生模块
- **个人信息**: 查看学生基本信息和学分
- **我的选课**: 查看已选课程和成绩
- **可选课程**: 浏览可选课程列表
- **密码修改**: 修改登录密码

## 界面特点

1. **现代化界面**: 使用Fusion风格，界面简洁美观
2. **响应式设计**: 自适应窗口大小
3. **直观操作**: 表格展示，操作简便
4. **状态提示**: 实时反馈操作结果
5. **数据可视化**: 使用表格展示数据，支持排序和筛选

## 配置管理

系统使用INI文件存储配置：

```ini
[Database]
Host=localhost
Database=teaching_manager
Username=root
Password=123456
Port=3306
```

配置文件位置：`程序目录/config.ini`

## 测试数据

系统包含完整的测试数据：
- 10名学生
- 6名教师
- 10门课程
- 完整的授课安排和选课记录

## 注意事项

### 安全说明
1. 默认密码为123456，建议首次登录后修改
2. 管理员账户权限较高，请妥善保管
3. SQL执行功能需谨慎使用，避免误操作

### 部署建议
1. 生产环境应修改默认数据库密码
2. 建议定期备份数据库
3. 可为不同用户配置不同数据库权限

### 故障排除

1. **数据库连接失败**
   - 检查MySQL服务是否运行
   - 验证用户名和密码是否正确
   - 确认端口号是否正确

2. **程序无法启动**
   - 检查Qt和MySQL驱动是否正确安装
   - 验证环境变量配置
   - 查看系统日志获取详细信息

3. **界面显示异常**
   - 更新显卡驱动
   - 检查系统DPI设置
   - 尝试使用其他Qt样式

4. **MySQL驱动问题**
   - 确保Qt编译时启用了MySQL插件
   - 将libmysql.dll（Windows）或libmysql.so（Linux）复制到程序运行目录
   - 在Qt Creator中检查是否加载了qsqlmysql插件

## 开发说明

### 代码规范
- 使用Google C++代码风格
- 类名使用PascalCase
- 变量名使用camelCase
- 常量使用UPPER_CASE

### 扩展开发
如需添加新功能，可参考现有模块实现：
1. 继承BaseWindow类创建新窗口
2. 在Database类中添加对应的数据库操作方法
3. 更新用户权限配置

## 项目配置说明

### qmake配置文件 (TeachingManager.pro)
```qmake
# 需要Qt模块
QT += core gui sql widgets

# C++标准
CONFIG += c++17

# 目标文件
TARGET = TeachingManager
TEMPLATE = app

# MySQL配置（根据实际安装路径调整）
INCLUDEPATH += "C:\Program Files\MySQL\MySQL Server 8.0\include"
LIBS += -L"C:\Program Files\MySQL\MySQL Server 8.0\lib" -llibmysql
```

### 依赖项
1. **Qt模块**：
   - QtCore
   - QtGui
   - QtWidgets
   - QtSql

2. **系统库**：
   - MySQL客户端库（libmysql）
   - Windows: libmysql.dll
   - Linux: libmysqlclient.so

## 常见问题

### Q1: 编译时提示找不到MySQL头文件
**解决方案**：修改TeachingManager.pro中的INCLUDEPATH，指向正确的MySQL安装路径

### Q2: 运行时提示缺少MySQL驱动
**解决方案**：
1. 检查Qt安装目录下的plugins/sqldrivers目录是否有qsqlmysql.dll
2. 如果没有，需要重新编译Qt的MySQL插件
3. 将libmysql.dll复制到程序运行目录

### Q3: 连接数据库时出现乱码
**解决方案**：确保数据库、表和连接都使用utf8mb4编码

### Q4: 在Linux/macOS上编译失败
**解决方案**：
1. 安装MySQL开发包：
   - Ubuntu: `sudo apt-get install libmysqlclient-dev`
   - macOS: `brew install mysql`
2. 修改.pro文件中的库路径

## 许可证

本项目采用MIT许可证。详见LICENSE文件。

## 联系方式

如有问题或建议，请联系项目维护者或提交Issue。

---
*最后更新: 2024年*
*版本: 1.0.0*
