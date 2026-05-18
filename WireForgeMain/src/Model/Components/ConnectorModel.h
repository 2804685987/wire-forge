#pragma once

#include <QObject>
#include <QUuid>
#include <QString>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonValue>
#include <QFile>
#include <QJsonParseError>
#include <QStringList>
#include <QVariant>
#include <utility>

/**
 * @brief ConnectorModel 表示节点的端口（连接器）
 *
 * 每个连接器拥有唯一 id、名称以及方向（输入或输出）。该类是轻量级的数据容器，
 * 用于在模型中表示可建立连线的端点。
 */
class ConnectorModel : public QObject
{
Q_OBJECT
public:
    /** 端口方向：输入或输出 */
    enum Direction { Input, Output };
    Q_ENUM(Direction)


    /**
     * @brief 构造函数
     * @param name 端口的可读名称
     * @param dir 端口方向（Input/Output）
     * @param parent 父 QObject
     */
    explicit ConnectorModel(QString name = QString(), Direction dir = Input, QObject *parent = nullptr)
        : QObject(parent), m_name(std::move(name)), m_direction(dir)
    {
    }

    /**
     * @brief 虚析构函数，允许子类正确析构
     */
    ~ConnectorModel() override = default;

    /** @brief 返回此连接器的唯一标识 id */
    [[nodiscard]] QUuid id() const { return m_id; }

    /** @brief 返回连接器名称 */
    [[nodiscard]] QString name() const { return m_name; }

    /** @brief 返回连接器方向（Input/Output） */
    [[nodiscard]] Direction direction() const { return m_direction; }

    /** @brief 设置连接器显示名称 */
    void setName(const QString &n) { m_name = n; }

    /**
     * @brief 子类必须实现：返回连接器的具体类型标识（用于序列化/类型判断）
     */
    [[nodiscard]] virtual QString connectorType() const = 0;

    /**
     * @brief 子类必须实现：将自身序列化为 QJsonObject
     */
    [[nodiscard]] virtual QJsonObject toJson() const = 0;

    /**
     * @brief 子类必须实现：从 QJsonObject 恢复自身状态
     */
    virtual void fromJson(const QJsonObject &obj) = 0;

protected:
    /** 子类可访问的成员，便于在子类实现中直接设置或读取 */
    QUuid m_id{QUuid::createUuid()};
    QString m_name;
    Direction m_direction{Input};
};
