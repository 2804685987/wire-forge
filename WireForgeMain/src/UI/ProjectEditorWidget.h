#pragma once
#include <QWidget>
#include <QSplitter>

class BluePrintView;
class BluePrintScene;
class PropertyPanel;
class QUndoStack;
class QToolBar;
class QDockWidget;

class ProjectEditorWidget : public QWidget
{
Q_OBJECT

public:
    explicit ProjectEditorWidget(QWidget* parent = nullptr);
    ~ProjectEditorWidget() override;

    BluePrintScene* scene() const;
    BluePrintView* view() const;

    void addTestComponent();        // 测试按钮：添加 Connector
    void createTestWire();          // 测试连线

public slots:
    void onSelectionChanged();

private:
    void setupUI();
    void setupToolbar();
    void setupConnections();

private:
    BluePrintView* m_view = nullptr;
    BluePrintScene* m_scene = nullptr;
    PropertyPanel* m_propertyPanel = nullptr;
    QUndoStack* m_undoStack = nullptr;

    // 布局
    QSplitter* m_mainSplitter = nullptr;
    QSplitter* m_rightSplitter = nullptr;

    // 工具栏
    QToolBar* m_toolBar = nullptr;
};