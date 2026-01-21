#include "ConsoleCommandManager.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>

using namespace ConsoleCommand;

/**
 * @class SimpleFileManager
 * @brief 简单的文件管理器示例
 * 
 * 演示了如何使用ConsoleCommandManager框架来构建一个实用的CLI应用程序。
 * 实现了ls、cp、mv、rm、mkdir、cat、info等常见文件操作命令。
 */
class SimpleFileManager {
public:
    /**
     * @brief 初始化文件管理器
     * @return CommandManager实例
     */
    static CommandManager initialize() {
        auto manager = createManager();
        manager.setPrompt("fm> ");
        
        // 注册ls命令
        manager.createCommand("ls", "列出目录内容",
            [this](const CommandContext& ctx) {
                return handleLS(ctx);
            })
            .addParameter("path", "目录路径", false, ".", "path")
            .addOption("long", "l", "长格式显示", false)
            .addOption("all", "a", "显示隐藏文件", false)
            .addExample("ls                 # 列出当前目录")
            .addExample("ls /path/to/dir   # 列出指定目录")
            .addExample("ls -l              # 长格式显示");
        
        // 注册cp命令
        manager.createCommand("cp", "复制文件或目录",
            [this](const CommandContext& ctx) {
                return handleCP(ctx);
            })
            .addParameter("source", "源文件路径", true, "", "file")
            .addParameter("dest", "目标文件路径", true, "", "file")
            .addOption("recursive", "r", "递归复制目录", false)
            .addOption("force", "f", "覆盖目标文件", false)
            .addExample("cp source.txt dest.txt      # 复制文件")
            .addExample("cp -r src_dir dest_dir      # 复制目录");
        
        // 注册mv命令
        manager.createCommand("mv", "移动或重命名文件",
            [this](const CommandContext& ctx) {
                return handleMV(ctx);
            })
            .addParameter("source", "源文件路径", true, "", "file")
            .addParameter("dest", "目标文件路径", true, "", "file")
            .addOption("force", "f", "覆盖目标文件", false)
            .addExample("mv old.txt new.txt   # 重命名文件")
            .addExample("mv file.txt /path/   # 移动文件");
        
        // 注册rm命令
        manager.createCommand("rm", "删除文件或目录",
            [this](const CommandContext& ctx) {
                return handleRM(ctx);
            })
            .addParameter("path", "文件或目录路径", true, "", "path")
            .addOption("recursive", "r", "递归删除目录", false)
            .addOption("force", "f", "强制删除，不提示", false)
            .addExample("rm file.txt           # 删除文件")
            .addExample("rm -r directory/      # 删除目录");
        
        // 注册mkdir命令
        manager.createCommand("mkdir", "创建目录",
            [this](const CommandContext& ctx) {
                return handleMKDIR(ctx);
            })
            .addParameter("path", "目录路径", true, "", "path")
            .addOption("parents", "p", "创建父目录", false)
            .addExample("mkdir newdir                   # 创建目录")
            .addExample("mkdir -p parent/child/subdir   # 创建多层目录");
        
        // 注册cat命令
        manager.createCommand("cat", "显示文件内容",
            [this](const CommandContext& ctx) {
                return handleCAT(ctx);
            })
            .addParameter("file", "文件路径", true, "", "file")
            .addOption("number", "n", "显示行号", false)
            .addExample("cat file.txt      # 显示文件内容")
            .addExample("cat -n file.txt   # 显示文件内容并标记行号");
        
        // 注册info命令
        manager.createCommand("info", "显示文件信息",
            [this](const CommandContext& ctx) {
                return handleINFO(ctx);
            })
            .addParameter("path", "文件或目录路径", true, "", "path")
            .addExample("info file.txt     # 显示文件信息")
            .addExample("info directory/   # 显示目录信息");
        
        return manager;
    }
    
private:
    /**
     * @brief 处理ls命令
     */
    bool handleLS(const CommandContext& ctx) {
        std::string path = ctx.getArgument(0, ".");
        
        try {
            std::cout << "目录内容: " << path << std::endl;
            std::cout << std::string(50, '-') << std::endl;
            
            for (const auto& entry : std::filesystem::directory_iterator(path)) {
                std::string type = entry.is_directory() ? "[DIR]" : "[FILE]";
                std::cout << type << " " << entry.path().filename().string();
                
                if (entry.is_regular_file()) {
                    std::cout << " (" << entry.file_size() << " bytes)";
                }
                std::cout << std::endl;
            }
            
            return true;
        } catch (const std::exception& e) {
            std::cerr << "错误: " << e.what() << std::endl;
            return false;
        }
    }
    
    /**
     * @brief 处理cp命令
     */
    bool handleCP(const CommandContext& ctx) {
        std::string source = ctx.getArgument(0);
        std::string dest = ctx.getArgument(1);
        bool recursive = ctx.hasFlag("r") || ctx.hasFlag("recursive");
        
        try {
            auto opts = std::filesystem::copy_options::none;
            if (recursive) {
                opts = std::filesystem::copy_options::recursive;
            }
            if (ctx.hasFlag("f") || ctx.hasFlag("force")) {
                opts |= std::filesystem::copy_options::overwrite_existing;
            }
            
            std::filesystem::copy(source, dest, opts);
            std::cout << "✓ 复制成功: " << source << " -> " << dest << std::endl;
            return true;
        } catch (const std::exception& e) {
            std::cerr << "✗ 复制失败: " << e.what() << std::endl;
            return false;
        }
    }
    
