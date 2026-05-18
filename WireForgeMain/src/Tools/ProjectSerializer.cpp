#include "ProjectSerializer.h"


#include "Model/ProjectModel.h"
#include "Model/Components/ConnectorModel.h"

#include <QDebug>
#include <QFile>
#include <QIODevice>
#include <QJsonDocument>


namespace {
    static QHash<QString, ProjectSerializer::ConnectorFactory> &connectorFactoryMap()
    {
        static QHash<QString, ProjectSerializer::ConnectorFactory> m;
        return m;
    }
}

bool ProjectSerializer::save(ProjectModel *model, const QString &path)
{
    if (!model) return false;
    QFile f(path);
    if (!f.open(QIODevice::WriteOnly)) {
        qWarning() << "ProjectSerializer::save: cannot open file" << path;
        return false;
    }
    QJsonObject root = model->toJson();
    QJsonDocument doc(root);
    QByteArray data = doc.toJson(QJsonDocument::Indented);
    qint64 written = f.write(data);
    f.close();
    return written == data.size();
}

bool ProjectSerializer::load(ProjectModel *model, const QString &path)
{
    if (!model) return false;
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly)) {
        qWarning() << "ProjectSerializer::load: cannot open file" << path;
        return false;
    }
    QByteArray data = f.readAll();
    f.close();
    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(data, &err);
    if (err.error != QJsonParseError::NoError) {
        qWarning() << "ProjectSerializer::load: parse error" << err.errorString();
        return false;
    }
    if (!doc.isObject()) return false;
    model->fromJson(doc.object());
    return true;
}

void ProjectSerializer::registerConnectorFactory(const QString &type, ConnectorFactory factory)
{
    connectorFactoryMap().insert(type, factory);
}

void ProjectSerializer::unregisterConnectorFactory(const QString &type)
{
    connectorFactoryMap().remove(type);
}

ConnectorModel* ProjectSerializer::createConnectorFromJson(const QJsonObject &obj, QObject *parent)
{
    QString type = obj.value("type").toString();
    if (type.isEmpty()) return nullptr;
    auto it = connectorFactoryMap().find(type);
    if (it == connectorFactoryMap().end()) return nullptr;
    ConnectorModel *c = it.value()(parent);
    if (c) c->fromJson(obj);
    return c;
}


