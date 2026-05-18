#include <QApplication>

#include "MainWindow.h"
#include "Theme.h"
#include "include/ConfigManager.h"
#include "Model/Components/BasicConnectorModel.h"
#include "Tools/ProjectSerializer.h"

void RegisterDefaultConnectors()
{
    ProjectSerializer::registerConnectorFactory(BasicConnectorModel::typeName(), [](QObject *parent) {
        return new BasicConnectorModel(QString(), ConnectorModel::Input, parent);
    });
}

int main(int argc, char *argv[])
{
#ifdef Q_OS_LINUX
    qputenv("QT_QPA_PLATFORM", "xcb");
#endif
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
    QGuiApplication::setHighDpiScaleFactorRoundingPolicy(
            Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);
#endif
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif

    QCoreApplication::setAttribute(Qt::AA_DontCreateNativeWidgetSiblings);

    QApplication app(argc, argv);
    app.setStyle("Fusion");

    RegisterDefaultConnectors();

    QFont font;
    font.setFamilies({"Microsoft YaHei", "PingFang SC", "Segoe UI"});
    font.setPixelSize(14);
    app.setFont(font);

    int theme = ConfigManager::instance().getValue("Window/theme", 0).toInt();
    Theme::setThemeColor(QColor(ConfigManager::instance().getValue("Window/color", "#0066b4").toString()), true);
    Theme::setThemeMode(static_cast<Fluent::ThemeMode>(theme), true);

    MainWindow w;
    w.show();
    return app.exec();
}