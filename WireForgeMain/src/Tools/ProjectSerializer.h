#pragma once

#include <QString>
#include <QJsonObject>
#include <functional>
#include <QHash>

class ProjectModel;
class ConnectorModel;

/**
 * @brief ProjectSerializer 提供 ProjectModel 的 JSON 序列化与反序列化
 */
class ProjectSerializer
{
public:
    /**
     * @brief 将 ProjectModel 序列化并保存到指定文件路径
     */
    static bool save(ProjectModel *model, const QString &path);

    /**
     * @brief 从指定文件加载项目并填充到 model
     */
    static bool load(ProjectModel *model, const QString &path);

    // --- Connector factory registry ---
    using ConnectorFactory = std::function<ConnectorModel*(QObject *parent)>;

    /**
     * @brief 注册连接器工厂
     */
    static void registerConnectorFactory(const QString &type, ConnectorFactory factory);

    /**
     * @brief 注销连接器工厂
     */
    static void unregisterConnectorFactory(const QString &type);

    /**
     * @brief 根据 JSON 创建相应连接器实例（并调用 fromJson 恢复），失败返回 nullptr
     */
    static ConnectorModel* createConnectorFromJson(const QJsonObject &obj, QObject *parent = nullptr);
};


