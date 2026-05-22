#pragma once
#include <QWidget>
#include <QFormLayout>
#include <QScrollArea>
#include <QMap>

class HarnessComponent;
class ComponentPort;
struct SubComponentSlot;

class PropertyPanel : public QWidget
{
Q_OBJECT

public:
    explicit PropertyPanel(QWidget* parent = nullptr);
    ~PropertyPanel() override;

    void setSelectedComponent(HarnessComponent* component);
    void clear();

    HarnessComponent* currentComponent() const { return m_currentComponent; }

signals:
    void propertyChanged(HarnessComponent* component, const QString& key, const QVariant& value);

private slots:
    void onComponentPropertyChanged(const QString& key, const QVariant& value);
    void onNameChanged(const QString& newName);
    void onPartNumberChanged(const QString& newPn);

private:
    void createGeneralProperties();
    void createDynamicProperties();
    void createConnectorProperties();      // Connector 专用（子插槽）
    void createWireProperties();           // Wire 专用
    void createDebugBlackBoxProperties();  // 后期扩展

    void addPropertyEditor(const QString& key, const QVariant& value);
    QWidget* createEditorForType(const QString& key, const QVariant& value);

private:
    HarnessComponent* m_currentComponent = nullptr;

    QScrollArea* m_scrollArea;
    QWidget* m_contentWidget;
    QFormLayout* m_formLayout;

    // 缓存编辑器控件，用于实时更新
    QMap<QString, QWidget*> m_propertyEditors;
};