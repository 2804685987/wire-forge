#include "ComponentAutoDiscovery.h"

#include "Model/Components/ConnectorModel.h"
#include "ComponentJson.h"

#include <QDir>
#include <QDirIterator>
#include <QFileInfo>

namespace {
QString fallbackKindFromPath(const QString &filePath)
{
    QFileInfo fi(filePath);
    QDir parentDir = fi.dir();
    const QString folder = parentDir.dirName();
    if (!folder.isEmpty()) {
        return folder;
    }
    return QStringLiteral("Unknown");
}
}

ComponentDiscoveryResult ComponentAutoDiscovery::scan(const QString &componentsRoot)
{
    ComponentDiscoveryResult result;

    QDir root(componentsRoot);
    if (!root.exists()) {
        ComponentDiscoveryItem item;
        item.filePath = componentsRoot;
        item.kind = QStringLiteral("Error");
        item.error = QStringLiteral("Components 目录不存在");
        result.items.append(item);
        result.groupedByKind[item.kind].append(item);
        return result;
    }

    QDirIterator it(componentsRoot, QStringList() << "*.json", QDir::Files, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        const QString filePath = it.next();

        ComponentDiscoveryItem item;
        item.filePath = filePath;

        ComponentJson::Descriptor descriptor;
        QString error;
        if (ComponentJson::loadFromFile(filePath, &descriptor, &item.warnings, &error)) {
            item.parsed = true;
            item.kind = descriptor.kind.trimmed();
            item.partNumber = descriptor.partNumber;
            item.name = descriptor.name;
            if (item.kind.isEmpty()) {
                item.kind = fallbackKindFromPath(filePath);
                item.warnings.append(QStringLiteral("kind 缺失，已使用目录名回退分类"));
            }
        } else {
            item.parsed = false;
            item.error = error;
            item.kind = fallbackKindFromPath(filePath);
        }

        if (item.kind.isEmpty()) {
            item.kind = QStringLiteral("Unknown");
        }

        result.items.append(item);
        result.groupedByKind[item.kind].append(item);
    }

    return result;
}

