/**
 * 完整的示例程序：简易文件管理器
 */

#include "ConsoleCommandManager.h"
#include <iostream>
#include <filesystem>
#include <fstream>
#ifdef _WIN32
    #include <windows.h>
#endif

namespace fs = std::filesystem;

class SimpleFileManager {
public:
    void registerCommands(ConsoleCommand::CommandManager& manager) {
        // 1. 列出文件 - 基础命令+选项+参数
        manager.createCommand("ls", "列出目录内容")
            .setCategory("文件操作")
            .addAlias("list")
            .addAlias("dir")
            .addParameter("path", "目录路径", false, ".", "path")
            .addOption("all", "a", "显示所有文件（包括隐藏文件）", false)
            .addOption("long", "l", "长格式显示", false)
            .addOption("recursive", "r", "递归显示子目录", false)
            .addExample("ls")
            .addExample("ls /home/user -la")
            .setExecutor([this](const ConsoleCommand::CommandContext& ctx) {
                return listFiles(ctx);
            });
            
        // 2. 复制文件 - 多参数+选项
        manager.createCommand("cp", "复制文件")
            .setCategory("文件操作")
            .addAlias("copy")
            .addParameter("source", "源文件", true, "", "file")
            .addParameter("destination", "目标位置", true, "", "path")
            .addOption("force", "f", "强制覆盖", false)
            .addOption("recursive", "r", "递归复制目录", false)
            .addExample("cp file.txt backup/")
            .addExample("cp -r src/ dst/")
            .setExecutor([this](const ConsoleCommand::CommandContext& ctx) {
                return copyFile(ctx);
            });
            
        // 3. 移动文件 - 基本操作
        manager.createCommand("mv", "移动文件")
            .setCategory("文件操作")
            .addAlias("move")
            .addParameter("source", "源文件", true, "", "file")
            .addParameter("destination", "目标位置", true, "", "path")
            .addOption("force", "f", "强制覆盖", false)
            .addExample("mv old.txt new.txt")
            .setExecutor([this](const ConsoleCommand::CommandContext& ctx) {
                return moveFile(ctx);
            });
            
        // 4. 删除文件 - 带确认提示
        manager.createCommand("rm", "删除文件")
            .setCategory("文件操作")
            .addAlias("remove")
            .addAlias("delete")
            .addParameter("target", "目标文件", true, "", "file")
            .addOption("force", "f", "强制删除，不确认", false)
            .addOption("recursive", "r", "递归删除目录", false)
            .addExample("rm file.txt")
            .addExample("rm -rf directory/")
            .setExecutor([this](const ConsoleCommand::CommandContext& ctx) {
                return deleteFile(ctx);
            });
            
        // 5. 创建目录 - 带父目录选项
        manager.createCommand("mkdir", "创建目录")
            .setCategory("文件操作")
            .addAlias("md")
            .addParameter("name", "目录名称", true, "", "path")
            .addOption("parents", "p", "创建父目录", false)
            .addExample("mkdir newdir")
            .addExample("mkdir -p /path/to/newdir")
            .setExecutor([this](const ConsoleCommand::CommandContext& ctx) {
                return createDirectory(ctx);
            });
            
        // 6. 查看文件内容 - 带行号选项
        manager.createCommand("cat", "查看文件内容")
            .setCategory("文件操作")
            .addParameter("file", "文件名", true, "", "file")
            .addOption("number", "n", "显示行号", false)
            .addExample("cat file.txt")
            .addExample("cat -n file.txt")
            .setExecutor([this](const ConsoleCommand::CommandContext& ctx) {
                return viewFile(ctx);
            });
            
        // 7. 文件信息 - 显示详细属性
        manager.createCommand("info", "显示文件信息")
            .setCategory("文件操作")
            .addParameter("path", "文件或目录路径", true, "", "path")
            .addExample("info file.txt")
            .setExecutor([this](const ConsoleCommand::CommandContext& ctx) {
                return showInfo(ctx);
            });
            
        // 8. 数学运算 - 不同类型参数+返回值
        manager.createCommand("calc", "简单数学计算")
            .setCategory("工具")
            .addAlias("calculate")
            .addParameter("a", "第一个数", true, "", "float")
            .addParameter("operation", "运算符号 (+, -, *, /)", true, "", "string")
            .addParameter("b", "第二个数", true, "", "float")
            .addExample("calc 10 + 20")
            .addExample("calc 50 * 3.14")
            .setExecutor([](const ConsoleCommand::CommandContext& ctx) {
                float a = std::stof(ctx.getArgument(0));
                std::string op = ctx.getArgument(1);
                float b = std::stof(ctx.getArgument(2));
                float result = 0;
                
                if (op == "+" || op == "加") {
                    result = a + b;
                } else if (op == "-" || op == "减") {
                    result = a - b;
                } else if (op == "*" || op == "乘") {
                    result = a * b;
                } else if (op == "/" || op == "除") {
                    if (b == 0) {
                        std::cerr << "错误: 除数不能为零" << std::endl;
                        return false;
                    }
                    result = a / b;
                } else {
                    std::cerr << "错误: 不支持的运算符号: " << op << std::endl;
                    return false;
                }
                
                std::cout << "结果: " << result << std::endl;
                return true;
            });
            
        // 9. 回显命令 - 可变参数支持
        manager.createCommand("echo", "回显输入的参数")
            .setCategory("工具")
            .addAlias("print")
            .addParameter("...", "要回显的文本", false, "", "string")
            .addOption("uppercase", "u", "转为大写输出", false)
            .addOption("lowercase", "l", "转为小写输出", false)
            .addExample("echo hello world")
            .addExample("echo -u HELLO WORLD")
            .setExecutor([](const ConsoleCommand::CommandContext& ctx) {
                bool toUpper = ctx.hasFlag("u") || ctx.hasFlag("uppercase");
                bool toLower = ctx.hasFlag("l") || ctx.hasFlag("lowercase");
                
                std::string result;
                for (size_t i = 0; i < ctx.argumentCount(); ++i) {
                    if (i > 0) result += " ";
                    result += ctx.getArgument(i);
                }
                
                if (toUpper) {
                    std::transform(result.begin(), result.end(), result.begin(), ::toupper);
                } else if (toLower) {
                    std::transform(result.begin(), result.end(), result.begin(), ::tolower);
                }
                
                std::cout << result << std::endl;
                return true;
            });
            
        // 10. 自定义帮助 - 覆盖默认帮助
        manager.createCommand("about", "关于此程序")
            .setCategory("信息")
            .setVersion("1.0.0")
            .setAuthor("ConsoleCommandManager")
            .setHelpText("显示程序的版本和作者信息\n\n使用方法: about")
            .setExecutor([](const ConsoleCommand::CommandContext& ctx) {
                std::cout << "简易文件管理器 v1.0.0" << std::endl;
                std::cout << "作者: ConsoleCommandManager" << std::endl;
                std::cout << "基于 ConsoleCommandManager 库构建" << std::endl;
                std::cout << "支持多种文件操作和工具命令" << std::endl;
                return true;
            });
            
        // 11. 布尔参数示例
        manager.createCommand("debug", "调试模式控制")
            .setCategory("系统")
            .addOption("enable", "e", "启用调试模式", false)
            .addOption("disable", "d", "禁用调试模式", false)
            .addOption("level", "l", "设置调试级别 (0-5)", true, "3", "int")
            .addExample("debug -e -l 4")
            .addExample("debug -d")
            .setExecutor([](const ConsoleCommand::CommandContext& ctx) {
                bool enable = ctx.hasFlag("e") || ctx.hasFlag("enable");
                bool disable = ctx.hasFlag("d") || ctx.hasFlag("disable");
                int level = std::stoi(ctx.getOption("level", "3"));
                
                if (enable) {
                    std::cout << "调试模式已启用，级别: " << level << std::endl;
                } else if (disable) {
                    std::cout << "调试模式已禁用" << std::endl;
                } else {
                    std::cout << "当前调试级别: " << level << std::endl;
                }
                return true;
            });
            
        // 12. 批量执行 - 演示processArgLoop
        manager.createCommand("batch", "批量执行命令示例")
            .setCategory("高级")
            .setDescription("演示批量命令执行功能")
            .addExample("batch")
            .setExecutor([&manager](const ConsoleCommand::CommandContext& ctx) {
                // 创建测试目录和文件
                std::cout << "=== 批量执行示例 ===" << std::endl;
                
                // 定义要执行的命令参数
                const char* argv[] = {
                    "mkdir", "test_dir",
                    "echo", "Hello", "World", "-u",
                    "ls", "-la"
                };
                
                // 使用processArgLoop批量执行
                bool success = manager.processArgLoop(3, (char**)argv);
                
                std::cout << "\n=== 批量执行完成 ===" << std::endl;
                return success;
            });
    }
    
private:
    bool listFiles(const ConsoleCommand::CommandContext& ctx) {
        std::string path = ctx.getArgument(0, ".");
        bool showAll = ctx.hasFlag("a") || ctx.hasFlag("all");
        bool longFormat = ctx.hasFlag("l") || ctx.hasFlag("long");
        bool recursive = ctx.hasFlag("r") || ctx.hasFlag("recursive");
        
        try {
            if (!fs::exists(path)) {
                std::cerr << "路径不存在: " << path << std::endl;
                return false;
            }
            
            if (!fs::is_directory(path)) {
                std::cout << path << " (文件)" << std::endl;
                return true;
            }
            
            std::cout << "目录: " << fs::absolute(path).string() << std::endl;
            
            if (recursive) {
                for (const auto& entry : fs::recursive_directory_iterator(path)) {
                    printEntry(entry, showAll, longFormat);
                }
            } else {
                for (const auto& entry : fs::directory_iterator(path)) {
                    printEntry(entry, showAll, longFormat);
                }
            }
            
            return true;
            
        } catch (const fs::filesystem_error& e) {
            std::cerr << "文件系统错误: " << e.what() << std::endl;
            return false;
        }
    }
    
