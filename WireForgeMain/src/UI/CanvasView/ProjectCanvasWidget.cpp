#include "ProjectCanvasWidget.h"

#include "Model/ProjectModel.h"
#include "Controller/ProjectController.h"
#include "Graphics/ConnectorModel.h"


#include <QPainter>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QtGlobal>
#include <cmath>
#include <algorithm>
#include <QPainterPath>

ProjectCanvasWidget::ProjectCanvasWidget(QWidget *parent)
        : QWidget(parent)
{
    setAutoFillBackground(true);
    setMinimumSize(QSize(400, 300));
    setFocusPolicy(Qt::StrongFocus);
}

void ProjectCanvasWidget::setModel(ProjectModel *model)
{
    if (m_model == model) return;
    if (m_model) {
        disconnect(m_nodeAddedConnection);
        disconnect(m_wireAddedConnection);
        disconnect(m_nodeRemovedConnection);
        disconnect(m_wireRemovedConnection);
    }
    m_model = model;
    m_selectedNodeIds.clear();
    m_hoverNodeId = QUuid();
    m_hoverConnectorId = QUuid();
    m_hoverWireId = QUuid();
    m_hoverConstraintWire = QUuid();
    m_hoverConstraintIndex = -1;
    m_draggingNode = false;
    m_dragNode = nullptr;
    m_draggingWire = false;
    m_wireFromConnector = QUuid();
    if (m_model) {
        m_nodeAddedConnection = connect(m_model, &ProjectModel::nodeAdded, this, [this]() { update(); });
        m_wireAddedConnection = connect(m_model, &ProjectModel::wireAdded, this, [this]() { update(); });
        m_nodeRemovedConnection = connect(m_model, &ProjectModel::nodeRemoved, this, [this]() { update(); });
        m_wireRemovedConnection = connect(m_model, &ProjectModel::wireRemoved, this, [this]() { update(); });
    }
    update();
}

void ProjectCanvasWidget::setController(ProjectController *controller)
{
    m_controller = controller;
}

void ProjectCanvasWidget::setStatusCallback(std::function<void(const QString &)> callback)
{
    m_statusCallback = std::move(callback);
}

void ProjectCanvasWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);

    p.fillRect(rect(), palette().window());
    drawGrid(p);

    if (!m_model) return;

    drawWires(p);
    drawNodes(p);
    drawInteractionHints(p);
}

void ProjectCanvasWidget::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (!m_model || event->button() != Qt::LeftButton) {
        QWidget::mouseDoubleClickEvent(event);
        return;
    }
    const QPointF pos =
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
            event->position();
#else
    event->localPos();
#endif
    int insertIndex = -1;
    WireModel *wire = nullptr;
    if (findWireSegmentAt(pos, &wire, &insertIndex)) {
        if (wire) {
            wire->insertConstraint(insertIndex, pos);
            m_selectedConstraintWire = wire->id();
            m_selectedConstraintIndex = insertIndex;
        }
        update();
        return;
    }
    QWidget::mouseDoubleClickEvent(event);
}

