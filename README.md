# MC Version Control

**Minecraft 基岩版 AddOn 版本控制器 / Minecraft Bedrock AddOn Version Control**

一个基于 Qt6 Widgets 的 Windows 桌面应用，用于管理 Minecraft 基岩版 AddOn 项目的版本备份与恢复。

---

## English

### Features

- **Project Recognition** — Recursively scan directories for `manifest.json` to identify behavior packs and resource packs
- **Version Management** — Create `.zip` backups of pack directories with version labels
- **Restore & Rollback** — Restore AddOn packs to any previously backed-up version
- **Dual Theme** — Dark and light themes, auto-follows Windows system theme
- **Windows 11 Native** — Rounded corners, snap layout support, acrylic blur effect

### Build Requirements

- **Qt 6.x** (Widgets, Core, Gui)
- **CMake 3.16+**
- **MinGW 64-bit** (or MSVC 2019+)
- **Windows 10/11**

### Build

```bash
mkdir build && cd build
cmake .. -G "MinGW Makefiles" -DCMAKE_PREFIX_PATH="path/to/Qt/6.x.x/mingw_64"
mingw32-make -j$(nproc)
```

### Usage

1. **File → Add Project** — Select an AddOn project directory containing `manifest.json`
2. **Right-click project → Backup** — Create a `.zip` backup of all pack directories
3. **Right-click version → Restore** — Roll back to a previous backup
4. **Right-click version → Delete** — Remove a backup

### Known Issues / Potential Bugs

| # | Issue | Severity | Notes |
|---|-------|----------|-------|
| 1 | PowerShell dependency for zip | Medium | Backup/restore relies on `PowerShell Compress-Archive`/`Expand-Archive`. If PowerShell is unavailable, backup falls back to directory copy. |
| 2 | Large project backup time | Low | No progress bar for backup/restore operations. Large AddOn projects may appear to freeze during compression. |
| 3 | No incremental backup | Low | Each backup is a full copy of all pack directories. Disk usage grows linearly. |
| 4 | Concurrent backup safety | Medium | No file locking during backup. Modifying project files while a backup is running may cause corruption. |
| 5 | Deeply nested paths | Low | `Compress-Archive` may fail on paths > 260 characters (Windows MAX_PATH limit). |
| 6 | Project path changes | Medium | If a project directory is moved/renamed outside the app, the project entry becomes stale. Refresh will not fix stale paths. |
| 7 | QSS theme edge cases | Low | Some Qt system dialogs (QMessageBox, QFileDialog) use the system theme and may not match the app's dark/light theme perfectly. |
| 8 | No multi-language persistence | Low | Language selection is not saved between sessions (currently only hardcoded Chinese UI strings). |
| 9 | manifest.json parse errors | Medium | Malformed or non-standard manifest.json files may cause the app to fail loading a project silently. |
| 10 | Backup file name collisions | Low | If two backups are created within the same second, the timestamp-based file names may collide. |
| 11 | 7z format not supported | Low | Spec originally called for `.7z` format, but `.zip` is used for portability. No migration path for existing `.7z` backups. |
| 12 | No drag-and-drop support | Low | Projects must be added via File menu only — no drag-and-drop onto the window. |

---

## 中文

### 功能

- **项目识别** — 递归扫描目录中的 `manifest.json`，自动识别行为包和资源包
- **版本管理** — 以 `.zip` 格式备份包目录，支持自定义版本标签
- **恢复与回滚** — 将 AddOn 包恢复到任意历史版本
- **双主题** — 暗色 / 亮色主题，自动跟随 Windows 系统主题
- **Windows 11 原生体验** — 圆角窗口、分屏布局支持、亚克力模糊效果

### 编译环境

- **Qt 6.x** (Widgets, Core, Gui)
- **CMake 3.16+**
- **MinGW 64-bit**（或 MSVC 2019+）
- **Windows 10/11**

### 编译

```bash
mkdir build && cd build
cmake .. -G "MinGW Makefiles" -DCMAKE_PREFIX_PATH="path/to/Qt/6.x.x/mingw_64"
mingw32-make -j$(nproc)
```

### 使用方法

1. **文件 → 添加项目** — 选择包含 `manifest.json` 的 AddOn 项目目录
2. **右键项目 → 备份** — 创建所有包目录的 `.zip` 备份
3. **右键版本 → 恢复** — 回滚到之前的备份版本
4. **右键版本 → 删除** — 删除指定备份

### 已知问题 / 潜在 Bug

| # | 问题 | 严重程度 | 说明 |
|---|------|----------|------|
| 1 | 依赖 PowerShell | 中 | 备份/恢复依赖 `PowerShell Compress-Archive`/`Expand-Archive`。如果 PowerShell 不可用，备份会降级为目录复制。 |
| 2 | 大项目备份无进度条 | 低 | 备份/恢复操作无进度提示，大型 AddOn 项目压缩时可能看起来像卡死。 |
| 3 | 无增量备份 | 低 | 每次备份都是所有包目录的完整复制，磁盘占用线性增长。 |
| 4 | 并发备份安全性 | 中 | 备份过程中无文件锁定。备份运行时修改项目文件可能导致数据损坏。 |
| 5 | 超长路径问题 | 低 | `Compress-Archive` 在路径超过 260 字符时可能失败（Windows MAX_PATH 限制）。 |
| 6 | 项目路径变更 | 中 | 如果在外部移动/重命名项目目录，项目条目会失效。刷新无法修复过期的路径。 |
| 7 | QSS 主题边缘问题 | 低 | 部分 Qt 系统对话框（QMessageBox、QFileDialog）使用系统主题，可能与应用的暗色/亮色主题不完全匹配。 |
| 8 | 多语言未持久化 | 低 | 语言选择不会在会话间保存（当前仅硬编码中文 UI 字符串）。 |
| 9 | manifest.json 解析异常 | 中 | 格式错误或非标准的 manifest.json 可能导致项目加载静默失败。 |
| 10 | 备份文件名冲突 | 低 | 同一秒内创建两个备份时，基于时间戳的文件名可能冲突。 |
| 11 | 不支持 7z 格式 | 低 | 原规格要求 `.7z` 格式，但出于可移植性考虑使用 `.zip`。没有现有 `.7z` 备份的迁移途径。 |
| 12 | 无拖放支持 | 低 | 只能通过文件菜单添加项目，不支持拖放目录到窗口。 |

---

## License

MIT