    void printEntry(const fs::directory_entry& entry, bool showAll, bool longFormat) {
        std::string filename = entry.path().filename().string();
        
        // 跳过隐藏文件（除非showAll为true）
        if (!showAll && !filename.empty() && filename[0] == '.') {
            return;
        }
        
        if (longFormat) {
            // 长格式：权限 大小 修改时间 名称
            auto status = entry.status();
            auto ftime = fs::last_write_time(entry);
            auto size = entry.is_directory() ? 0 : fs::file_size(entry);
            
            std::cout << (fs::is_directory(status) ? "d" : "-")
                      << " " << std::setw(10) << size
                      << " " << filename;
        } else {
            // 短格式：仅名称
            std::cout << filename;
        }
        
        std::cout << std::endl;
    }
    
    bool copyFile(const ConsoleCommand::CommandContext& ctx) {
        std::string source = ctx.getArgument(0);
        std::string dest = ctx.getArgument(1);
        bool force = ctx.hasFlag("f") || ctx.hasFlag("force");
        bool recursive = ctx.hasFlag("r") || ctx.hasFlag("recursive");
        
        try {
            if (!fs::exists(source)) {
                std::cerr << "源文件不存在: " << source << std::endl;
                return false;
            }
            
            if (fs::exists(dest) && !force) {
                std::cerr << "目标文件已存在，使用 -f 选项强制覆盖" << std::endl;
                return false;
            }
            
            if (recursive && fs::is_directory(source)) {
                fs::copy(source, dest, fs::copy_options::recursive);
                std::cout << "目录已复制: " << source << " -> " << dest << std::endl;
            } else {
                fs::copy(source, dest);
                std::cout << "文件已复制: " << source << " -> " << dest << std::endl;
            }
            
            return true;
            
        } catch (const fs::filesystem_error& e) {
            std::cerr << "复制失败: " << e.what() << std::endl;
            return false;
        }
    }
    
