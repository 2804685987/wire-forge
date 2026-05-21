#include "PropertyPanel.h"
#include "../../Graphics/HarnessComponent.h"
#include "../../Graphics/ConnectorItem.h"
#include "../../Graphics/WireItem.h"
#include <QLineEdit>
#include <QComboBox>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QCheckBox>
#include <QLabel>
#include <QGroupBox>
#include <QVBoxLayout>

PropertyPanel::PropertyPanel(QWidget* parent) : QWidget(parent)
{
    auto mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(8, 8, 8, 8);

    m_scrollArea = new QScrollArea(this);
    m_scrollArea->setWidgetResizable(true);
    m_contentWidget = new QWidget();
    m_formLayout = new QFormLayout(m_contentWidget);
    m_formLayout->setLabelAlignment(Qt::AlignRight);
    m_formLayout->setSpacing(10);

    m_scrollArea->setWidget(m_contentWidget);
    mainLayout->addWidget(m_scrollArea);

    // 默认显示提示
    clear();
}

void PropertyPanel::setSelectedComponent(HarnessComponent* component)
{
    if (m_currentComponent == component) return;

    // 断开旧连接
    if (m_currentComponent) {
        disconnect(m_currentComponent, nullptr, this, nullptr);
    }

    m_currentComponent = component;
    m_propertyEditors.clear();

    // 清空旧界面
    QLayoutItem* item;
    while ((item = m_formLayout->takeAt(0)) != nullptr) {
        delete item->widget();
        delete item;
    }

    if (!component) {
        auto label = new QLabel("没有选中组件", this);
        label->setAlignment(Qt::AlignCenter);
        m_formLayout->addRow(label);
        return;
    }

    // 连接信号
    connect(component, &HarnessComponent::propertyChanged, this, &PropertyPanel::onComponentPropertyChanged);
    connect(component, &HarnessComponent::nameChanged, this, &PropertyPanel::onNameChanged);
    connect(component, &HarnessComponent::partNumberChanged, this, &PropertyPanel::onPartNumberChanged);

    createGeneralProperties();
    createDynamicProperties();

    // 根据类型显示特定属性
    if (auto connector = qobject_cast<ConnectorItem*>(component)) {
        createConnectorProperties();
    } else if (auto wire = qobject_cast<WireItem*>(component)) {
        createWireProperties();
    }
}

void PropertyPanel::createGeneralProperties()
{
    auto nameEdit = new QLineEdit(m_currentComponent->name());
    connect(nameEdit, &QLineEdit::textChanged, m_currentComponent, &HarnessComponent::setName);
    m_formLayout->addRow("名称:", nameEdit);
    m_propertyEditors["name"] = nameEdit;

    auto pnEdit = new QLineEdit(m_currentComponent->partNumber());
    connect(pnEdit, &QLineEdit::textChanged, m_currentComponent, &HarnessComponent::setPartNumber);
    m_formLayout->addRow("零件号:", pnEdit);
}

void PropertyPanel::createDynamicProperties()
{
    if (!m_currentComponent) return;

    auto props = m_currentComponent->allProperties();
    for (auto it = props.begin(); it != props.end(); ++it) {
        addPropertyEditor(it.key(), it.value());
    }
}

QWidget* PropertyPanel::createEditorForType(const QString& key, const QVariant& value)
{
    switch (value.userType()) {
        case QMetaType::QString: {
            auto edit = new QLineEdit(value.toString());
            connect(edit, &QLineEdit::textChanged, this,
                    [this, key](const QString& text) {
                        if (m_currentComponent) m_currentComponent->setProperty(key, text);
                    });
            return edit;
        }
        case QMetaType::Int:
        case QMetaType::UInt: {
            auto spin = new QSpinBox();
            spin->setValue(value.toInt());
            connect(spin, &QSpinBox::valueChanged, this,
                    [this, key](int v) { if (m_currentComponent) m_currentComponent->setProperty(key, v); });
            return spin;
        }
        case QMetaType::Double: {
            auto dspin = new QDoubleSpinBox();
            dspin->setValue(value.toDouble());
            connect(dspin, &QDoubleSpinBox::valueChanged, this,
                    [this, key](double v) { if (m_currentComponent) m_currentComponent->setProperty(key, v); });
            return dspin;
        }
        case QMetaType::Bool: {
            auto check = new QCheckBox();
            check->setChecked(value.toBool());
            connect(check, &QCheckBox::toggled, this,
                    [this, key](bool checked) { if (m_currentComponent) m_currentComponent->setProperty(key, checked); });
            return check;
        }
        default:
            return new QLabel(value.toString());
    }
}

void PropertyPanel::addPropertyEditor(const QString& key, const QVariant& value)
{
    QWidget* editor = createEditorForType(key, value);
    m_formLayout->addRow(key + ":", editor);
    m_propertyEditors[key] = editor;
}

void PropertyPanel::clear()
{
    setSelectedComponent(nullptr);
}