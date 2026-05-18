#pragma once

#include <QList>
#include <QMap>
#include <QString>
#include <QStringList>

/**
 * @brief 单个组件 JSON 发现结果
 */
struct ComponentDiscoveryItem
{
    QString filePath;
    QString kind;
    QString partNumber;
    QString name;
    bool parsed{false};
    QString error;
    QStringList warnings;
};

/**
 * @brief 组件自动发现与分类结果
 */
struct ComponentDiscoveryResult
{
    QList<ComponentDiscoveryItem> items;
    QMap<QString, QList<ComponentDiscoveryItem>> groupedByKind;
};

/**
 * @brief 自动扫描 Components 目录下所有 JSON，并按 kind 分类
 *
 * 设计目标：
 * 1. 自动识别未来新增类别（不仅 Connector/Terminal）。
 * 2. 优先读取 JSON 内 kind 字段；若缺失则回退到目录名。
 * 3. 保留解析警告，便于后续补全数据。
 */
class ComponentAutoDiscovery
{
public:
    /**
     * @brief 扫描指定目录（递归）并分类组件 JSON
     * @param componentsRoot 例如 ./Components（可由运行目录推导）
     */
    static ComponentDiscoveryResult scan(const QString &componentsRoot);
};

