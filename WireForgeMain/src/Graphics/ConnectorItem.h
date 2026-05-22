// src/Graphics/ConnectorItem.h
#pragma once
#include "HarnessComponent.h"
#include "ComponentPort.h"
#include "SubComponentSlot.h"

class ConnectorItem : public HarnessComponent
{
Q_OBJECT
    Q_PROPERTY(QString partNumber READ partNumber WRITE setPartNumber)

public:
    explicit ConnectorItem(QGraphicsItem* parent = nullptr);
    ~ConnectorItem() override;

    // 【关键修改】使用 componentType 而不是 type
    QString componentType() const override { return "Connector"; }

    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

    QList<ComponentPort*> ports() const override { return m_ports; }
    void addPort(ComponentPort* port);

    QList<SubComponentSlot*> subSlots() const override;
    void addSubSlot(const SubComponentSlot& slot);
    SubComponentSlot* findSlot(const QString& slotId);

    bool attachSubComponent(const QString& slotId, HarnessComponent* subComponent);
    bool detachSubComponent(const QString& slotId);

    bool validate() const override;

    QJsonObject toJson() const override;
    bool fromJson(const QJsonObject& obj) override;

private:
    void drawPorts(QPainter* painter) const; // 新增绘制端子的私有方法

    QList<ComponentPort*> m_ports;
    QList<SubComponentSlot*> m_subSlots;
    QSizeF m_size = QSizeF(120, 80);

};