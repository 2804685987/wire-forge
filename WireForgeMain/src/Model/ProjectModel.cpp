#include "ProjectModel.h"

#include <QDebug>
#include <QJsonArray>
#include <QJsonValue>
#include <QSet>
#include "Tools/ProjectSerializer.h"
#include "Components/ConnectorModel.h"
#include "Components/BasicConnectorModel.h"

NodeModel::NodeModel(const QUuid &id, QObject *parent)
        : QObject(parent)
{
    if (!id.isNull()) {
        m_id = id;
    }
}

NodeModel::NodeModel(QObject *parent)
        : QObject(parent)
{
}

void NodeModel::addConnector(ConnectorModel *connector)
{
    if (!connector) return;
    connector->setParent(this);
    m_connectors.append(connector);
}

QVector<ConnectorModel*> NodeModel::connectors() const
{
    return m_connectors;
}

ConnectorModel *NodeModel::connectorById(const QUuid &id) const
{
    for (auto *c : m_connectors) {
        if (c && c->id() == id) {
            return c;
        }
    }
    return nullptr;
}

QJsonObject NodeModel::toJson() const
{
    QJsonObject o;
    o.insert("id", m_id.toString());
    QJsonObject pos;
    pos.insert("x", m_position.x());
    pos.insert("y", m_position.y());
    o.insert("pos", pos);
    QJsonArray carr;
    for (const auto *c : m_connectors) {
        QJsonObject cj = c->toJson();
        // ensure type is present
        if (!cj.contains("type")) cj.insert("type", c->connectorType());
        carr.append(cj);
    }
    o.insert("connectors", carr);
    return o;
}

void NodeModel::fromJson(const QJsonObject &obj)
{
    if (obj.contains("id")) m_id = QUuid(obj.value("id").toString());
    if (obj.contains("pos") && obj.value("pos").isObject()) {
        const QJsonObject pos = obj.value("pos").toObject();
        m_position.setX(pos.value("x").toDouble());
        m_position.setY(pos.value("y").toDouble());
    }
    qDeleteAll(m_connectors);
    m_connectors.clear();
    if (obj.contains("connectors") && obj.value("connectors").isArray()) {
        QJsonArray ca = obj.value("connectors").toArray();
        for (const auto &cv : ca) {
            if (!cv.isObject()) continue;
            QJsonObject co = cv.toObject();
            ConnectorModel *c = ProjectSerializer::createConnectorFromJson(co, this);
            if (c) m_connectors.append(c);
        }
    }
}

WireModel::WireModel(const QUuid &id, const QUuid &from, const QUuid &to, QObject *parent)
        : QObject(parent), m_from(from), m_to(to)
{
    if (!id.isNull()) {
        m_id = id;
    }
}

WireModel::WireModel(QObject *parent)
        : QObject(parent)
{
}

ProjectModel::ProjectModel(QObject *parent)
        : QObject(parent)
{
}

void WireModel::insertConstraint(int index, const QPointF &point)
{
    if (index < 0 || index > m_constraints.size()) {
        m_constraints.append(point);
        return;
    }
    m_constraints.insert(index, point);
}

bool WireModel::removeConstraint(int index)
{
    if (index < 0 || index >= m_constraints.size()) {
        return false;
    }
    m_constraints.removeAt(index);
    return true;
}

bool WireModel::setConstraint(int index, const QPointF &point)
{
    if (index < 0 || index >= m_constraints.size()) {
        return false;
    }
    m_constraints[index] = point;
    return true;
}


