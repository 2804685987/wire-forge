#include "WireItem.h"
#include <QPainter>
#include <QPainterPath>
#include <QStyleOptionGraphicsItem>
#include <QJsonArray>

WireItem::WireItem(QGraphicsItem* parent)
        : HarnessComponent(parent)
{
    setName("Wire");
    setFlag(QGraphicsItem::ItemIsSelectable, true);
    setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
    setZValue(-1);   // 线在组件下方
}

WireItem::~WireItem() = default;

void WireItem::setStartPort(ComponentPort* port)
{
    m_startPort = port;
    updateLength();
    update();
}

void WireItem::setEndPort(ComponentPort* port)
{
    m_endPort = port;
    updateLength();
    update();
}

void WireItem::setControlPoints(const QList<QPointF>& points)
{
    m_controlPoints = points;
    updateLength();
    update();
}

void WireItem::addControlPoint(const QPointF& point)
{
    m_controlPoints.append(point);
    updateLength();
    update();
}

void WireItem::setWireGauge(const QString& gauge)
{
    m_wireGauge = gauge;
    update();
}

void WireItem::setColor(const QString& color)
{
    m_color = color;
    update();
}

void WireItem::setSignalName(const QString& name)
{
    m_signalName = name;
    update();
}

QRectF WireItem::boundingRect() const
{
    if (m_controlPoints.size() < 2) return QRectF();

    QRectF rect;
    for (const auto& p : m_controlPoints) {
        rect = rect.united(QRectF(p, QSizeF(1, 1)));
    }
    return rect.adjusted(-10, -10, 10, 10);   // 留出选中边距
}

QPainterPath WireItem::shape() const
{
    QPainterPath path;
    if (m_controlPoints.size() < 2) return path;

    path.moveTo(m_controlPoints.first());
    for (int i = 1; i < m_controlPoints.size(); ++i) {
        path.lineTo(m_controlPoints[i]);
    }
    return path;
}

void WireItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget*)
{
    painter->setRenderHint(QPainter::Antialiasing);

    QPen pen;
    pen.setColor(QColor(m_color));
    pen.setWidth(m_lineWidth);
    pen.setCapStyle(Qt::RoundCap);
    pen.setJoinStyle(Qt::RoundJoin);

    painter->setPen(pen);

    QPainterPath path;
    if (!m_controlPoints.isEmpty()) {
        path.moveTo(m_controlPoints.first());
        for (int i = 1; i < m_controlPoints.size(); ++i) {
            path.lineTo(m_controlPoints[i]);
        }
    }

    painter->drawPath(path);

    // 选中效果
    if (isSelected()) {
        QPen selectPen(Qt::blue, m_lineWidth + 2, Qt::DashLine);
        painter->setPen(selectPen);
        painter->drawPath(path);
    }

    // 可在此绘制箭头或标签
}

void WireItem::updateLength()
{
    if (m_controlPoints.size() < 2) {
        m_length = 0.0;
        return;
    }

    double len = 0.0;
    for (int i = 1; i < m_controlPoints.size(); ++i) {
        len += QLineF(m_controlPoints[i-1], m_controlPoints[i]).length();
    }
    if (m_length != len) {
        m_length = len;
        emit lengthChanged(m_length);
    }
}

bool WireItem::validate() const
{
    return m_startPort && m_endPort && m_length > 0;
}

QJsonObject WireItem::toJson() const
{
    QJsonObject obj = HarnessComponent::toJson();
    obj["wireGauge"] = m_wireGauge;
    obj["color"] = m_color;
    obj["signalName"] = m_signalName;
    obj["length"] = m_length;

    QJsonArray points;
    for (const auto& p : m_controlPoints) {
        QJsonObject pt;
        pt["x"] = p.x();
        pt["y"] = p.y();
        points.append(pt);
    }
    obj["controlPoints"] = points;

    return obj;
}

bool WireItem::fromJson(const QJsonObject& obj)
{
    HarnessComponent::fromJson(obj);
    if (obj.contains("wireGauge")) m_wireGauge = obj["wireGauge"].toString();
    if (obj.contains("color")) m_color = obj["color"].toString();
    if (obj.contains("signalName")) m_signalName = obj["signalName"].toString();

    m_controlPoints.clear();
    QJsonArray points = obj["controlPoints"].toArray();
    for (const auto& p : points) {
        QJsonObject pt = p.toObject();
        m_controlPoints.append(QPointF(pt["x"].toDouble(), pt["y"].toDouble()));
    }
    updateLength();
    return true;
}

QVariant WireItem::itemChange(GraphicsItemChange change, const QVariant& value)
{
    if (change == ItemPositionHasChanged || change == ItemRotationHasChanged) {
        updateLength();
    }
    return HarnessComponent::itemChange(change, value);
}