    bool moveFile(const ConsoleCommand::CommandContext& ctx) {
        std::string source = ctx.getArgument(0);
        std::string dest = ctx.getArgument(1);
        bool force = ctx.hasFlag("f") || ctx.hasFlag("force");
        
        try {
            if (!fs::exists(source)) {
                std::cerr << "源文件不存在: " << source << std::endl;
                return false;
            }
            
            if (fs::exists(dest) && !force) {
                std::cerr << "目标文件已存在，使用 -f 选项强制覆盖" << std::endl;
                return false;
            }
            
            fs::rename(source, dest);
            std::cout << "文件已移动: " << source << " -> " << dest << std::endl;
            
            return true;
            
        } catch (const fs::filesystem_error& e) {
            std::cerr << "移动失败: " << e.what() << std::endl;
            return false;
        }
    }
    
    bool deleteFile(const ConsoleCommand::CommandContext& ctx) {
        std::string target = ctx.getArgument(0);
        bool force = ctx.hasFlag("f") || ctx.hasFlag("force");
        bool recursive = ctx.hasFlag("r") || ctx.hasFlag("recursive");
        
        try {
            if (!fs::exists(target)) {
                std::cerr << "文件不存在: " << target << std::endl;
                return false;
            }
            
            if (!force) {
                std::cout << "确定要删除 " << target << " 吗？(y/N): ";
                std::string response;
                std::getline(std::cin, response);
                
                if (response != "y" && response != "Y") {
                    std::cout << "取消删除" << std::endl;
                    return true;
                }
            }
            
            if (recursive && fs::is_directory(target)) {
                fs::remove_all(target);
                std::cout << "目录已删除: " << target << std::endl;
            } else {
                fs::remove(target);
                std::cout << "文件已删除: " << target << std::endl;
            }
            
            return true;
            
        } catch (const fs::filesystem_error& e) {
            std::cerr << "删除失败: " << e.what() << std::endl;
            return false;
        }
    }
    
