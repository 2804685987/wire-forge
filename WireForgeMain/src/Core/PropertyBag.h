#pragma once

#include <QObject>
#include <QVariant>
#include <QMap>
#include <QJsonObject>

/**
 * @brief PropertyBag 提供可序列化的键-值属性集合
 *
 * 用于存储节点、连线、图层等对象的自定义属性。支持将属性转换为 QJsonObject
 * 以便序列化到项目文件中，或从 QJsonObject 恢复属性。
 */
class PropertyBag : public QObject
{
    Q_OBJECT
public:
    /**
     * @brief 构造 PropertyBag
     * @param parent QObject 父对象
     */
    explicit PropertyBag(QObject *parent = nullptr);

    /**
     * @brief 设置属性
     * @param key 属性名
     * @param value 属性值（QVariant，可为基本类型或字符串）
     */
    void setProperty(const QString &key, const QVariant &value);

    /**
     * @brief 返回指定属性的值
     * @param key 属性名
     * @return 属性值（若不存在返回空 QVariant）
     */
    QVariant property(const QString &key) const;

    /** @brief 返回当前所有属性的副本 */
    QMap<QString, QVariant> properties() const;

    /**
     * @brief 将属性集合序列化为 QJsonObject
     * @return QJsonObject 表示的属性集
     */
    QJsonObject toJson() const;

    /**
     * @brief 从 QJsonObject 恢复属性集合（会清空现有属性）
     * @param obj JSON 对象
     */
    void fromJson(const QJsonObject &obj);

private:
    QMap<QString, QVariant> m_props;
};


