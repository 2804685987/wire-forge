#include "HarnessComponent.h"
#include "ComponentPort.h"      // 后续创建
#include <QPainter>
#include <QStyleOptionGraphicsItem>

HarnessComponent::HarnessComponent(QGraphicsItem* parent)
        : QGraphicsObject(parent)
        , m_id(QUuid::createUuid())
        , m_name("New Component")
{
    setFlag(QGraphicsItem::ItemIsMovable, true);
    setFlag(QGraphicsItem::ItemIsSelectable, true);
    setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
}

HarnessComponent::~HarnessComponent() = default;

void HarnessComponent::setName(const QString& name)
{
    if (m_name != name) {
        m_name = name;
        emit nameChanged(m_name);
        update();
    }
}

void HarnessComponent::setPartNumber(const QString& pn)
{
    if (m_partNumber != pn) {
        m_partNumber = pn;
        emit partNumberChanged(pn);
    }
}

void HarnessComponent::setSimplifiedMode(bool enabled)
{
    if (m_simplifiedMode != enabled) {
        m_simplifiedMode = enabled;
        update();
    }
}

QVariant HarnessComponent::property(const QString& key) const
{
    return m_customProperties.value(key);
}

void HarnessComponent::setProperty(const QString& key, const QVariant& value)
{
    if (m_customProperties.value(key) != value) {
        m_customProperties[key] = value;
        emit propertyChanged(key, value);
        update();
    }
}

QStringList HarnessComponent::propertyKeys() const
{
    return m_customProperties.keys();
}

QMap<QString, QVariant> HarnessComponent::allProperties() const
{
    return m_customProperties;
}

QJsonObject HarnessComponent::toJson() const
{
    QJsonObject obj;
    obj["id"] = m_id.toString();
    obj["type"] = type();
    obj["name"] = m_name;
    obj["partNumber"] = m_partNumber;
    obj["x"] = pos().x();
    obj["y"] = pos().y();
    obj["rotation"] = rotation();
    // 可继续添加 customProperties 序列化
    return obj;
}

bool HarnessComponent::fromJson(const QJsonObject& obj)
{
    // 基础反序列化实现，子类可扩展
    if (obj.contains("name")) m_name = obj["name"].toString();
    if (obj.contains("partNumber")) m_partNumber = obj["partNumber"].toString();

    qreal x = obj["x"].toDouble();
    qreal y = obj["y"].toDouble();
    setPos(x, y);

    if (obj.contains("rotation"))
        setRotation(obj["rotation"].toDouble());

    return true;
}

bool HarnessComponent::validate() const
{
    return true;    // 子类重写实现具体校验
}

// ==================== 保护辅助方法 ====================
void HarnessComponent::drawSelectionFrame(QPainter* painter) const
{
    if (isSelected()) {
        painter->setPen(QPen(Qt::blue, 2, Qt::DashLine));
        painter->drawRect(boundingRect().adjusted(-2, -2, 2, 2));
    }
}

void HarnessComponent::drawSimplified(QPainter* painter) const
{
    // 简化模式：只画矩形 + 名称
    painter->setBrush(Qt::lightGray);
    painter->drawRect(boundingRect());
    painter->setPen(Qt::black);
    painter->drawText(boundingRect(), Qt::AlignCenter, m_name);
}