    /**
     * @brief 处理mv命令
     */
    bool handleMV(const CommandContext& ctx) {
        std::string source = ctx.getArgument(0);
        std::string dest = ctx.getArgument(1);
        
        try {
            std::filesystem::rename(source, dest);
            std::cout << "✓ 移动成功: " << source << " -> " << dest << std::endl;
            return true;
        } catch (const std::exception& e) {
            std::cerr << "✗ 移动失败: " << e.what() << std::endl;
            return false;
        }
    }
    
    /**
     * @brief 处理rm命令
     */
    bool handleRM(const CommandContext& ctx) {
        std::string path = ctx.getArgument(0);
        bool recursive = ctx.hasFlag("r") || ctx.hasFlag("recursive");
        
        try {
            if (std::filesystem::is_directory(path) && !recursive) {
                std::cerr << "✗ 错误: 目录需要使用 -r 选项删除" << std::endl;
                return false;
            }
            
            auto removed = std::filesystem::remove_all(path);
            std::cout << "✓ 删除成功: " << path << " (" << removed << " 项目)" << std::endl;
            return true;
        } catch (const std::exception& e) {
            std::cerr << "✗ 删除失败: " << e.what() << std::endl;
            return false;
        }
    }
    
    /**
     * @brief 处理mkdir命令
     */
    bool handleMKDIR(const CommandContext& ctx) {
        std::string path = ctx.getArgument(0);
        bool parents = ctx.hasFlag("p") || ctx.hasFlag("parents");
        
        try {
            if (parents) {
                std::filesystem::create_directories(path);
            } else {
                std::filesystem::create_directory(path);
            }
            std::cout << "✓ 目录创建成功: " << path << std::endl;
            return true;
        } catch (const std::exception& e) {
            std::cerr << "✗ 创建失败: " << e.what() << std::endl;
            return false;
        }
    }
    
    /**
     * @brief 处理cat命令
     */
    bool handleCAT(const CommandContext& ctx) {
        std::string file = ctx.getArgument(0);
        bool showNumbers = ctx.hasFlag("n") || ctx.hasFlag("number");
        
        try {
            std::ifstream ifs(file);
            if (!ifs.is_open()) {
                std::cerr << "✗ 无法打开文件: " << file << std::endl;
                return false;
            }
            
            std::string line;
            int lineNum = 1;
            
            while (std::getline(ifs, line)) {
                if (showNumbers) {
                    std::cout << std::setw(4) << lineNum++ << " | ";
                }
                std::cout << line << std::endl;
            }
            
            return true;
        } catch (const std::exception& e) {
            std::cerr << "✗ 读取失败: " << e.what() << std::endl;
            return false;
        }
    }
    
    /**
     * @brief 处理info命令
     */
    bool handleINFO(const CommandContext& ctx) {
        std::string path = ctx.getArgument(0);
        
        try {
            if (!std::filesystem::exists(path)) {
                std::cerr << "✗ 路径不存在: " << path << std::endl;
                return false;
            }
            
            auto absPath = std::filesystem::absolute(path);
            std::cout << "路径信息:" << std::endl;
            std::cout << "  绝对路径: " << absPath << std::endl;
            
            if (std::filesystem::is_directory(path)) {
                std::cout << "  类型: 目录" << std::endl;
                int fileCount = 0;
                for (const auto& entry : std::filesystem::directory_iterator(path)) {
                    fileCount++;
                }
                std::cout << "  包含项目数: " << fileCount << std::endl;
            } else if (std::filesystem::is_regular_file(path)) {
                std::cout << "  类型: 文件" << std::endl;
                std::cout << "  大小: " << std::filesystem::file_size(path) << " bytes" << std::endl;
            }
            
            auto lastWrite = std::filesystem::last_write_time(path);
            std::time_t sctp = std::chrono::system_clock::to_time_t(
                std::chrono::time_point_cast<std::chrono::system_clock::duration>(
                    lastWrite - std::filesystem::file_time_type::clock::now() + 
                    std::chrono::system_clock::now()
                )
            );
            std::cout << "  最后修改: " << std::ctime(&sctp);
            
            return true;
        } catch (const std::exception& e) {
            std::cerr << "✗ 获取信息失败: " << e.what() << std::endl;
            return false;
        }
    }
};

/**
 * @brief 主函数
 */
int main(int argc, char* argv[]) {
    SimpleFileManager manager;
    auto cmd = manager.initialize();
    
    if (argc > 1) {
        // 命令行模式
        return cmd.processArgs(argc, argv) ? 0 : 1;
    } else {
        // 交互模式
        std::cout << "ConsoleCommandManager - 文件管理器示例" << std::endl;
        std::cout << "输入 'help' 查看帮助，'list' 列出所有命令" << std::endl;
        cmd.runInteractive();
        return 0;
    }
}
