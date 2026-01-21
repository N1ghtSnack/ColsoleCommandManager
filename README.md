# ConsoleCommandManager

一个功能完整、灵活易用的C++命令行交互系统，支持命令注册、参数解析、选项处理、自动帮助生成等功能。适用于任何需要命令行接口的C++应用程序。

## 主要特性

- ✅ **完整的命令元数据支持**：每个命令、参数、选项都有详细的描述信息
- ✅ **智能帮助系统**：自动为每个命令生成帮助文档，支持全局和特定命令帮助
- ✅ **多种调用模式**：支持交互式命令行、直接参数传递、批处理等多种使用方式
- ✅ **类型安全**：参数验证和类型检查
- ✅ **可扩展性**：易于添加新命令和功能
- ✅ **完善的错误处理**：用户友好的错误提示
- ✅ **跨平台支持**：适用于Windows、Linux、macOS等系统

## 快速开始

### 安装

只需将`ConsoleCommandManager.h`头文件包含到您的项目中即可：

```cpp
#include "ConsoleCommandManager.h"
```

### 基本用法

```cpp
#include "ConsoleCommandManager.h"
#include <iostream>

int main() {
    // 创建命令管理器
    ConsoleCommand::CommandManager manager;
    
    // 注册命令
    manager.createCommand("echo", "回显输入的参数")
        .addParameter("text", "要回显的文本", true)
        .setExecutor([](const ConsoleCommand::CommandContext& ctx) {
            for (size_t i = 0; i < ctx.argumentCount(); ++i) {
                if (i > 0) std::cout << " ";
                std::cout << ctx.getArgument(i);
            }
            std::cout << std::endl;
            return true;
        });
    
    // 运行交互模式
    manager.runInteractive();
    return 0;
}
```

## 核心概念

### 1. CommandManager

命令管理器是整个系统的核心，负责管理所有注册的命令，提供命令注册、查找、执行等功能。

**主要方法**：
- `createCommand(name, description)`：创建并注册命令
- `registerCommand(cmdDef)`：注册完整的命令定义
- `processCommand(context)`：处理单个命令
- `processArgs(argc, argv)`：处理main函数参数
- `processString(input)`：处理字符串命令
- `runInteractive()`：运行交互式命令行模式

### 2. CommandDefinition

命令定义封装了命令的完整信息，包括名称、描述、参数、选项、执行器等。

**主要方法**：
- `setCategory(category)`：设置命令分类
- `addAlias(alias)`：添加命令别名
- `addParameter(name, description, required, defaultValue, type)`：添加参数
- `addOption(name, shortName, description, requiresValue, defaultValue, valueType)`：添加选项
- `addExample(example)`：添加使用示例
- `setExecutor(executor)`：设置命令执行器

### 3. CommandContext

命令执行上下文封装了命令执行时所需的所有信息，包括命令名称、参数、选项等。

**主要方法**：
- `getCommandName()`：获取命令名称
- `getArgument(index, defaultValue)`：获取指定位置的参数
- `getOption(name, defaultValue)`：获取选项值
- `hasFlag(name)`：检查是否存在某个标志选项

## 详细用法

### 1. 命令注册

#### 基本命令注册

```cpp
manager.createCommand("hello", "输出Hello World")
    .setExecutor([](const ConsoleCommand::CommandContext& ctx) {
        std::cout << "Hello World!" << std::endl;
        return true;
    });
```

#### 带参数的命令

```cpp
manager.createCommand("add", "计算两个数的和")
    .addParameter("a", "第一个数", true, "", "int")
    .addParameter("b", "第二个数", true, "", "int")
    .setExecutor([](const ConsoleCommand::CommandContext& ctx) {
        int a = std::stoi(ctx.getArgument(0));
        int b = std::stoi(ctx.getArgument(1));
        std::cout << "结果: " << a + b << std::endl;
        return true;
    });
```

#### 带选项的命令

```cpp
manager.createCommand("ls", "列出目录内容")
    .addParameter("path", "目录路径", false, ".", "path")
    .addOption("all", "a", "显示所有文件", false)
    .addOption("long", "l", "长格式显示", false)
    .setExecutor([](const ConsoleCommand::CommandContext& ctx) {
        std::string path = ctx.getArgument(0);
        bool showAll = ctx.hasFlag("a") || ctx.hasFlag("all");
        bool longFormat = ctx.hasFlag("l") || ctx.hasFlag("long");
        // 实现列出文件逻辑
        return true;
    });
```

### 2. 命令执行模式

#### 交互式模式

```cpp
manager.runInteractive();
```

#### 命令行模式

```cpp
int main(int argc, char* argv[]) {
    ConsoleCommand::CommandManager manager;
    // 注册命令...
    return manager.processArgs(argc, argv) ? 0 : 1;
}
```

#### 字符串命令处理

```cpp
manager.processString("ls -la");
```

### 3. 命令元数据

#### 添加分类和别名

```cpp
manager.createCommand("copy", "复制文件")
    .setCategory("文件操作")
    .addAlias("cp")
    .addParameter("source", "源文件", true, "", "file")
    .addParameter("dest", "目标位置", true, "", "path")
    .setExecutor([](const ConsoleCommand::CommandContext& ctx) {
        // 实现复制逻辑
        return true;
    });
```