bool ProjectModel::canAddWire(const QUuid &from, const QUuid &to, QString *error) const
{
    const WireValidationRules rules = m_wireRules;
    if (from.isNull() || to.isNull()) {
        if (error) *error = QStringLiteral("连接器id不能为空");
        return false;
    }
    if (from == to && !rules.allowSameConnector) {
        if (error) *error = QStringLiteral("相同连接器不能连接");
        return false;
    }

    NodeModel *fromNode = nullptr;
    NodeModel *toNode = nullptr;
    ConnectorModel *fromConnector = findConnector(from, &fromNode);
    ConnectorModel *toConnector = findConnector(to, &toNode);
    if (!fromConnector || !toConnector) {
        if (error) *error = QStringLiteral("连接器不存在");
        return false;
    }
    if (!fromNode || !toNode) {
        if (error) *error = QStringLiteral("节点不存在");
        return false;
    }
    if (fromNode == toNode && !rules.allowSameNode) {
        if (error) *error = QStringLiteral("相同节点不能连接");
        return false;
    }

    const ConnectorModel::Direction fromDir = fromConnector->direction();
    const ConnectorModel::Direction toDir = toConnector->direction();
    if (fromDir == toDir) {
        if (fromDir == ConnectorModel::Input && !rules.allowInputToInput) {
            if (error) *error = QStringLiteral("输入端口不能连接到输入端口");
            return false;
        }
        if (fromDir == ConnectorModel::Output && !rules.allowOutputToOutput) {
            if (error) *error = QStringLiteral("输出端口不能连接到输出端口");
            return false;
        }
    } else {
        if (fromDir == ConnectorModel::Input && toDir == ConnectorModel::Output && !rules.allowInputToOutput) {
            if (error) *error = QStringLiteral("输入端口不能连接到输出端口");
            return false;
        }
        if (fromDir == ConnectorModel::Output && toDir == ConnectorModel::Input) {
            // ok
        }
    }

    if (!rules.allowMultipleToInput && toDir == ConnectorModel::Input) {
        for (const auto *w : m_wires) {
            if (w && w->toConnector() == to) {
                if (error) *error = QStringLiteral("输入端口已连接");
                return false;
            }
        }
    }
    if (!rules.allowMultipleFromOutput && fromDir == ConnectorModel::Output) {
        for (const auto *w : m_wires) {
            if (w && w->fromConnector() == from) {
                if (error) *error = QStringLiteral("输出端口已连接");
                return false;
            }
        }
    }
    if (!rules.allowDuplicateWire) {
        for (const auto *w : m_wires) {
            if (w && w->fromConnector() == from && w->toConnector() == to) {
                if (error) *error = QStringLiteral("重复连接");
                return false;
            }
        }
    }
    return true;
}

bool ProjectModel::removeNode(const QUuid &id)
{
    for (int i = 0; i < m_nodes.size(); ++i) {
        if (m_nodes[i] && m_nodes[i]->id() == id) {
            NodeModel *node = m_nodes.takeAt(i);
            QSet<QUuid> connectorIds;
            for (auto *c : node->connectors()) {
                if (c) connectorIds.insert(c->id());
            }
            for (int w = m_wires.size() - 1; w >= 0; --w) {
                auto *wire = m_wires[w];
                if (!wire) continue;
                if (connectorIds.contains(wire->fromConnector()) || connectorIds.contains(wire->toConnector())) {
                    m_wires.removeAt(w);
                    emit wireRemoved(wire->id());
                    delete wire;
                }
            }
            emit nodeRemoved(node->id());
            delete node;
            return true;
        }
    }
    return false;
}

bool ProjectModel::removeWire(const QUuid &id)
{
    for (int i = 0; i < m_wires.size(); ++i) {
        if (m_wires[i] && m_wires[i]->id() == id) {
            WireModel *wire = m_wires.takeAt(i);
            emit wireRemoved(wire->id());
            delete wire;
            return true;
        }
    }
    return false;
}

QJsonObject ProjectModel::toJson() const
{
    QJsonObject root;
    root.insert("version", QStringLiteral("1.0"));

    QJsonArray nodesArr;
    for (const auto *n : m_nodes) {
        nodesArr.append(n->toJson());
    }
    root.insert("nodes", nodesArr);

    QJsonArray wiresArr;
    for (const auto *w : m_wires) {
        QJsonObject wo;
        wo.insert("id", w->id().toString());
        wo.insert("from", w->fromConnector().toString());
        wo.insert("to", w->toConnector().toString());
        QJsonArray constraintArr;
        for (const auto &pt : w->constraints()) {
            QJsonObject p;
            p.insert("x", pt.x());
            p.insert("y", pt.y());
            constraintArr.append(p);
        }
        wo.insert("constraints", constraintArr);
        wiresArr.append(wo);
    }
    root.insert("wires", wiresArr);

    QJsonArray layersArr;
    for (const auto *l : m_layers) {
        QJsonObject lo;
        lo.insert("id", l->id().toString());
        lo.insert("name", l->name());
        lo.insert("visible", l->isVisible());
        lo.insert("locked", l->isLocked());
        lo.insert("properties", l->properties()->toJson());
        layersArr.append(lo);
    }
    root.insert("layers", layersArr);
    return root;
}