void ProjectCanvasWidget::mousePressEvent(QMouseEvent *event)
{
    if (!m_model) {
        QWidget::mousePressEvent(event);
        return;
    }
    const QPointF pos =
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
            event->position();
#else
    event->localPos();
#endif
    if (event->button() == Qt::RightButton) {
        QUuid wireId;
        int index = -1;
        if (findConstraintAt(pos, &wireId, &index)) {
            if (auto *wire = wireById(wireId)) {
                wire->removeConstraint(index);
                m_selectedConstraintWire = QUuid();
                m_selectedConstraintIndex = -1;
                update();
                return;
            }
        }
        QWidget::mousePressEvent(event);
        return;
    }
    if (event->button() != Qt::LeftButton) {
        QWidget::mousePressEvent(event);
        return;
    }
    if (beginConstraintDrag(pos)) {
        update();
        return;
    }
    m_dragStartPos = pos;
    m_draggingSelection = false;

    NodeModel *node = nullptr;
    ConnectorModel *connector = connectorAt(pos, &node);
    if (connector) {
        m_draggingWire = true;
        m_wireFromConnector = connector->id();
        m_wireDragPos = pos;
        m_hoverConnectorId = QUuid();
        update();
        return;
    }

    QPointF offset;
    NodeModel *hitNode = nodeAt(pos, &offset);
    if (hitNode) {
        if (event->modifiers().testFlag(Qt::ControlModifier)) {
            toggleSelection(hitNode->id());
        } else if (!m_selectedNodeIds.contains(hitNode->id())) {
            m_selectedNodeIds.clear();
            m_selectedNodeIds.insert(hitNode->id());
        }
        m_draggingNode = true;
        m_dragNode = hitNode;
        m_dragOffset = offset;
        update();
        return;
    }

    if (!event->modifiers().testFlag(Qt::ControlModifier)) {
        m_selectedNodeIds.clear();
    }
    m_draggingSelection = true;
    m_selectionRect = QRectF(pos, pos);
    update();
}

void ProjectCanvasWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (!m_model) {
        QWidget::mouseMoveEvent(event);
        return;
    }
    const QPointF pos =
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
            event->position();
#else
    event->localPos();
#endif
    if (m_draggingConstraint) {
        updateConstraintDrag(pos);
        update();
        return;
    }
    if (m_draggingNode && m_dragNode) {
        const QPointF newTopLeft = snapToGrid(pos - m_dragOffset);
        const QPointF delta = newTopLeft - m_dragNode->position();
        applyDragDelta(delta);
        update();
        return;
    }
    if (m_draggingWire) {
        m_wireDragPos = pos;
        NodeModel *node = nullptr;
        ConnectorModel *connector = connectorAt(pos, &node);
        m_hoverConnectorId = connector ? connector->id() : QUuid{};
        updateWireHoverText(pos);
        update();
        return;
    }
    if (m_draggingSelection) {
        m_selectionRect = QRectF(m_dragStartPos, pos).normalized();
        update();
        return;
    }
    updateHoverState(pos);
    update();
    QWidget::mouseMoveEvent(event);
}

void ProjectCanvasWidget::leaveEvent(QEvent *event)
{
    m_hoverNodeId = QUuid();
    m_hoverConnectorId = QUuid();
    m_hoverWireId = QUuid();
    m_hoverConstraintWire = QUuid();
    m_hoverConstraintIndex = -1;
    update();
    QWidget::leaveEvent(event);
}

void ProjectCanvasWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if (!m_model || event->button() != Qt::LeftButton) {
        QWidget::mouseReleaseEvent(event);
        return;
    }
    const QPointF pos =
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
            event->position();
#else
    event->localPos();
#endif
    if (m_draggingConstraint) {
        m_draggingConstraint = false;
        m_dragConstraintWire = QUuid();
        m_dragConstraintIndex = -1;
        update();
        return;
    }
    if (m_draggingWire) {
        NodeModel *toNode = nullptr;
        ConnectorModel *toConnector = connectorAt(pos, &toNode);
        if (toConnector && !m_wireFromConnector.isNull() && m_controller) {
            NodeModel *fromNode = nullptr;
            ConnectorModel *fromConnector = m_model->findConnector(m_wireFromConnector, &fromNode);
            if (fromConnector) {
                const auto rules = m_model->wireValidationRules();
                QUuid fromId = fromConnector->id();
                QUuid toId = toConnector->id();
                if (fromConnector->direction() == ConnectorModel::Input &&
                    toConnector->direction() == ConnectorModel::Output &&
                    !rules.allowInputToOutput) {
                    std::swap(fromId, toId);
                }
                const QUuid wireId = m_controller->addWire(fromId, toId);
                if (m_statusCallback) {
                    m_statusCallback(wireId.isNull() ? tr("创建连线失败")
                                                     : tr("已创建连线 %1").arg(wireId.toString(QUuid::WithoutBraces).left(8)));
                }
            }
        }
    }

    if (m_draggingSelection) {
        selectNodesInRect(m_selectionRect, event->modifiers().testFlag(Qt::ControlModifier));
    }

    m_draggingWire = false;
    m_wireFromConnector = QUuid();
    m_draggingNode = false;
    m_dragNode = nullptr;
    m_draggingSelection = false;
    m_selectionRect = {};
    m_hoverConnectorId = QUuid();
    m_hoverText.clear();
    m_hoverTextPos = QPointF(0, 0);
    update();
}