    bool createDirectory(const ConsoleCommand::CommandContext& ctx) {
        std::string name = ctx.getArgument(0);
        bool parents = ctx.hasFlag("p") || ctx.hasFlag("parents");
        
        try {
            if (parents) {
                fs::create_directories(name);
                std::cout << "目录已创建（包括父目录）: " << name << std::endl;
            } else {
                fs::create_directory(name);
                std::cout << "目录已创建: " << name << std::endl;
            }
            
            return true;
            
        } catch (const fs::filesystem_error& e) {
            std::cerr << "创建目录失败: " << e.what() << std::endl;
            return false;
        }
    }
    
    bool viewFile(const ConsoleCommand::CommandContext& ctx) {
        std::string filename = ctx.getArgument(0);
        bool showNumbers = ctx.hasFlag("n") || ctx.hasFlag("number");
        
        try {
            std::ifstream file(filename);
            if (!file.is_open()) {
                std::cerr << "无法打开文件: " << filename << std::endl;
                return false;
            }
            
            std::string line;
            int lineNumber = 1;
            
            while (std::getline(file, line)) {
                if (showNumbers) {
                    std::cout << std::setw(4) << lineNumber++ << ": " << line << std::endl;
                } else {
                    std::cout << line << std::endl;
                }
            }
            
            file.close();
            return true;
            
        } catch (const std::exception& e) {
            std::cerr << "读取文件失败: " << e.what() << std::endl;
            return false;
        }
    }
    
    bool showInfo(const ConsoleCommand::CommandContext& ctx) {
        std::string path = ctx.getArgument(0);
        
        try {
            if (!fs::exists(path)) {
                std::cerr << "路径不存在: " << path << std::endl;
                return false;
            }
            
            auto status = fs::status(path);
            auto ftime = fs::last_write_time(path);
            
            std::cout << "路径: " << fs::absolute(path).string() << std::endl;
            std::cout << "类型: " << (fs::is_directory(status) ? "目录" : "文件") << std::endl;
            
            if (!fs::is_directory(status)) {
                std::cout << "大小: " << fs::file_size(path) << " 字节" << std::endl;
            }
            
            std::cout << "权限: ";
            std::cout << ((status.permissions() & fs::perms::owner_read) != fs::perms::none ? "r" : "-");
            std::cout << ((status.permissions() & fs::perms::owner_write) != fs::perms::none ? "w" : "-");
            std::cout << ((status.permissions() & fs::perms::owner_exec) != fs::perms::none ? "x" : "-");
            std::cout << ((status.permissions() & fs::perms::group_read) != fs::perms::none ? "r" : "-");
            std::cout << ((status.permissions() & fs::perms::group_write) != fs::perms::none ? "w" : "-");
            std::cout << ((status.permissions() & fs::perms::group_exec) != fs::perms::none ? "x" : "-");
            std::cout << ((status.permissions() & fs::perms::others_read) != fs::perms::none ? "r" : "-");
            std::cout << ((status.permissions() & fs::perms::others_write) != fs::perms::none ? "w" : "-");
            std::cout << ((status.permissions() & fs::perms::others_exec) != fs::perms::none ? "x" : "-");
            std::cout << std::endl;
            
            return true;
            
        } catch (const fs::filesystem_error& e) {
            std::cerr << "获取信息失败: " << e.what() << std::endl;
            return false;
        }
    }
};

int main(int argc, char* argv[]) {
    // Windows系统下设置控制台为UTF-8编码
    #ifdef _WIN32
        SetConsoleOutputCP(CP_UTF8);
        SetConsoleCP(CP_UTF8);
    #endif
    
    ConsoleCommand::CommandManager manager;
    SimpleFileManager fileManager;
    
    // 配置管理器
    manager.setPrompt("fileman> ");
    manager.setColorOutput(true);
    manager.setVerboseErrors(true);
    manager.setAutoHelp(true);
    
    // 注册命令
    fileManager.registerCommands(manager);
    
    // 添加欢迎信息
    std::cout << "简易文件管理器 v1.0" << std::endl;
    std::cout << "输入 'help' 查看帮助，'list' 列出命令" << std::endl;
    std::cout << "输入 'exit' 退出" << std::endl << std::endl;
    
    if (argc > 1) {
        // 命令行模式
        return manager.processArgs(argc, argv) ? 0 : 1;
    } else {
        // 交互模式
        manager.runInteractive();
        return 0;
    }
}