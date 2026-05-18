#pragma once

#include <QObject>
#include <QUuid>
#include <QVector>
#include <QPointF>
#include <QJsonObject>
#include <QString>
#include "LayerModel.h"

/**
 * @brief NodeModel 表示蓝图中的节点
 *
 * 目前为最小实现：仅包含唯一 id。后续将添加位置、端口列表、属性袋等字段。
 */
class NodeModel : public QObject {
Q_OBJECT

public:
    /**
     * @brief 构造 NodeModel
     * @param id 可选的节点 id（若为空则自动生成）
     * @param parent QObject 父对象
     */
    explicit NodeModel(const QUuid &id = QUuid(), QObject *parent = nullptr);
    /**
     * @brief 便捷构造：仅传入父对象
     */
    explicit NodeModel(QObject *parent);

    /** @brief 返回节点唯一 id */
    [[nodiscard]] QUuid id() const { return m_id; }

    /** @brief 返回节点在画布中的位置 */
    [[nodiscard]] QPointF position() const { return m_position; }

    /** @brief 设置节点在画布中的位置 */
    void setPosition(const QPointF &pos) { m_position = pos; }

    /**
     * @brief 向节点添加一个连接器（所有权由 NodeModel 接管）
     */
    void addConnector(class ConnectorModel *connector);

    /** @brief 返回当前节点的连接器列表（指针列表） */
    QVector<class ConnectorModel *> connectors() const;

    /** @brief 按 id 查找连接器，找不到返回 nullptr */
    class ConnectorModel *connectorById(const QUuid &id) const;

    /**
     * @brief 将节点序列化为 JSON（包含连接器数组）
     */
    QJsonObject toJson() const;

    /**
     * @brief 从 JSON 恢复节点状态（含连接器）；注意会创建新的 ConnectorModel 实例
     */
    void fromJson(const QJsonObject &obj);

private:
    QUuid m_id{QUuid::createUuid()};
    QPointF m_position;
    QVector<class ConnectorModel *> m_connectors;
};

/**
 * @brief WireModel 表示连接两个端口的连线
 */
class WireModel : public QObject {
Q_OBJECT

public:
    /**
     * @brief WireModel 表示连接两个端口的连线
     * @param id 可选的连线 id（若为空则自动生成）
     * @param from 源端口 id（可为空）
     * @param to 目标端口 id（可为空）
     */
    explicit WireModel(const QUuid &id = QUuid(), const QUuid &from = QUuid(), const QUuid &to = QUuid(), QObject *parent = nullptr);
    /**
     * @brief 便捷构造：仅传入父对象
     */
    explicit WireModel(QObject *parent);

    /** @brief 返回连线唯一 id */
    QUuid id() const { return m_id; }

    /** @brief 返回源端口 id */
    QUuid fromConnector() const { return m_from; }

    /** @brief 返回目标端口 id */
    QUuid toConnector() const { return m_to; }

    /** @brief 设置连线端点 */
    void setEndpoints(const QUuid &from, const QUuid &to) { m_from = from; m_to = to; }

    /** @brief 返回约束点列表 */
    QVector<QPointF> constraints() const { return m_constraints; }

    /** @brief 设置约束点列表 */
    void setConstraints(const QVector<QPointF> &points) { m_constraints = points; }

    /** @brief 插入约束点 */
    void insertConstraint(int index, const QPointF &point);

    /** @brief 删除约束点 */
    bool removeConstraint(int index);

    /** @brief 修改约束点 */
    bool setConstraint(int index, const QPointF &point);

private:
    QUuid m_id{QUuid::createUuid()};
    QUuid m_from;
    QUuid m_to;
    QVector<QPointF> m_constraints;
};

/**
 * @brief ProjectModel 为项目数据的根模型
 *
 * 管理节点、连线集合并发出变更信号。该类负责数据的内存管理与简单业务操作，
 * 但不直接处理序列化或 UI 渲染，职责由 ProjectSerializer 和 View 分担。
 */
class ProjectModel : public QObject {
Q_OBJECT

public:
    struct WireValidationRules {
        bool allowInputToInput{false};
        bool allowOutputToOutput{false};
        bool allowInputToOutput{false};
        bool allowSameNode{false};
        bool allowSameConnector{false};
        bool allowDuplicateWire{false};
        bool allowMultipleToInput{true};
        bool allowMultipleFromOutput{true};
    };

    /**
     * @brief 构造 ProjectModel
     * @param parent QObject 父对象
     */
    explicit ProjectModel(QObject *parent = nullptr);

    /**
     * @brief 向项目中添加一个节点
     * @param pos 节点在画布中的位置（当前实现忽略该参数）
     * @return 新增节点的 id
     */
    QUuid addNode(const QPointF &pos);

    /**
     * @brief 使用指定 id 添加节点（用于 Undo/Redo 恢复）
     */
    QUuid addNodeWithId(const QUuid &id, const QPointF &pos);

    /**
     * @brief 添加一条连线，连接 from -> to
     * @param from 源端口的 id
     * @param to 目标端口的 id
     * @return 新增连线的 id
     */
    QUuid addWire(const QUuid &from, const QUuid &to);

    /**
     * @brief 使用指定 id 添加连线（用于 Undo/Redo 恢复）
     */
    QUuid addWireWithId(const QUuid &id, const QUuid &from, const QUuid &to);

    /**
     * @brief 判断是否允许建立连接
     */
    bool canAddWire(const QUuid &from, const QUuid &to, QString *error = nullptr) const;

    /** @brief 设置连线验证规则 */
    void setWireValidationRules(const WireValidationRules &rules) { m_wireRules = rules; }

    /** @brief 获取连线验证规则 */
    WireValidationRules wireValidationRules() const { return m_wireRules; }

    /** @brief 删除指定节点 */
    bool removeNode(const QUuid &id);

    /** @brief 删除指定连线 */
    bool removeWire(const QUuid &id);

    /** @brief 返回节点列表（只读指针） */
    QVector<NodeModel *> nodes() const { return m_nodes; }

    /** @brief 返回连线列表（只读指针） */
    QVector<WireModel *> wires() const { return m_wires; }

    /** @brief 查找连接器及其所属节点 */
    class ConnectorModel *findConnector(const QUuid &id, NodeModel **outNode = nullptr) const;

    /**
     * @brief 将项目模型序列化为 QJsonObject
     * @return 表示项目的 QJsonObject（包含 version、nodes、wires 等字段）
     */
    QJsonObject toJson() const;

    /**
     * @brief 从 QJsonObject 恢复项目模型（会清空现有数据）
     * @param obj JSON 对象
     */
    void fromJson(const QJsonObject &obj);

Q_SIGNALS:

    /** @brief 当节点被添加时发出，参数为节点 id */
    void nodeAdded(const QUuid &id);

    /** @brief 当连线被添加时发出，参数为连线 id */
    void wireAdded(const QUuid &id);

    /** @brief 当节点被删除时发出 */
    void nodeRemoved(const QUuid &id);

    /** @brief 当连线被删除时发出 */
    void wireRemoved(const QUuid &id);

private:
    NodeModel *findNode(const QUuid &id) const;
    WireModel *findWire(const QUuid &id) const;

    QVector<NodeModel *> m_nodes;
    QVector<WireModel *> m_wires;
    QVector<LayerModel *> m_layers;
    WireValidationRules m_wireRules;
};