void ProjectCanvasWidget::keyPressEvent(QKeyEvent *event)
{
    if (!m_model) {
        QWidget::keyPressEvent(event);
        return;
    }
    if ((event->key() == Qt::Key_Delete || event->key() == Qt::Key_Backspace) &&
        !m_selectedConstraintWire.isNull() && m_selectedConstraintIndex >= 0) {
        if (auto *wire = wireById(m_selectedConstraintWire)) {
            wire->removeConstraint(m_selectedConstraintIndex);
        }
        m_selectedConstraintWire = QUuid();
        m_selectedConstraintIndex = -1;
        update();
        return;
    }
    QWidget::keyPressEvent(event);
}

void ProjectCanvasWidget::drawGrid(QPainter &p)
{
    const int gridSize = 20;
    const QColor gridColor = palette().midlight().color();
    p.save();
    p.setPen(QPen(gridColor, 1));
    for (int x = 0; x < width(); x += gridSize) {
        p.drawLine(x, 0, x, height());
    }
    for (int y = 0; y < height(); y += gridSize) {
        p.drawLine(0, y, width(), y);
    }
    p.restore();
}

void ProjectCanvasWidget::drawNodes(QPainter &p)
{
    p.save();
    for (auto *node : m_model->nodes()) {
        if (!node) continue;
        const QRectF rect(node->position(), QSizeF(kNodeWidth, kNodeHeight));
        const bool selected = m_selectedNodeIds.contains(node->id());
        const bool hovered = node->id() == m_hoverNodeId;
        const QColor borderColor = selected ? QColor(0, 120, 215)
                                            : (hovered ? QColor(0, 160, 230) : palette().text().color());
        const int borderWidth = selected ? 2 : (hovered ? 2 : 1);
        p.setPen(QPen(borderColor, borderWidth));
        p.setBrush(selected ? palette().alternateBase() : palette().base());
        p.drawRoundedRect(rect, 6, 6);

        p.drawText(rect.adjusted(8, 6, -8, -6), Qt::AlignLeft | Qt::AlignTop,
                   node->id().toString(QUuid::WithoutBraces).left(8));

        drawConnectors(p, node, rect);
    }
    p.restore();
}

void ProjectCanvasWidget::drawConnectors(QPainter &p, const NodeModel *node, const QRectF &rect)
{
    int inputIndex = 0;
    int outputIndex = 0;
    for (auto *connector : node->connectors()) {
        if (!connector) continue;
        const bool isInput = connector->direction() == ConnectorModel::Input;
        const int index = isInput ? inputIndex++ : outputIndex++;
        const qreal y = rect.top() + 20.0 + index * 16.0;
        const qreal x = isInput ? rect.left() : rect.right();
        const bool hovered = connector->id() == m_hoverConnectorId;
        p.setBrush(isInput ? QColor(70, 160, 255) : QColor(80, 200, 120));
        p.setPen(QPen(hovered ? QColor(255, 200, 0) : Qt::transparent, hovered ? 2 : 1));
        p.drawEllipse({x, y}, kConnectorRadius + (hovered ? 1.5 : 0.0), kConnectorRadius + (hovered ? 1.5 : 0.0));
    }
}

