
## 一、项目整体架构

### 1. 架构模式：MVC分层设计

```
┌─────────────────────────────────────────────────────────────┐
│                      View 层                               │
│  ProjectEditorWidget ──────────────────────────────────────│
├─────────────────────────────────────────────────────────────┤
│                    Controller 层                           │
│  ProjectController (命令分发、Undo/Redo、文件操作协调)       │
├─────────────────────────────────────────────────────────────┤
│                      Model 层                              │
│  ProjectModel ─ NodeModel ─ ConnectorModel                 │
│              ─ WireModel                                   │
│              ─ LayerModel                                  │
├─────────────────────────────────────────────────────────────┤
│                      Tools 层                              │
│  ProjectSerializer │ ComponentAutoDiscovery │ PropertyBag   │
└─────────────────────────────────────────────────────────────┘
```

### 2. 核心设计原则

| 设计原则 | 实现方式 |
|---------|---------|
| **单一职责** | Model负责数据，Controller负责业务协调，View负责渲染 |
| **依赖倒置** | 通过接口抽象（如ConnectorModel虚函数）降低耦合 |
| **开闭原则** | 支持扩展新的连接器类型，无需修改核心框架 |
| **序列化与业务分离** | ProjectSerializer独立处理JSON读写 |

---

## 二、核心数据模型

### 1. 模型层次结构

```
ProjectModel (项目根)
├── m_nodes: QVector<NodeModel*>      // 节点集合
├── m_wires: QVector<WireModel*>      // 连线集合
└── m_layers: QVector<LayerModel*>    // 图层集合

NodeModel (节点)
├── m_id: QUuid                       // 唯一标识
└── m_connectors: QVector<ConnectorModel*> // 端口列表

ConnectorModel (端口/连接器)
├── m_id: QUuid                       // 唯一标识
├── m_name: QString                   // 显示名称
└── m_direction: Direction            // Input/Output

WireModel (连线)
└── m_id: QUuid                       // 唯一标识

LayerModel (图层)
├── m_id: QUuid                       // 唯一标识
├── m_name: QString                   // 图层名称
├── m_visible: bool                   // 可见性
├── m_locked: bool                    // 锁定状态
└── m_props: PropertyBag              // 自定义属性
```

### 2. ConnectorModel 设计要点

**核心特点：**
- 作为抽象基类，子类需实现 `connectorType()`、`toJson()`、`fromJson()`
- 内置唯一ID自动生成机制 (`QUuid::createUuid()`)
- 支持端口方向区分（输入/输出）

**扩展设计：**
```cpp
// 子类必须实现的接口
virtual QString connectorType() const = 0;
virtual QJsonObject toJson() const = 0;
virtual void fromJson(const QJsonObject &obj) = 0;
```

---

## 三、组件系统

### 1. 组件JSON格式规范

项目定义了统一的组件描述格式，支持Connector和Terminal类型：

```json
{
"schemaVersion": 1,
"kind": "Connector",                    // "Connector" 或 "Terminal"
"partNumber": "770680-4",              // 部件编号
"name": "TE Connectivity Connector",   // 显示名称
"manufacturer": "TE Connectivity",     // 制造商
"sourceUrl": "...",                    // 数据源URL
"description": "...",                  // 描述信息
"properties": {                        // 扩展属性（类别、系列等）
    "category": "Automotive",
    "series": "AMP MCP"
},
"ports": [...],                        // 端口/触点定义
"compatibility": {...}                 // 兼容性信息
}
```

### 2. ComponentJson 工具类

提供标准的JSON解析与序列化能力：

| 功能 | 方法 |
|-----|------|
| 解析JSON | `Descriptor fromJson(const QJsonObject &obj)` |
| 序列化为JSON | `QJsonObject toJson(const Descriptor &d)` |
| 从文件加载 | `bool loadFromFile(const QString &path, Descriptor *out)` |
| 保存到文件 | `bool saveToFile(const QString &path, const Descriptor &d)` |
| 类型判断 | `bool isComponentJson(const QJsonObject &obj)` |

**兼容性设计：**
- 自动收集未识别字段到 `extra` 字段，保证版本升级兼容性
- 提供可选的警告列表，便于数据补全

### 3. 组件自动发现机制

`ComponentAutoDiscovery` 实现自动扫描功能：

```cpp
// 扫描目录并按kind分类
static ComponentDiscoveryResult scan(const QString &componentsRoot);
```

**设计目标：**
1. 自动识别未来新增类别（不限于Connector/Terminal）
2. 优先读取JSON内kind字段，缺失时回退到目录名
3. 保留解析警告，便于后续数据补全

---

## 四、序列化系统

### 1. ProjectSerializer 核心功能

| 功能 | 方法 |
|-----|------|
| 保存项目 | `static bool save(ProjectModel *model, const QString &path)` |
| 加载项目 | `static bool load(ProjectModel *model, const QString &path)` |
| 注册连接器工厂 | `static void registerConnectorFactory(...)` |
| 从JSON创建连接器 | `static ConnectorModel* createConnectorFromJson(...)` |

### 2. 工厂注册模式

支持动态扩展连接器类型：

```cpp
using ConnectorFactory = std::function<ConnectorModel*(QObject *parent)>;

// 注册自定义连接器类型
ProjectSerializer::registerConnectorFactory("MyConnector", [](QObject *parent) {
    return new MyConnectorModel(parent);
});
```

---

## 五、属性管理系统

### PropertyBag 设计

提供通用的键值属性存储：

| 方法 | 功能 |
|-----|------|
| `setProperty(key, value)` | 设置属性 |
| `property(key)` | 获取属性 |
| `toJson()` | 序列化为JSON |
| `fromJson(obj)` | 从JSON恢复 |

**应用场景：**
- 节点自定义属性
- 图层附加信息
- 连线参数配置

---

## 六、项目状态管理

### ProjectController 职责

| 功能 | 实现方法 |
|-----|---------|
| 新建项目 | `bool createProject(const QString &path)` |
| 打开项目 | `bool openProject(const QString &path)` |
| 保存项目 | `bool saveProject(const QString &path = QString())` |

**信号机制：**
```cpp
void projectOpened();   // 项目打开/新建完成
void projectSaved();    // 项目保存成功
```

---

## 七、技术亮点总结

| 技术点 | 实现策略 | 优势 |
|-------|---------|------|
| **模型抽象** | ConnectorModel作为抽象基类 | 支持多种连接器类型扩展 |
| **JSON兼容性** | extra字段保留未知属性 | 支持Schema演进 |
| **工厂模式** | ConnectorFactory注册机制 | 动态扩展连接器类型 |
| **组件自动发现** | 目录扫描 + kind分类 | 零配置添加新组件 |
| **MVC分离** | 职责清晰划分 | 便于测试和维护 |
| **UUID标识** | 全局唯一ID | 支持分布式协作和版本控制 |

---

## 八、待完善事项

基于现有代码分析，以下功能尚未完全实现：

1. **WireModel**：当前仅包含ID，缺少源端口/目标端口关联
2. **ProjectModel::addWire**：尚未实现具体连线逻辑
3. **Undo/Redo**：Controller层预留接口但未实现
4. **视图渲染**：ProjectEditorWidget仅为测试界面，缺少实际画布渲染
5. **端口连接验证**：缺少兼容性检查（如Input-Output匹配）

如需进一步了解某个模块的实现细节或需要补充特定功能，请告诉我！
        