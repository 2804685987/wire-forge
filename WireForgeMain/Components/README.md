# WireForgeMain 组件 JSON 规范

> 说明：当前离线环境无法直接抓取 TE 网页内容，因此 `Connector/Terminal` 目录中的 JSON 先采用**结构化模板**方式生成。
> 后续只要补全 `properties`、`ports`、`compatibility` 即可形成完整组件描述。

## 1. 通用字段

所有组件 JSON 建议遵循以下结构：

- `schemaVersion`：JSON 结构版本，默认 `1`
- `kind`：组件类型，建议为 `Connector` 或 `Terminal`
- `partNumber`：TE 零件号
- `name`：显示名称（通常与零件号一致）
- `manufacturer`：制造商，默认 `TE Connectivity`
- `sourceUrl`：官方产品页地址
- `description`：简要说明
- `properties`：基础属性对象（适合放类别、方向、材料、安装方式等）
- `ports`：端口/触点数组
- `compatibility`：兼容性对象
- `extra`：可选扩展字段（反序列化时会保留）

## 2. `properties` 推荐字段

建议按“基础属性 + 后续扩展”的方式组织：

- `category`
- `series`
- `orientation`
- `mountingType`
- `gender`
- `color`
- `notes`

## 3. `ports` 推荐字段

每个端口建议包含：

- `name`
- `index`
- `direction`
- `contactType`
- `signals`

## 4. `compatibility` 推荐字段

用于描述组件之间的互配关系：

- `connectorPartNumbers`
- `terminalPartNumbers`
- `matingPartNumbers`
- `notes`

## 5. 扩展更多组件的建议

1. 保持 `schemaVersion` 不变或递增。
2. 新组件只需要新增一个 JSON 文件。
3. 若新增字段，不要删除旧字段；使用 `extra` 保持向后兼容。
4. 后续 Terminal 解析器可以复用 Connector 解析器的结构，只把 `kind` 改成 `Terminal`。

## 6. 命名约定

- 连接器：`Components/Connector/<partNumber>.json`
- 端子：`Components/Terminal/<partNumber>.json`

## 7. 当前已生成的样例

- `Components/Connector/770680-4.json`
- `Components/Terminal/770520-1.json`

