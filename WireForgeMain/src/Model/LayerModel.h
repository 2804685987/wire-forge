#pragma once

#include <QObject>
#include <QUuid>
#include <QString>
#include "Core/PropertyBag.h"

/**
 * @brief LayerModel 表示画布上的一层（可见/锁定/顺序）
 */
class LayerModel : public QObject
{
    Q_OBJECT
public:
    /**
     * @brief 构造 LayerModel
     * @param name 层名称
     * @param parent QObject 父对象
     */
    explicit LayerModel(const QString &name = QString(), QObject *parent = nullptr);

    QUuid id() const { return m_id; }
    QString name() const { return m_name; }
    void setName(const QString &n) { m_name = n; }

    bool isVisible() const { return m_visible; }
    void setVisible(bool v) { m_visible = v; }

    bool isLocked() const { return m_locked; }
    void setLocked(bool l) { m_locked = l; }

    PropertyBag* properties() { return &m_props; }
    /** @brief const 访问属性 */
    const PropertyBag* properties() const { return &m_props; }

private:
    QUuid m_id{QUuid::createUuid()};
    QString m_name;
    bool m_visible{true};
    bool m_locked{false};
    PropertyBag m_props{this};
};