QPointF ProjectCanvasWidget::connectorAnchor(const QUuid &connectorId) const
{
    NodeModel *node = nullptr;
    ConnectorModel *connector = m_model->findConnector(connectorId, &node);
    if (!node || !connector) return {};

    const QRectF rect(node->position(), QSizeF(kNodeWidth, kNodeHeight));
    int inputIndex = 0;
    int outputIndex = 0;
    for (auto *c : node->connectors()) {
        if (!c) continue;
        const bool isInput = c->direction() == ConnectorModel::Input;
        if (c == connector) {
            const int index = isInput ? inputIndex : outputIndex;
            const qreal y = rect.top() + 20.0 + index * 16.0;
            const qreal x = isInput ? rect.left() : rect.right();
            return {x, y};
        }
        if (isInput) {
            ++inputIndex;
        } else {
            ++outputIndex;
        }
    }
    return {};
}

void ProjectCanvasWidget::drawWires(QPainter &p)
{
    p.save();
    for (auto *wire : m_model->wires()) {
        if (!wire) continue;
        const QPointF from = connectorAnchor(wire->fromConnector());
        const QPointF to = connectorAnchor(wire->toConnector());
        if (from.isNull() || to.isNull()) continue;
        const bool hovered = (wire->id() == m_hoverWireId || wire->id() == m_hoverConstraintWire);
        const QColor wireColor = hovered ? QColor(120, 170, 255) : QColor(180, 120, 220);
        p.setPen(QPen(wireColor, hovered ? 3 : 2));
        const QVector<QPointF> constraints = wire->constraints();
        const QPainterPath path = buildBezierPath(from, to, constraints);
        p.drawPath(path);
        drawConstraintPoints(p, wire->id(), constraints);
    }
    if (m_draggingWire && !m_wireFromConnector.isNull()) {
        const QPointF from = connectorAnchor(m_wireFromConnector);
        if (!from.isNull()) {
            bool canConnect = false;
            if (!m_hoverConnectorId.isNull()) {
                QString error;
                canConnect = m_model->canAddWire(m_wireFromConnector, m_hoverConnectorId, &error) ||
                             m_model->canAddWire(m_hoverConnectorId, m_wireFromConnector, &error);
            }
            p.setPen(QPen(canConnect ? QColor(80, 200, 120) : QColor(200, 80, 80), 2, Qt::DashLine));
            p.drawPath(buildBezierPath(from, m_wireDragPos, {}));
        }
    }
    p.restore();
}

void ProjectCanvasWidget::drawConstraintPoints(QPainter &p, const QUuid &wireId, const QVector<QPointF> &constraints)
{
    if (constraints.isEmpty()) return;
    p.save();
    p.setPen(QPen(QColor(60, 60, 60), 1));
    p.setBrush(QColor(255, 255, 255));
    for (int i = 0; i < constraints.size(); ++i) {
        const bool selected = (wireId == m_selectedConstraintWire && i == m_selectedConstraintIndex);
        const bool hovered = (wireId == m_hoverConstraintWire && i == m_hoverConstraintIndex);
        const auto &pt = constraints[i];
        if (selected) {
            p.setBrush(QColor(255, 230, 150));
        } else if (hovered) {
            p.setBrush(QColor(210, 235, 255));
        } else {
            p.setBrush(QColor(255, 255, 255));
        }
        p.drawEllipse(pt, selected ? 5.0 : (hovered ? 5.0 : 4.0), selected ? 5.0 : (hovered ? 5.0 : 4.0));
    }
    p.restore();
}

bool ProjectCanvasWidget::beginConstraintDrag(const QPointF &pos)
{
    const qreal hit = 6.0;
    for (auto *wire : m_model->wires()) {
        if (!wire) continue;
        const auto points = wire->constraints();
        for (int i = 0; i < points.size(); ++i) {
            if (QLineF(pos, points[i]).length() <= hit) {
                m_draggingConstraint = true;
                m_dragConstraintWire = wire->id();
                m_dragConstraintIndex = i;
                m_selectedConstraintWire = wire->id();
                m_selectedConstraintIndex = i;
                return true;
            }
        }
    }
    return false;
}

void ProjectCanvasWidget::updateConstraintDrag(const QPointF &pos)
{
    if (!m_draggingConstraint) return;
    if (auto *wire = wireById(m_dragConstraintWire)) {
        wire->setConstraint(m_dragConstraintIndex, pos);
    }
}

