#include "PropertyBag.h"

#include <QJsonValue>

/**
 * @brief 构造 PropertyBag，对象父子关系初始化
 */
PropertyBag::PropertyBag(QObject *parent)
    : QObject(parent)
{
}

/**
 * @brief 设置属性（插入或覆盖）
 */
void PropertyBag::setProperty(const QString &key, const QVariant &value)
{
    m_props.insert(key, value);
}

/**
 * @brief 获取属性值，若不存在返回空 QVariant
 */
QVariant PropertyBag::property(const QString &key) const
{
    return m_props.value(key, QVariant());
}

/** @brief 返回所有属性的副本 */
QMap<QString, QVariant> PropertyBag::properties() const
{
    return m_props;
}

/**
 * @brief 将属性集合转换为 QJsonObject 用于序列化
 *
 * 目前对常见的基本类型做了简单处理，复杂类型会被转换为字符串或 QVariant
 */
QJsonObject PropertyBag::toJson() const
{
    QJsonObject obj;
    for (auto it = m_props.constBegin(); it != m_props.constEnd(); ++it) {
        obj.insert(it.key(), QJsonValue::fromVariant(it.value()));
    }
    return obj;
}

/**
 * @brief 从 QJsonObject 恢复属性集合，现有属性会被清空
 */
void PropertyBag::fromJson(const QJsonObject &obj)
{
    m_props.clear();
    for (auto it = obj.begin(); it != obj.end(); ++it) {
        const QString &k = it.key();
        const QJsonValue &v = it.value();
        if (v.isBool()) m_props.insert(k, QVariant(v.toBool()));
        else if (v.isDouble()) m_props.insert(k, QVariant(v.toDouble()));
        else if (v.isString()) m_props.insert(k, QVariant(v.toString()));
        else if (v.isNull()) m_props.insert(k, QVariant());
        else m_props.insert(k, QVariant(v.toVariant()));
    }
}