#### 添加使用示例

```cpp
manager.createCommand("grep", "搜索文本")
    .addParameter("pattern", "搜索模式", true)
    .addParameter("file", "要搜索的文件", true, "", "file")
    .addOption("ignore-case", "i", "忽略大小写", false)
    .addOption("line-number", "n", "显示行号", false)
    .addExample("grep hello file.txt")
    .addExample("grep -i hello file.txt")
    .addExample("grep -n hello file.txt")
    .setExecutor([](const ConsoleCommand::CommandContext& ctx) {
        // 实现搜索逻辑
        return true;
    });
```

### 4. 帮助系统

#### 自动生成帮助

每个命令会自动生成帮助文档，用户可以通过`help <命令名>`查看详细帮助：

```
命令: ls (别名: list, dir)
描述: 列出目录内容
分类: 文件操作

用法: ls [path] [选项...]

参数:
  [path]               目录路径 [默认: .] (path)

选项:
  -a, --all            显示所有文件（包括隐藏文件）
  -l, --long           长格式显示
  -r, --recursive      递归显示子目录

示例:
  ls
  ls /home/user -la
```

## 高级特性

### 1. 全局选项

系统内置了一些全局选项：
- `-h, --help`：显示帮助信息
- `-v, --verbose`：详细输出模式
- `-q, --quiet`：安静模式
- `-V, --version`：显示版本信息
- `-c, --config`：指定配置文件

### 2. 命令分类

可以将命令按分类组织，便于用户查找：

```cpp
manager.createCommand("ls", "列出目录内容").setCategory("文件操作");
manager.createCommand("cp", "复制文件").setCategory("文件操作");
manager.createCommand("mv", "移动文件").setCategory("文件操作");
manager.createCommand("rm", "删除文件").setCategory("文件操作");

manager.createCommand("add", "计算两个数的和").setCategory("数学运算");
manager.createCommand("sub", "计算两个数的差").setCategory("数学运算");
manager.createCommand("mul", "计算两个数的积").setCategory("数学运算");
manager.createCommand("div", "计算两个数的商").setCategory("数学运算");
```

### 3. 可变参数

通过将最后一个参数命名为`...`来支持可变参数：

```cpp
manager.createCommand("echo", "回显输入的参数")
    .addParameter("...", "要回显的文本", true)
    .setExecutor([](const ConsoleCommand::CommandContext& ctx) {
        for (size_t i = 0; i < ctx.argumentCount(); ++i) {
            if (i > 0) std::cout << " ";
            std::cout << ctx.getArgument(i);
        }
        std::cout << std::endl;
        return true;
    });
```

## 配置选项

```cpp
// 设置命令行提示符
manager.setPrompt("$ ");

// 启用/禁用彩色输出
manager.setColorOutput(true);

// 启用/禁用详细错误信息
manager.setVerboseErrors(true);

// 启用/禁用自动帮助
manager.setAutoHelp(true);

// 设置最大建议命令数
manager.setMaxSuggestions(5);
```

## 跨平台兼容性

### Windows系统

在Windows系统下，控制台默认使用GBK编码，需要设置控制台为UTF-8模式：

```cpp
#ifdef _WIN32
    #include <windows.h>
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif
```

### 编译选项

使用g++编译时，建议添加以下选项以支持UTF-8：

```bash
g++ -std=c++17 -finput-charset=utf-8 -fexec-charset=utf-8 main.cpp -o main
```

## 示例程序

### 简易文件管理器

`example.cpp`文件包含了一个完整的简易文件管理器示例，展示了ConsoleCommandManager的主要功能：

- `ls`：列出目录内容
- `cp`：复制文件
- `mv`：移动文件
- `rm`：删除文件
- `mkdir`：创建目录
- `cat`：查看文件内容
- `info`：显示文件信息

## 常见问题

### Q: 如何添加自定义帮助文本？

A: 使用`setHelpText`方法：

```cpp
manager.createCommand("help", "显示帮助信息")
    .setHelpText("自定义帮助文本")
    .setExecutor([](const ConsoleCommand::CommandContext& ctx) {
        // 实现帮助逻辑
        return true;
    });
```

### Q: 如何处理命令执行错误？

A: 命令执行器返回`false`表示执行失败，系统会自动显示错误提示：

```cpp
manager.createCommand("div", "计算两个数的商")
    .addParameter("a", "被除数", true, "", "int")
    .addParameter("b", "除数", true, "", "int")
    .setExecutor([](const ConsoleCommand::CommandContext& ctx) {
        int a = std::stoi(ctx.getArgument(0));
        int b = std::stoi(ctx.getArgument(1));
        if (b == 0) {
            std::cerr << "错误: 除数不能为零" << std::endl;
            return false;
        }
        std::cout << "结果: " << (a / b) << std::endl;
        return true;
    });
```

## 许可证

MIT License

## 作者

Maxim2022013262@gmail.com

## 版本历史

- v0.0.0 (2026-01-16)：初始版本