bool ProjectCanvasWidget::findWireSegmentAt(const QPointF &pos, WireModel **outWire, int *outInsertIndex) const
{
    const qreal hit = 8.0;
    for (auto *wire : m_model->wires()) {
        if (!wire) continue;
        const QPointF from = connectorAnchor(wire->fromConnector());
        const QPointF to = connectorAnchor(wire->toConnector());
        if (from.isNull() || to.isNull()) continue;
        QVector<QPointF> points;
        points.append(from);
        points.append(wire->constraints());
        points.append(to);
        for (int i = 0; i < points.size() - 1; ++i) {
            if (distanceToSegment(pos, points[i], points[i + 1]) <= hit) {
                if (outWire) *outWire = wire;
                if (outInsertIndex) *outInsertIndex = i;
                return true;
            }
        }
    }
    return false;
}

bool ProjectCanvasWidget::findConstraintAt(const QPointF &pos, QUuid *outWireId, int *outIndex) const
{
    const qreal hit = 6.0;
    for (auto *wire : m_model->wires()) {
        if (!wire) continue;
        const auto points = wire->constraints();
        for (int i = 0; i < points.size(); ++i) {
            if (QLineF(pos, points[i]).length() <= hit) {
                if (outWireId) *outWireId = wire->id();
                if (outIndex) *outIndex = i;
                return true;
            }
        }
    }
    return false;
}

qreal ProjectCanvasWidget::distanceToSegment(const QPointF &p, const QPointF &a, const QPointF &b)
{
    const QPointF ab = b - a;
    const qreal ab2 = QPointF::dotProduct(ab, ab);
    if (ab2 <= 0.0001) return QLineF(p, a).length();
    const QPointF ap = p - a;
    qreal t = QPointF::dotProduct(ap, ab) / ab2;
    t = std::max<qreal>(0.0, std::min<qreal>(1.0, t));
    const QPointF proj = a + ab * t;
    return QLineF(p, proj).length();
}

void ProjectCanvasWidget::updateWireHoverText(const QPointF &pos)
{
    m_hoverTextPos = pos + QPointF(12.0, 12.0);
    if (!m_model || !m_draggingWire || m_wireFromConnector.isNull() || m_hoverConnectorId.isNull()) {
        m_hoverText.clear();
        return;
    }
    QString error;
    const bool ok = m_model->canAddWire(m_wireFromConnector, m_hoverConnectorId, &error) ||
                    m_model->canAddWire(m_hoverConnectorId, m_wireFromConnector, &error);
    if (ok) {
        m_hoverText = tr("Connect: ok");
    } else {
        m_hoverText = tr("Connect: %1").arg(error.isEmpty() ? tr("not allowed") : error);
    }
}

WireModel *ProjectCanvasWidget::wireById(const QUuid &id) const
{
    for (auto *wire : m_model->wires()) {
        if (wire && wire->id() == id) return wire;
    }
    return nullptr;
}

QPointF ProjectCanvasWidget::snapToGrid(const QPointF &pos) const
{
    const qreal grid = 20.0;
    return {std::round(pos.x() / grid) * grid, std::round(pos.y() / grid) * grid};
}

void ProjectCanvasWidget::applyDragDelta(const QPointF &delta)
{
    if (delta.isNull()) return;
    for (auto *node : m_model->nodes()) {
        if (node && m_selectedNodeIds.contains(node->id())) {
            node->setPosition(node->position() + delta);
        }
    }
}

void ProjectCanvasWidget::selectNodesInRect(const QRectF &rect, bool additive)
{
    if (!additive) {
        m_selectedNodeIds.clear();
    }
    for (auto *node : m_model->nodes()) {
        if (!node) continue;
        const QRectF nodeRect(node->position(), QSizeF(kNodeWidth, kNodeHeight));
        if (rect.intersects(nodeRect)) {
            m_selectedNodeIds.insert(node->id());
        }
    }
}

