#pragma once

#include <QWidget>
#include <QSet>
#include <QUuid>
#include <functional>

class ProjectModel;
class ProjectController;
class NodeModel;
class ConnectorModel;
class WireModel;

/**
 * @brief ProjectCanvasWidget 是项目画布视图，负责节点和连线的绘制与交互
 */
class ProjectCanvasWidget : public QWidget {
Q_OBJECT

public:
    /** @brief 项目画布视图构造函数 */
    explicit ProjectCanvasWidget(QWidget *parent = nullptr);

    /** @brief 设置项目模型 */
    void setModel(ProjectModel *model);

    /** @brief 设置项目控制器 */
    void setController(ProjectController *controller);

    /** @brief 设置状态更新回调 */
    void setStatusCallback(std::function<void(const QString &)> callback);

protected:
    /** @brief 绘制事件处理函数 */
    void paintEvent(QPaintEvent *event) override;

    /** @brief 处理鼠标双击事件 */
    void mouseDoubleClickEvent(QMouseEvent *event) override;

    /** @brief 处理鼠标按下事件 */
    void mousePressEvent(QMouseEvent *event) override;

    /** @brief 处理鼠标移动事件 */
    void mouseMoveEvent(QMouseEvent *event) override;

    /** @brief 处理鼠标释放事件 */
    void mouseReleaseEvent(QMouseEvent *event) override;

    /** @brief 处理键盘事件 */
    void keyPressEvent(QKeyEvent *event) override;

    /** @brief 处理鼠标离开事件 */
    void leaveEvent(QEvent *event) override;

private:
    /** @brief 绘制网格 */
    void drawGrid(QPainter &p);

    /** @brief 绘制节点 */
    void drawNodes(QPainter &p);

    /** @brief 绘制端口 */
    void drawConnectors(QPainter &p, const NodeModel *node, const QRectF &rect);

    /** @brief 返回端口锚点 */
    QPointF connectorAnchor(const QUuid &connectorId) const;

    /** @brief 绘制连线 */
    void drawWires(QPainter &p);

    /** @brief 绘制交互提示 */
    void drawInteractionHints(QPainter &p);

    /** @brief 更新悬停状态 */
    void updateHoverState(const QPointF &pos);

    /** @brief 构建贝塞尔曲线路径 */
    QPainterPath buildBezierPath(const QPointF &from, const QPointF &to, const QVector<QPointF> &constraints) const;

    /** @brief 绘制连线的约束点 */
    void drawConstraintPoints(QPainter &p, const QUuid &wireId, const QVector<QPointF> &constraints);

    /** @brief 开始拖动约束点 */
    bool beginConstraintDrag(const QPointF &pos);

    /** @brief 更新拖动约束点 */
    void updateConstraintDrag(const QPointF &pos);

    /** @brief 查找指定位置的连线段 */
    bool findWireSegmentAt(const QPointF &pos, WireModel **outWire, int *outInsertIndex) const;

    /** @brief 查找指定位置的约束点 */
    bool findConstraintAt(const QPointF &pos, QUuid *outWireId, int *outIndex) const;

    /** @brief 计算点到线段的距离 */
    static qreal distanceToSegment(const QPointF &p, const QPointF &a, const QPointF &b);

    void updateWireHoverText(const QPointF &pos);

    /** @brief 返回指定连线的模型 */
    WireModel *wireById(const QUuid &id) const;

    /** @brief 将位置 snap 到网格 */
    QPointF snapToGrid(const QPointF &pos) const;

    /** @brief 应用拖动偏移量 */
    void applyDragDelta(const QPointF &delta);

    /** @brief 选择矩形区域内的节点 */
    void selectNodesInRect(const QRectF &rect, bool additive);

    /** @brief 切换节点选择状态 */
    void toggleSelection(const QUuid &id);

    /** @brief 返回指定位置的节点模型 */
    NodeModel *nodeAt(const QPointF &pos, QPointF *offset) const;

    /** @brief 返回指定位置的连接器模型 */
    ConnectorModel *connectorAt(const QPointF &pos, NodeModel **outNode) const;

private:
    static constexpr qreal kNodeWidth = 140.0;  // 节点宽度
    static constexpr qreal kNodeHeight = 72.0;      // 节点高度
    static constexpr qreal kConnectorRadius = 4.0;  // 连接器半径

    ProjectModel *m_model{nullptr};  // 项目模型
    ProjectController *m_controller{nullptr};   // 项目控制器
    std::function<void(const QString &)> m_statusCallback;  // 状态更新回调函数
    QSet<QUuid> m_selectedNodeIds;  // 当前选中的节点ID集合
    QUuid m_hoverNodeId;    // 当前悬停的节点ID
    QUuid m_hoverConnectorId;   // 当前悬停的连接器ID
    QUuid m_hoverWireId;    // 当前悬停的连线ID
    QUuid m_hoverConstraintWire;    // 当前悬停的约束点所属的线ID
    int m_hoverConstraintIndex{-1}; // 当前悬停的约束点索引
    QString m_hoverText;     // 当前悬停的文本
    QPointF m_hoverTextPos; // 当前悬停的文本位置
    bool m_draggingNode{false};     // 是否正在拖动节点
    NodeModel *m_dragNode{nullptr}; // 当前拖动的节点
    QPointF m_dragOffset;   // 当前拖动的节点的偏移量
    bool m_draggingWire{false}; // 是否正在拖动连线
    QUuid m_wireFromConnector;  // 当前拖动的连线的源端口ID
    QPointF m_wireDragPos;      // 当前拖动的连线的鼠标位置
    bool m_draggingConstraint{false};   // 是否正在拖动约束点
    QUuid m_dragConstraintWire;        // 当前拖动的约束点所属的线ID
    int m_dragConstraintIndex{-1};      // 当前拖动的约束点索引
    QUuid m_selectedConstraintWire;  // 当前选中的约束点所属的线ID
    int m_selectedConstraintIndex{-1};  // 当前选中的约束点索引
    bool m_draggingSelection{false};     // 是否正在拖动选择
    QRectF m_selectionRect; // // 当前选中的矩形区域
    QPointF m_dragStartPos; // 拖动开始时的鼠标位置
    QMetaObject::Connection m_nodeAddedConnection;      // 连接到 ProjectModel::nodeAdded 信号
    QMetaObject::Connection m_wireAddedConnection;      // 连接到 ProjectModel::wireAdded 信号
    QMetaObject::Connection m_nodeRemovedConnection;    // 连接到 ProjectModel::nodeRemoved 信号
    QMetaObject::Connection m_wireRemovedConnection;    // 连接到 ProjectModel::wireRemoved 信号


};