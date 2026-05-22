#include "ConnectorItem.h"
#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <QJsonArray>

ConnectorItem::ConnectorItem(QGraphicsItem* parent)
        : HarnessComponent(parent)
{
    setName("Connector");
    setPartNumber("XXXXXX-XXX");

    // 默认创建一个示例端子
    auto port1 = new ComponentPort("Pin1");
    port1->setDirection(PortDirection::Right);
    port1->setLocalPos(QPointF(120, 25));
    addPort(port1);

    auto port2 = new ComponentPort("Pin2");
    port2->setDirection(PortDirection::Right);
    port2->setLocalPos(QPointF(120, 55));
    addPort(port2);

    // 默认子组件插槽
    SubComponentSlot rearClip;
    rearClip.slotId = "RearClip";
    rearClip.slotName = "尾夹";
    rearClip.compatibleTypes = "Clip";
    rearClip.relativePos = QPointF(10, 30);
    rearClip.required = true;
    addSubSlot(rearClip);

    SubComponentSlot seal;
    seal.slotId = "WaterproofSeal";
    seal.slotName = "防水塞";
    seal.compatibleTypes = "Seal";
    seal.relativePos = QPointF(10, 50);
    addSubSlot(seal);
}

ConnectorItem::~ConnectorItem()
{
    qDeleteAll(m_ports);
    qDeleteAll(m_subSlots);
}

QRectF ConnectorItem::boundingRect() const
{
    if (m_simplifiedMode) {
        return QRectF(0, 0, m_size.width(), m_size.height());
    }
    return QRectF(0, 0, m_size.width(), m_size.height());
}

void ConnectorItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget*)
{
    painter->setRenderHint(QPainter::Antialiasing);

    if (m_simplifiedMode) {
        drawSimplified(painter);
        return;
    }

    // 正常模式绘制
    QRectF rect = boundingRect();

    // 主体
    painter->setBrush(QColor(240, 248, 255));
    painter->setPen(QPen(Qt::black, 2));
    painter->drawRect(rect);

    // 端口（端子）
    drawPorts(painter);

    // 子组件插槽位置标记
    painter->setPen(QPen(Qt::darkGray, 1, Qt::DashLine));
    for (auto slot : m_subSlots) {
        QPointF pos = slot->relativePos;
        painter->drawEllipse(pos, 6, 6);
    }

    // 名称和零件号
    painter->setPen(Qt::black);
    painter->drawText(rect.adjusted(8, 8, -8, -8), Qt::AlignTop | Qt::AlignLeft,
                      m_name + "\n" + m_partNumber);

    drawSelectionFrame(painter);
}

void ConnectorItem::addPort(ComponentPort* port)
{
    m_ports.append(port);
}

QList<SubComponentSlot*> ConnectorItem::subSlots() const
{
    return m_subSlots;
}

void ConnectorItem::addSubSlot(const SubComponentSlot& slot)
{
    auto newSlot = new SubComponentSlot(slot);
    m_subSlots.append(newSlot);
}

SubComponentSlot* ConnectorItem::findSlot(const QString& slotId)
{
    for (auto slot : m_subSlots) {
        if (slot->slotId == slotId) return slot;
    }
    return nullptr;
}

bool ConnectorItem::attachSubComponent(const QString& slotId, HarnessComponent* subComponent)
{
    auto slot = findSlot(slotId);
    if (!slot || !slot->isCompatible(subComponent)) return false;

    slot->attachedComponent = subComponent;
    return true;
}

bool ConnectorItem::detachSubComponent(const QString& slotId)
{
    auto slot = findSlot(slotId);
    if (!slot) return false;
    slot->attachedComponent = nullptr;
    return true;
}

bool ConnectorItem::validate() const
{
    // 检查必选子组件是否齐全
    for (auto slot : m_subSlots) {
        if (slot->required && !slot->isOccupied()) {
            return false;
        }
    }
    return true;
}

QJsonObject ConnectorItem::toJson() const
{
    QJsonObject obj = HarnessComponent::toJson();

    QJsonArray portsArray;
    for (auto port : m_ports) {
        portsArray.append(port->toJson());
    }
    obj["ports"] = portsArray;

    // 子插槽序列化（简化版）
    QJsonArray slotsArray;
    for (auto slot : m_subSlots) {
        QJsonObject s;
        s["slotId"] = slot->slotId;
        s["slotName"] = slot->slotName;
        // 可继续扩展
        slotsArray.append(s);
    }
    obj["subSlots"] = slotsArray;

    return obj;
}

bool ConnectorItem::fromJson(const QJsonObject& obj)
{
    HarnessComponent::fromJson(obj);
    // 后续实现 ports 和 subSlots 的反序列化
    return true;
}
void ConnectorItem::drawPorts(QPainter* painter) const
{
    // 简单的端子绘制逻辑 (占位实现，让端子显示为一个绿色小方块)
    painter->setBrush(Qt::green);
    painter->setPen(Qt::black);
    for (auto port : m_ports) {
        // 假设端子是一个 10x10 的小方块
        QRectF portRect(port->localPos().x() - 5, port->localPos().y() - 5, 10, 10);
        painter->drawRect(portRect);
    }
}