void ProjectCanvasWidget::toggleSelection(const QUuid &id)
{
    if (m_selectedNodeIds.contains(id)) {
        m_selectedNodeIds.remove(id);
    } else {
        m_selectedNodeIds.insert(id);
    }
}

NodeModel *ProjectCanvasWidget::nodeAt(const QPointF &pos, QPointF *offset) const
{
    for (auto *node : m_model->nodes()) {
        if (!node) continue;
        const QRectF rect(node->position(), QSizeF(kNodeWidth, kNodeHeight));
        if (rect.contains(pos)) {
            if (offset) *offset = pos - rect.topLeft();
            return node;
        }
    }
    return nullptr;
}

ConnectorModel *ProjectCanvasWidget::connectorAt(const QPointF &pos, NodeModel **outNode) const
{
    const qreal hitRadius = kConnectorRadius + 4.0;
    for (auto *node : m_model->nodes()) {
        if (!node) continue;
        const QRectF rect(node->position(), QSizeF(kNodeWidth, kNodeHeight));
        int inputIndex = 0;
        int outputIndex = 0;
        for (auto *connector : node->connectors()) {
            if (!connector) continue;
            const bool isInput = connector->direction() == ConnectorModel::Input;
            const int index = isInput ? inputIndex++ : outputIndex++;
            const qreal y = rect.top() + 20.0 + index * 16.0;
            const qreal x = isInput ? rect.left() : rect.right();
            if (QLineF(pos, QPointF(x, y)).length() <= hitRadius) {
                if (outNode) *outNode = node;
                return connector;
            }
        }
    }
    if (outNode) *outNode = nullptr;
    return nullptr;
}

void ProjectCanvasWidget::updateHoverState(const QPointF &pos)
{

    m_hoverNodeId = QUuid();
    m_hoverWireId = QUuid();
    m_hoverConstraintWire = QUuid();
    m_hoverConstraintIndex = -1;

    QUuid constraintWire;
    int constraintIndex = -1;
    if (findConstraintAt(pos, &constraintWire, &constraintIndex)) {
        m_hoverConstraintWire = constraintWire;
        m_hoverConstraintIndex = constraintIndex;
        m_hoverWireId = constraintWire;
        return;
    }

    WireModel *wire = nullptr;
    int insertIndex = -1;
    if (findWireSegmentAt(pos, &wire, &insertIndex) && wire) {
        m_hoverWireId = wire->id();
        return;
    }

    if (NodeModel *node = nodeAt(pos, nullptr)) {
        m_hoverNodeId = node->id();
    }
}

void ProjectCanvasWidget::drawInteractionHints(QPainter &p){
    p.save();
    p.setPen(Qt::NoPen);
    p.setBrush(QColor(255, 255, 220, 200));
    if (!m_hoverText.isEmpty()) {
        p.drawText(m_hoverTextPos, m_hoverText);
        p.restore();
        return;
    }
    if (m_draggingSelection) {
        p.setBrush(QColor(100, 150, 255, 100));
        p.drawRect(m_selectionRect);
    }
    p.restore();

}

QPainterPath ProjectCanvasWidget::buildBezierPath(const QPointF &from, const QPointF &to,
                                                  const QVector<QPointF> &constraints) const {
    QPainterPath path(from);
    if (constraints.isEmpty()) {
        const qreal dx = to.x() - from.x();
        const QPointF ctrl1(from.x() + dx * 0.5, from.y());
        const QPointF ctrl2(from.x() + dx * 0.5, to.y());
        path.cubicTo(ctrl1, ctrl2, to);
    } else {
        QPointF prev = from;
        for (const auto &pt : constraints) {
            const qreal dx = pt.x() - prev.x();
            const QPointF ctrl1(prev.x() + dx * 0.5, prev.y());
            const QPointF ctrl2(prev.x() + dx * 0.5, pt.y());
            path.cubicTo(ctrl1, ctrl2, pt);
            prev = pt;
        }
    }
    return path;
}