void ProjectModel::fromJson(const QJsonObject &obj)
{
    qDeleteAll(m_nodes);
    m_nodes.clear();
    qDeleteAll(m_wires);
    m_wires.clear();
    qDeleteAll(m_layers);
    m_layers.clear();

    if (obj.contains("nodes") && obj.value("nodes").isArray()) {
        const QJsonArray na = obj.value("nodes").toArray();
        for (const auto &v : na) {
            if (!v.isObject()) {
                continue;
            }
            const QJsonObject no = v.toObject();
            auto node = new NodeModel(QUuid(no.value("id").toString()), this);
            node->fromJson(no);
            m_nodes.append(node);
        }
    }

    if (obj.contains("wires") && obj.value("wires").isArray()) {
        const QJsonArray wa = obj.value("wires").toArray();
        for (const auto &v : wa) {
            if (!v.isObject()) {
                continue;
            }
            const QJsonObject wo = v.toObject();
            const QUuid fromId(wo.value("from").toString());
            const QUuid toId(wo.value("to").toString());
            auto wire = new WireModel(QUuid(wo.value("id").toString()), fromId, toId, this);
            if (wo.contains("constraints") && wo.value("constraints").isArray()) {
                QVector<QPointF> points;
                const QJsonArray ca = wo.value("constraints").toArray();
                points.reserve(ca.size());
                for (const auto &cv : ca) {
                    if (!cv.isObject()) continue;
                    const QJsonObject po = cv.toObject();
                    points.append(QPointF(po.value("x").toDouble(), po.value("y").toDouble()));
                }
                wire->setConstraints(points);
            }
            m_wires.append(wire);
        }
    }

    if (obj.contains("layers") && obj.value("layers").isArray()) {
        const QJsonArray la = obj.value("layers").toArray();
        for (const auto &v : la) {
            if (!v.isObject()) {
                continue;
            }
            const QJsonObject lo = v.toObject();
            auto layer = new LayerModel(lo.value("name").toString(), this);
            layer->setVisible(lo.value("visible").toBool(true));
            layer->setLocked(lo.value("locked").toBool(false));
            if (lo.contains("properties") && lo.value("properties").isObject()) {
                layer->properties()->fromJson(lo.value("properties").toObject());
            }
            m_layers.append(layer);
        }
    }
}

NodeModel *ProjectModel::findNode(const QUuid &id) const
{
    for (auto *n : m_nodes) {
        if (n && n->id() == id) {
            return n;
        }
    }
    return nullptr;
}

WireModel *ProjectModel::findWire(const QUuid &id) const
{
    for (auto *w : m_wires) {
        if (w && w->id() == id) {
            return w;
        }
    }
    return nullptr;
}

ConnectorModel *ProjectModel::findConnector(const QUuid &id, NodeModel **outNode) const
{
    for (auto *n : m_nodes) {
        if (!n) continue;
        auto *c = n->connectorById(id);
        if (c) {
            if (outNode) *outNode = n;
            return c;
        }
    }
    if (outNode) *outNode = nullptr;
    return nullptr;
}

QUuid ProjectModel::addNode(const QPointF &pos)
{
    return addNodeWithId(QUuid(), pos);
}

QUuid ProjectModel::addNodeWithId(const QUuid &id, const QPointF &pos)
{
    auto node = new NodeModel(id, this);
    node->setPosition(pos);
    node->addConnector(new BasicConnectorModel(QStringLiteral("In"), ConnectorModel::Input, node));
    node->addConnector(new BasicConnectorModel(QStringLiteral("Out"), ConnectorModel::Output, node));
    m_nodes.append(node);
    emit nodeAdded(node->id());
    return node->id();
}

QUuid ProjectModel::addWire(const QUuid &from, const QUuid &to)
{
    return addWireWithId(QUuid(), from, to);
}

QUuid ProjectModel::addWireWithId(const QUuid &id, const QUuid &from, const QUuid &to)
{
    QString error;
    if (!canAddWire(from, to, &error)) {
        qWarning() << "ProjectModel::addWire failed" << error;
        return {};
    }
    auto wire = new WireModel(id, from, to, this);
    m_wires.append(wire);
    emit wireAdded(wire->id());
    return wire->id();
}
