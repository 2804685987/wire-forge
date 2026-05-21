// ComponentJson.h
// 公共的组件 JSON 描述与解析器（用于 Connector / Terminal 等组件）
#pragma once

#include <QString>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QFile>
#include <QJsonParseError>
#include <QStringList>

namespace ComponentJson {

/**
 * @brief 组件 JSON 的标准描述数据
 *
 * 该结构尽量保持“基础字段 + 扩展字段”的形式，便于后续拓展更多连接器/端子组件。
 */
struct Descriptor
{
    int schemaVersion{1};
    QString kind{QStringLiteral("Connector")};
    QString partNumber;
    QString name;
    QString manufacturer{QStringLiteral("TE Connectivity")};
    QString sourceUrl;
    QString description;

    // 常规属性区域：适合存放 category / series / gender / orientation / package 等基础信息
    QJsonObject properties;

    // 端口/触点信息：Connector/Terminal 均可使用
    QJsonArray ports;

    // 兼容性信息：例如 Terminal 兼容哪些 Connector
    QJsonObject compatibility;

    // 未知字段：反序列化时保留，避免丢失外部扩展内容
    QJsonObject extra;
};

inline QJsonObject toJson(const Descriptor &d)
{
    QJsonObject obj;
    obj.insert(QStringLiteral("schemaVersion"), d.schemaVersion);
    obj.insert(QStringLiteral("kind"), d.kind);
    obj.insert(QStringLiteral("partNumber"), d.partNumber);
    obj.insert(QStringLiteral("name"), d.name);
    obj.insert(QStringLiteral("manufacturer"), d.manufacturer);
    obj.insert(QStringLiteral("sourceUrl"), d.sourceUrl);
    obj.insert(QStringLiteral("description"), d.description);
    obj.insert(QStringLiteral("properties"), d.properties);
    obj.insert(QStringLiteral("ports"), d.ports);
    obj.insert(QStringLiteral("compatibility"), d.compatibility);

    // extra 字段合并到根对象，便于保留未来扩展字段
    for (auto it = d.extra.constBegin(); it != d.extra.constEnd(); ++it) {
        if (!obj.contains(it.key())) {
            obj.insert(it.key(), it.value());
        }
    }
    return obj;
}

inline Descriptor fromJson(const QJsonObject &obj, QStringList *warnings = nullptr)
{
    Descriptor d;
    d.schemaVersion = obj.value(QStringLiteral("schemaVersion")).toInt(1);
    d.kind = obj.value(QStringLiteral("kind")).toString(QStringLiteral("Connector"));
    d.partNumber = obj.value(QStringLiteral("partNumber")).toString();
    d.name = obj.value(QStringLiteral("name")).toString();
    d.manufacturer = obj.value(QStringLiteral("manufacturer")).toString(QStringLiteral("TE Connectivity"));
    d.sourceUrl = obj.value(QStringLiteral("sourceUrl")).toString();
    d.description = obj.value(QStringLiteral("description")).toString();

    if (obj.contains(QStringLiteral("properties")) && obj.value(QStringLiteral("properties")).isObject()) {
        d.properties = obj.value(QStringLiteral("properties")).toObject();
    }
    if (obj.contains(QStringLiteral("ports")) && obj.value(QStringLiteral("ports")).isArray()) {
        d.ports = obj.value(QStringLiteral("ports")).toArray();
    }
    if (obj.contains(QStringLiteral("compatibility")) && obj.value(QStringLiteral("compatibility")).isObject()) {
        d.compatibility = obj.value(QStringLiteral("compatibility")).toObject();
    }

    const QStringList known{
        QStringLiteral("schemaVersion"), QStringLiteral("kind"), QStringLiteral("partNumber"),
        QStringLiteral("name"), QStringLiteral("manufacturer"), QStringLiteral("sourceUrl"),
        QStringLiteral("description"), QStringLiteral("properties"), QStringLiteral("ports"),
        QStringLiteral("compatibility")
    };
    for (auto it = obj.constBegin(); it != obj.constEnd(); ++it) {
        if (!known.contains(it.key())) {
            d.extra.insert(it.key(), it.value());
        }
    }

    if (warnings) {
        if (d.partNumber.isEmpty()) warnings->append(QStringLiteral("缺少 partNumber"));
        if (d.sourceUrl.isEmpty()) warnings->append(QStringLiteral("缺少 sourceUrl"));
        if (d.name.isEmpty()) warnings->append(QStringLiteral("缺少 name"));
    }
    return d;
}

inline bool loadFromFile(const QString &path, Descriptor *out, QStringList *warnings = nullptr, QString *error = nullptr)
{
    if (!out) {
        if (error) *error = QStringLiteral("out 不能为空");
        return false;
    }
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly)) {
        if (error) *error = QStringLiteral("无法打开文件：") + path;
        return false;
    }
    const QByteArray data = f.readAll();
    QJsonParseError parseError{};
    const QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);
    if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
        if (error) *error = QStringLiteral("JSON 解析失败：") + parseError.errorString();
        return false;
    }
    *out = fromJson(doc.object(), warnings);
    return true;
}

inline bool saveToFile(const QString &path, const Descriptor &d, QString *error = nullptr)
{
    QFile f(path);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        if (error) *error = QStringLiteral("无法写入文件：") + path;
        return false;
    }
    QJsonDocument doc(toJson(d));
    const QByteArray data = doc.toJson(QJsonDocument::Indented);
    if (f.write(data) != data.size()) {
        if (error) *error = QStringLiteral("文件写入不完整：") + path;
        return false;
    }
    return true;
}

inline bool isComponentJson(const QJsonObject &obj)
{
    const QString kind = obj.value(QStringLiteral("kind")).toString();
    return kind == QStringLiteral("Connector") || kind == QStringLiteral("Terminal");
}

} // namespace ComponentJson

