// src/Graphics/HarnessComponent.h
#pragma once
#include <QGraphicsObject>
#include <QUuid>
#include <QJsonObject>
#include <QMap>
#include <QVariant>
#include <QList>

class ComponentPort;
struct SubComponentSlot;

class HarnessComponent : public QGraphicsObject
{
Q_OBJECT
    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
    Q_PROPERTY(QString partNumber READ partNumber WRITE setPartNumber NOTIFY partNumberChanged)

public:
    explicit HarnessComponent(QGraphicsItem* parent = nullptr);
    ~HarnessComponent() override;


    enum { Type = QGraphicsItem::UserType + 1 };
    int type() const override { return Type; }

    // 我们自己使用的字符串类型（重要！）
    virtual QString componentType() const = 0;

    QString name() const { return m_name; }
    void setName(const QString& name);

    QString partNumber() const { return m_partNumber; }
    void setPartNumber(const QString& pn);

    QRectF boundingRect() const override = 0;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override = 0;

    bool isSimplifiedMode() const { return m_simplifiedMode; }
    void setSimplifiedMode(bool enabled);

    virtual QList<ComponentPort*> ports() const { return {}; }
    virtual QList<SubComponentSlot*> subSlots() const { return {}; }

    QVariant property(const QString& key) const;
    void setProperty(const QString& key, const QVariant& value);
    QStringList propertyKeys() const;
    QMap<QString, QVariant> allProperties() const;

    virtual QJsonObject toJson() const;
    virtual bool fromJson(const QJsonObject& obj);

    virtual bool validate() const;

signals:
    void nameChanged(const QString& newName);
    void partNumberChanged(const QString& newPn);
    void propertyChanged(const QString& key, const QVariant& newValue);

protected:
    QUuid m_id;
    QString m_name;
    QString m_partNumber;
    QMap<QString, QVariant> m_customProperties;
    bool m_simplifiedMode = false;

    // 添加以下两个绘制辅助方法的声明：
    void drawSelectionFrame(QPainter* painter) const;
    void drawSimplified(QPainter* painter) const;
};