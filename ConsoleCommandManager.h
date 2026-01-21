/**
 * @file ConsoleCommandManager.h
 * @brief 通用控制台命令管理器头文件
 * @details 提供一个完整、灵活、易用的命令行交互系统，支持命令注册、参数解析、选项处理、自动帮助生成等功能。
 *          适用于任何需要命令行接口的C++应用程序。
 * 
 * 主要特性：
 * 1. 完整的命令元数据支持：每个命令、参数、选项都有详细的描述信息
 * 2. 智能帮助系统：自动为每个命令生成帮助文档，支持全局和特定命令帮助
 * 3. 多种调用模式：支持交互式命令行、直接参数传递、批处理等多种使用方式
 * 4. 类型安全：参数验证和类型检查
 * 5. 可扩展性：易于添加新命令和功能
 * 6. 错误处理：完善的错误处理和用户友好的错误提示
 * 
 * @author Maxim2022013262@gmail.com
 * @version 0.0.0
 * @date 20260116
 * 
 * 使用示例：
 * @code
 * #include "ConsoleCommandManager.h"
 * 
 * int main(int argc, char* argv[]) {
 *     // 创建命令管理器
 *     auto manager = ConsoleCommand::createManager();
 *     
 *     // 注册命令
 *     manager.createCommand("echo", "回显输入的参数",
 *         [](const ConsoleCommand::CommandContext& ctx) {
 *             for (size_t i = 0; i < ctx.argumentCount(); ++i) {
 *                 if (i > 0) std::cout << " ";
 *                 std::cout << ctx.getArgument(i);
 *             }
 *             std::cout << std::endl;
 *             return true;
 *         })
 *         .addParameter("text", "要回显的文本", true);
 *     
 *     // 运行交互模式
 *     manager.runInteractive();
 *     return 0;
 * }
 * @endcode
 */

#ifndef CONSOLE_COMMAND_MANAGER_H
#define CONSOLE_COMMAND_MANAGER_H

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <functional>
#include <memory>
#include <sstream>
#include <algorithm>
#include <optional>
#include <cstring>
#include <iomanip>

namespace ConsoleCommand {

// ============================================================================
// 类型定义和常量
// ============================================================================

/** @brief 参数值类型的字符串表示 */
const std::string TYPE_STRING = "string";    ///< 字符串类型
const std::string TYPE_INTEGER = "int";      ///< 整数类型
const std::string TYPE_FLOAT = "float";      ///< 浮点数类型
const std::string TYPE_BOOL = "bool";        ///< 布尔类型
const std::string TYPE_FILE = "file";        ///< 文件路径类型
const std::string TYPE_PATH = "path";        ///< 路径类型
const std::string TYPE_COMMAND = "command";  ///< 命令名称类型

/** @brief 默认配置常量 */
const std::string DEFAULT_PROMPT = "> ";     ///< 默认命令行提示符
const int DEFAULT_MAX_SUGGESTIONS = 5;       ///< 默认最大建议命令数

// ============================================================================
// 参数定义结构体
// ============================================================================

/**
 * @struct ParameterDefinition
 * @brief 命令行参数定义
 * 
 * 用于定义命令的参数，包括参数名称、描述、是否必需、默认值和类型。
 * 每个参数都应该有完整的描述信息，以便自动生成帮助文档。
 */
struct ParameterDefinition {
    std::string name;        ///< 参数名称（在帮助文档中显示）
    std::string description; ///< 参数描述，说明参数的作用和用法
    bool required;          ///< 是否为必需参数，必需参数必须提供
    std::string defaultValue; ///< 默认值，当参数未提供时使用此值
    std::string type;       ///< 参数值类型，用于类型检查和帮助文档生成
    
    /**
     * @brief 构造函数
     * @param n 参数名称
     * @param desc 参数描述
     * @param req 是否必需，默认为false
     * @param defVal 默认值，默认为空字符串
     * @param t 参数类型，默认为字符串类型
     */
    ParameterDefinition(const std::string& n = "", 
                       const std::string& desc = "",
                       bool req = false,
                       const std::string& defVal = "",
                       const std::string& t = TYPE_STRING)
        : name(n), description(desc), required(req), 
          defaultValue(defVal), type(t) {}
    
    /**
     * @brief 获取参数的用法表示
     * @return 返回参数的用法字符串，如"<filename>"或"[filename]"
     */
    std::string getUsage() const {
        if (required) {
            return "<" + name + ">";
        } else {
            return "[" + name + "]";
        }
    }
};

// ============================================================================
// 选项定义结构体
// ============================================================================

/**
 * @struct OptionDefinition
 * @brief 命令行选项定义
 * 
 * 用于定义命令的选项（flags），包括长选项名、短选项名、描述、是否需要值等。
 * 支持短选项（如 -h）和长选项（如 --help），可以指定是否需要值。
 */
struct OptionDefinition {
    std::string name;        ///< 长选项名（不带"--"前缀）
    std::string shortName;   ///< 短选项名（不带"-"前缀），单字符
    std::string description; ///< 选项描述，说明选项的作用和用法
    bool requiresValue;      ///< 选项是否需要值，true表示需要附加参数值
    std::string defaultValue; ///< 默认值，当选项未提供值但需要值时使用
    std::string valueType;   ///< 选项值的类型描述，用于帮助文档生成
    
    /**
     * @brief 构造函数
     * @param n 长选项名
     * @param sn 短选项名（单字符），默认为空
     * @param desc 选项描述
     * @param reqValue 是否需要值，默认为false
     * @param defVal 默认值，默认为空字符串
     * @param vType 值类型描述，默认为空
     */
    OptionDefinition(const std::string& n = "",
                    const std::string& sn = "",
                    const std::string& desc = "",
                    bool reqValue = false,
                    const std::string& defVal = "",
                    const std::string& vType = "")
        : name(n), shortName(sn), description(desc), 
          requiresValue(reqValue), defaultValue(defVal), valueType(vType) {}
    
    /**
     * @brief 获取选项的用法表示
     * @return 返回选项的用法字符串，如"-h, --help"或"--port <number>"
     */
    std::string getUsage() const {
        std::string usage;
        
        // 添加短选项
        if (!shortName.empty()) {
            usage += "-" + shortName;
            if (!name.empty()) {
                usage += ", ";
            }
        }
        
        // 添加长选项
        if (!name.empty()) {
            usage += "--" + name;
        }
        
        // 如果需要值，添加值占位符
        if (requiresValue) {
            usage += " <" + (valueType.empty() ? "value" : valueType) + ">";
        }
        
        return usage;
    }
};

// ============================================================================
// 命令上下文类
// ============================================================================

/**
 * @class CommandContext
 * @brief 命令执行上下文类
 * 
 * 封装了命令执行时所需的所有信息，包括命令名称、参数、选项等。
 * 提供了便捷的方法来获取和解析命令行输入。
 */
class CommandContext {
private:
    std::string commandName;      ///< 当前执行的命令名称
    std::map<std::string, std::string> options;  ///< 选项键值对映射
    std::map<std::string, std::string> flags;    ///< 标志选项映射（布尔选项）
    std::vector<std::string> args;               ///< 位置参数列表
    std::map<std::string, std::string> metadata; ///< 附加元数据存储
    
public:
    /**
     * @brief 默认构造函数
     */
    CommandContext() = default;
    
    /**
     * @brief 从main函数参数构造命令上下文
     * @param argc 参数个数
     * @param argv 参数数组
     * @details 自动解析命令行参数，区分命令、选项和参数
     */
    explicit CommandContext(int argc, char* argv[]) {
        parseArgs(argc, argv);
    }
    
    /**
     * @brief 从字符串构造命令上下文
     * @param input 命令行字符串
     * @details 将字符串分割为tokens，然后解析为命令上下文
     */
    explicit CommandContext(const std::string& input) {
        parseString(input);
    }
    
    // ========================================================================
    // 公共接口方法
    // ========================================================================
    
    /**
     * @brief 设置命令名称
     * @param name 命令名称
     */
    void setCommandName(const std::string& name) { commandName = name; }
    
    /**
     * @brief 获取命令名称
     * @return 当前命令名称
     */
    const std::string& getCommandName() const { return commandName; }
    
    /**
     * @brief 设置选项值
     * @param key 选项名称（不带"--"前缀）
     * @param value 选项值
     */
    void setOption(const std::string& key, const std::string& value) {
        options[key] = value;
    }
    
    /**
     * @brief 获取选项值
     * @param key 选项名称
     * @return 选项值的可选类型，如果选项不存在返回std::nullopt
     */
    std::optional<std::string> getOption(const std::string& key) const {
        auto it = options.find(key);
        if (it != options.end()) return it->second;
        return std::nullopt;
    }
    
    /**
     * @brief 获取选项值，如果不存在返回默认值
     * @param key 选项名称
     * @param defaultValue 默认值
     * @return 选项值或默认值
     */
    std::string getOption(const std::string& key, const std::string& defaultValue) const {
        auto val = getOption(key);
        return val.has_value() ? val.value() : defaultValue;
    }
    
    /**
     * @brief 设置标志选项（布尔选项）
     * @param flag 标志名称
     */
    void setFlag(const std::string& flag) {
        flags[flag] = "true";
    }
    
    /**
     * @brief 检查是否存在某个标志
     * @param flag 标志名称
     * @return 如果标志存在返回true，否则返回false
     */
    bool hasFlag(const std::string& flag) const {
        return flags.find(flag) != flags.end();
    }
    
    /**
     * @brief 添加位置参数
     * @param arg 参数值
     */
    void addArgument(const std::string& arg) {
        args.push_back(arg);
    }
    
    /**
     * @brief 获取指定位置的参数值
     * @param index 参数索引（从0开始）
     * @param defaultValue 默认值，当索引超出范围时返回
     * @return 参数值或默认值
     */
    std::string getArgument(size_t index, const std::string& defaultValue = "") const {
        if (index < args.size()) return args[index];
        return defaultValue;
    }
    
    /**
     * @brief 获取所有参数
     * @return 参数列表的常量引用
     */
    const std::vector<std::string>& getArguments() const {
        return args;
    }
    
    /**
     * @brief 获取参数数量
     * @return 参数个数
     */
    size_t argumentCount() const {
        return args.size();
    }
    
    /**
     * @brief 设置元数据
     * @param key 元数据键
     * @param value 元数据值
     */
    void setMetadata(const std::string& key, const std::string& value) {
        metadata[key] = value;
    }
    
    /**
     * @brief 获取元数据
     * @param key 元数据键
     * @return 元数据的可选类型，如果不存在返回std::nullopt
     */
    std::optional<std::string> getMetadata(const std::string& key) const {
        auto it = metadata.find(key);
        if (it != metadata.end()) return it->second;
        return std::nullopt;
    }
    
    /**
     * @brief 清空上下文内容
     */
    void clear() {
        commandName.clear();
        options.clear();
        flags.clear();
        args.clear();
        metadata.clear();
    }
    
private:
    // ========================================================================
    // 私有辅助方法
    // ========================================================================
    
    /**
     * @brief 解析命令行参数
     * @param argc 参数个数
     * @param argv 参数数组
     * @details 支持以下格式：
     *          - 命令：第一个非选项参数
     *          - 长选项：--option=value 或 --option value
     *          - 短选项：-o value 或 -f (标志)
     *          - 参数：非选项参数
     */
    void parseArgs(int argc, char* argv[]) {
        if (argc == 0) return;
        
        commandName = argv[0];
        
        for (int i = 1; i < argc; ++i) {
            std::string arg = argv[i];
            
            if (arg == "--") {
                // 分隔符，后面的都是参数
                for (int j = i + 1; j < argc; ++j) {
                    args.push_back(argv[j]);
                }
                break;
            } else if (arg.size() > 2 && arg.substr(0, 2) == "--") {
                // 长选项处理
                parseLongOption(arg.substr(2), i, argc, argv);
            } else if (arg.size() > 1 && arg[0] == '-') {
                // 短选项处理
                parseShortOption(arg.substr(1), i, argc, argv);
            } else {
                // 位置参数
                args.push_back(arg);
            }
        }
    }
    
    /**
     * @brief 解析字符串命令
     * @param input 命令行字符串
     * @details 将字符串分割为tokens，处理引号包围的参数
     */
    void parseString(const std::string& input) {
        std::vector<std::string> tokens;
        std::istringstream iss(input);
        std::string token;
        
        // 分割字符串，处理带引号的参数
        while (iss >> token) {
            if (!tokens.empty() && tokens.back().front() == '"' && tokens.back().back() != '"') {
                // 续接被空格分割的带引号字符串
                tokens.back() += " " + token;
            } else {
                tokens.push_back(token);
            }
        }
        
        // 清理引号
        for (auto& t : tokens) {
            if (t.front() == '"' && t.back() == '"') {
                t = t.substr(1, t.size() - 2);
            }
        }
        
        if (tokens.empty()) return;
        
        // 转换为argc/argv格式进行解析
        std::vector<char*> argv;
        std::vector<std::string> argvStorage;
        
        for (const auto& t : tokens) {
            argvStorage.push_back(t);
        }
        
        for (auto& s : argvStorage) {
            argv.push_back(&s[0]);
        }
        
        parseArgs(argv.size(), argv.data());
    }
    
    /**
     * @brief 解析长选项（--option格式）
     * @param opt 选项字符串（已去掉"--"前缀）
     * @param index 当前参数索引（引用，可能会被修改）
     * @param argc 参数总数
     * @param argv 参数数组
     */
    void parseLongOption(const std::string& opt, int& index, int argc, char* argv[]) {
        size_t eqPos = opt.find('=');
        
        if (eqPos != std::string::npos) {
            // 格式: --key=value
            std::string key = opt.substr(0, eqPos);
            std::string value = opt.substr(eqPos + 1);
            options[key] = value;
        } else {
            // 检查下一个参数是否为值
            if (index + 1 < argc && argv[index + 1][0] != '-') {
                options[opt] = argv[index + 1];
                ++index;  // 跳过值参数
            } else {
                // 布尔标志
                flags[opt] = "true";
            }
        }
    }
    
    /**
     * @brief 解析短选项（-o格式）
     * @param opt 选项字符串（已去掉"-"前缀）
     * @param index 当前参数索引（引用，可能会被修改）
     * @param argc 参数总数
     * @param argv 参数数组
     * @details 支持单字符选项（-o）和组合选项（-xyz）
     */
    void parseShortOption(const std::string& opt, int& index, int argc, char* argv[]) {
        if (opt.empty()) return;
        
        if (opt.size() == 1) {
            // 单字符选项
            char c = opt[0];
            if (index + 1 < argc && argv[index + 1][0] != '-') {
                options[std::string(1, c)] = argv[index + 1];
                ++index;  // 跳过值参数
            } else {
                flags[std::string(1, c)] = "true";
            }
        } else {
            // 组合短选项，如 -xyz
            for (char c : opt) {
                flags[std::string(1, c)] = "true";
            }
        }
    }
};

// ============================================================================
// 命令定义类
// ============================================================================

/**
 * @class CommandDefinition
 * @brief 命令定义类
 * 
 * 封装了命令的完整定义，包括名称、描述、参数、选项、执行器等。
 * 提供了流式接口（Fluent Interface）便于构建命令定义。
 */
class CommandDefinition {
private:
    // 命令基本信息
    std::string name;        ///< 命令名称（主名称）
    std::string description; ///< 命令描述，说明命令的作用
    std::string category;    ///< 命令分类，用于组织命令
    std::string usage;       ///< 使用说明，如果为空则自动生成
    std::vector<std::string> aliases;  ///< 命令别名列表
    std::vector<ParameterDefinition> parameters;  ///< 参数定义列表
    std::vector<OptionDefinition> options;        ///< 选项定义列表
    std::function<bool(const CommandContext&)> executor;  ///< 命令执行函数
    
    // 附加信息
    std::vector<std::string> examples; ///< 使用示例列表
    std::string version;               ///< 命令版本
    std::string author;                ///< 命令作者
    std::string helpText;              ///< 自定义帮助文本，如果为空则自动生成
    
public:
    /**
     * @brief 构造函数
     * @param n 命令名称
     * @param desc 命令描述
     */
    CommandDefinition(const std::string& n = "", const std::string& desc = "")
        : name(n), description(desc), category("General") {}
    
    // ========================================================================
    // 流式接口设置方法（返回*this以便链式调用）
    // ========================================================================
    
    /**
     * @brief 设置命令名称
     * @param n 命令名称
     * @return 当前对象的引用（支持链式调用）
     */
    CommandDefinition& setName(const std::string& n) { name = n; return *this; }
    
    /**
     * @brief 设置命令描述
     * @param desc 命令描述
     * @return 当前对象的引用
     */
    CommandDefinition& setDescription(const std::string& desc) { description = desc; return *this; }
    
    /**
     * @brief 设置命令分类
     * @param cat 分类名称
     * @return 当前对象的引用
     */
    CommandDefinition& setCategory(const std::string& cat) { category = cat; return *this; }
    
    /**
     * @brief 设置使用说明
     * @param use 使用说明字符串
     * @return 当前对象的引用
     */
    CommandDefinition& setUsage(const std::string& use) { usage = use; return *this; }
    
    /**
     * @brief 设置命令执行器
     * @param exec 执行函数，接受CommandContext并返回bool表示成功与否
     * @return 当前对象的引用
     */
    CommandDefinition& setExecutor(std::function<bool(const CommandContext&)> exec) { 
        executor = exec; 
        return *this; 
    }
    
    /**
     * @brief 设置自定义帮助文本
     * @param text 帮助文本
     * @return 当前对象的引用
     */
    CommandDefinition& setHelpText(const std::string& text) { helpText = text; return *this; }
    
    /**
     * @brief 设置命令版本
     * @param ver 版本字符串
     * @return 当前对象的引用
     */
    CommandDefinition& setVersion(const std::string& ver) { version = ver; return *this; }
    
    /**
     * @brief 设置命令作者
     * @param auth 作者信息
     * @return 当前对象的引用
     */
    CommandDefinition& setAuthor(const std::string& auth) { author = auth; return *this; }
    
    /**
     * @brief 添加命令别名
     * @param alias 别名
     * @return 当前对象的引用
     */
    CommandDefinition& addAlias(const std::string& alias) {
        aliases.push_back(alias);
        return *this;
    }
    
    /**
     * @brief 添加参数定义
     * @param param 参数定义对象
     * @return 当前对象的引用
     */
    CommandDefinition& addParameter(const ParameterDefinition& param) {
        parameters.push_back(param);
        return *this;
    }
    
    /**
     * @brief 添加参数（便捷方法）
     * @param name 参数名称
     * @param description 参数描述
     * @param required 是否必需，默认为false
     * @param defaultValue 默认值，默认为空
     * @param type 参数类型，默认为"string"
     * @return 当前对象的引用
     */
    CommandDefinition& addParameter(const std::string& name, 
                                   const std::string& description = "",
                                   bool required = false,
                                   const std::string& defaultValue = "",
                                   const std::string& type = TYPE_STRING) {
        parameters.emplace_back(name, description, required, defaultValue, type);
        return *this;
    }
    
    /**
     * @brief 添加选项定义
     * @param opt 选项定义对象
     * @return 当前对象的引用
     */
    CommandDefinition& addOption(const OptionDefinition& opt) {
        options.push_back(opt);
        return *this;
    }
    
    /**
     * @brief 添加选项（便捷方法）
     * @param name 选项名称（长选项）
     * @param shortName 短选项名称（单字符）
     * @param description 选项描述
     * @param requiresValue 是否需要值，默认为false
     * @param defaultValue 默认值，默认为空
     * @param valueType 值类型描述，默认为空
     * @return 当前对象的引用
     */
    CommandDefinition& addOption(const std::string& name,
                                const std::string& shortName = "",
                                const std::string& description = "",
                                bool requiresValue = false,
                                const std::string& defaultValue = "",
                                const std::string& valueType = "") {
        options.emplace_back(name, shortName, description, requiresValue, defaultValue, valueType);
        return *this;
    }
    
    /**
     * @brief 添加使用示例
     * @param example 示例字符串
     * @return 当前对象的引用
     */
    CommandDefinition& addExample(const std::string& example) {
        examples.push_back(example);
        return *this;
    }
    
    // ========================================================================
    // 获取方法
    // ========================================================================
    
    /**
     * @brief 获取命令名称
     * @return 命令名称
     */
    const std::string& getName() const { return name; }
    
    /**
     * @brief 获取命令描述
     * @return 命令描述
     */
    const std::string& getDescription() const { return description; }
    
    /**
     * @brief 获取命令分类
     * @return 命令分类
     */
    const std::string& getCategory() const { return category; }
    
    /**
     * @brief 获取使用说明
     * @return 使用说明
     */
    const std::string& getUsage() const { return usage; }
    
    /**
     * @brief 获取命令别名列表
     * @return 别名列表的常量引用
     */
    const std::vector<std::string>& getAliases() const { return aliases; }
    
    /**
     * @brief 获取参数定义列表
     * @return 参数定义列表的常量引用
     */
    const std::vector<ParameterDefinition>& getParameters() const { return parameters; }
    
    /**
     * @brief 获取选项定义列表
     * @return 选项定义列表的常量引用
     */
    const std::vector<OptionDefinition>& getOptions() const { return options; }
    
    /**
     * @brief 获取使用示例列表
     * @return 示例列表的常量引用
     */
    const std::vector<std::string>& getExamples() const { return examples; }
    
    /**
     * @brief 获取命令版本
     * @return 版本字符串
     */
    const std::string& getVersion() const { return version; }
    
    /**
     * @brief 获取命令作者
     * @return 作者信息
     */
    const std::string& getAuthor() const { return author; }
    
    /**
     * @brief 获取自定义帮助文本
     * @return 帮助文本
     */
    const std::string& getHelpText() const { return helpText; }
    
    // ========================================================================
    // 功能方法
    // ========================================================================
    
    /**
     * @brief 检查命令是否可执行
     * @return 如果设置了执行器返回true，否则返回false
     */
    bool isExecutable() const {
        return static_cast<bool>(executor);
    }
    
    /**
     * @brief 执行命令
     * @param context 命令执行上下文
     * @return 执行成功返回true，失败返回false
     * @throws 可能抛出执行器中的异常
     */
    bool execute(const CommandContext& context) const {
        if (executor) {
            return executor(context);
        }
        return false;
    }
    
    /**
     * @brief 验证参数是否符合定义
     * @param context 命令上下文，包含实际参数
     * @param errorMsg 输出参数，验证失败时存储错误信息
     * @return 验证通过返回true，否则返回false
     */
    bool validateArguments(const CommandContext& context, std::string& errorMsg) const {
        size_t argCount = context.argumentCount();
        
        // 检查必需参数
        for (size_t i = 0; i < parameters.size(); ++i) {
            if (parameters[i].required && i >= argCount) {
                errorMsg = "缺少必需参数: " + parameters[i].name;
                return false;
            }
        }
        
        // 检查参数数量是否过多（如果没有定义可变参数）
        if (!hasVariadicParameters() && argCount > parameters.size()) {
            errorMsg = "参数数量过多，最多允许 " + std::to_string(parameters.size()) + " 个参数";
            return false;
        }
        
        return true;
    }
    
    /**
     * @brief 生成命令使用说明
     * @return 使用说明字符串
     * @details 如果设置了自定义usage则使用之，否则根据参数定义自动生成
     */
    std::string generateUsage() const {
        if (!usage.empty()) {
            return usage;
        }
        
        std::stringstream ss;
        ss << name;
        
        // 添加参数
        for (const auto& param : parameters) {
            ss << " " << param.getUsage();
        }
        
        // 添加选项占位符
        if (!options.empty()) {
            ss << " [选项...]";
        }
        
        return ss.str();
    }
    
    /**
     * @brief 生成帮助文档
     * @param detailed 是否生成详细帮助（包括示例）
     * @return 帮助文档字符串
     * @details 如果设置了自定义helpText且不需要详细帮助，则使用自定义文本
     */
    std::string generateHelp(bool detailed = false) const {
        if (!helpText.empty() && !detailed) {
            return helpText;
        }
        
        std::stringstream ss;
        
        // 命令名称和描述
        ss << "命令: " << name;
        if (!aliases.empty()) {
            ss << " (别名: ";
            for (size_t i = 0; i < aliases.size(); ++i) {
                if (i > 0) ss << ", ";
                ss << aliases[i];
            }
            ss << ")";
        }
        ss << "\n";
        
        if (!description.empty()) {
            ss << "描述: " << description << "\n";
        }
        
        if (!category.empty() && category != "General") {
            ss << "分类: " << category << "\n";
        }
        
        if (!version.empty()) {
            ss << "版本: " << version << "\n";
        }
        
        if (!author.empty()) {
            ss << "作者: " << author << "\n";
        }
        
        ss << "\n用法: " << generateUsage() << "\n";
        
        // 参数说明
        if (!parameters.empty()) {
            ss << "\n参数:\n";
            for (const auto& param : parameters) {
                ss << "  " << std::left << std::setw(20) << param.getUsage();
                ss << " " << param.description;
                if (!param.defaultValue.empty()) {
                    ss << " [默认: " << param.defaultValue << "]";
                }
                if (!param.type.empty() && param.type != TYPE_STRING) {
                    ss << " (" << param.type << ")";
                }
                ss << "\n";
            }
        }
        
        // 选项说明
        if (!options.empty()) {
            ss << "\n选项:\n";
            for (const auto& opt : options) {
                ss << "  " << std::left << std::setw(40) << opt.getUsage();
                ss << " " << opt.description;
                if (!opt.defaultValue.empty()) {
                    ss << " [默认: " << opt.defaultValue << "]";
                }
                ss << "\n";
            }
        }
        
        // 使用示例
        if (!examples.empty() && detailed) {
            ss << "\n示例:\n";
            for (const auto& example : examples) {
                ss << "  " << example << "\n";
            }
        }
        
        return ss.str();
    }
    
private:
    /**
     * @brief 检查是否包含可变参数
     * @return 如果最后一个参数是"..."则返回true，表示支持可变参数
     */
    bool hasVariadicParameters() const {
        return !parameters.empty() && parameters.back().name == "...";
    }
};

// ============================================================================
// 命令管理器类（核心类）
// ============================================================================

/**
 * @class CommandManager
 * @brief 命令管理器类
 * 
 * 管理所有注册的命令，提供命令注册、查找、执行等功能。
 * 支持交互式命令行模式、参数解析、帮助系统等。
 */
class CommandManager {
private:
    // 命令存储结构
    std::map<std::string, CommandDefinition> commands;  ///< 命令名称到定义的映射
    std::map<std::string, std::string> aliasToCommand;  ///< 别名到命令名称的映射
    std::map<std::string, std::vector<std::string>> categoryToCommands;  ///< 分类到命令列表的映射
    
    // 全局选项定义
    std::vector<OptionDefinition> globalOptions;
    
    // 配置结构
    struct Config {
        std::string prompt = DEFAULT_PROMPT;  ///< 命令行提示符
        bool autoHelp = true;                 ///< 是否自动显示帮助
        bool verboseErrors = true;            ///< 是否详细显示错误
        bool colorOutput = true;              ///< 是否使用彩色输出
        int maxSuggestions = DEFAULT_MAX_SUGGESTIONS;  ///< 最大建议命令数
    } config;
    
public:
    /**
     * @brief 构造函数
     * @details 初始化全局选项和内置命令
     */
    CommandManager() {
        setupGlobalOptions();
        setupBuiltinCommands();
    }
    
    // ========================================================================
    // 配置方法
    // ========================================================================
    
    /**
     * @brief 设置命令行提示符
     * @param prompt 提示符字符串
     */
    void setPrompt(const std::string& prompt) { config.prompt = prompt; }
    
    /**
     * @brief 设置是否自动显示帮助
     * @param enable 启用或禁用自动帮助
     */
    void setAutoHelp(bool enable) { config.autoHelp = enable; }
    
    /**
     * @brief 设置是否详细显示错误
     * @param enable 启用或禁用详细错误
     */
    void setVerboseErrors(bool enable) { config.verboseErrors = enable; }
    
    /**
     * @brief 设置是否使用彩色输出
     * @param enable 启用或禁用彩色输出
     */
    void setColorOutput(bool enable) { config.colorOutput = enable; }
    
    /**
     * @brief 设置最大建议命令数
     * @param max 最大建议数
     */
    void setMaxSuggestions(int max) { config.maxSuggestions = max; }
    
    // ========================================================================
    // 命令注册方法
    // ========================================================================
    
    /**
     * @brief 注册完整命令定义
     * @param cmd 命令定义对象
     * @return 注册成功返回true，失败返回false
     * @note 如果命令已存在，会发出警告并覆盖
     */
    bool registerCommand(const CommandDefinition& cmd) {
        if (cmd.getName().empty()) {
            std::cerr << "错误: 命令名称不能为空" << std::endl;
            return false;
        }
        
        if (commands.find(cmd.getName()) != commands.end()) {
            std::cerr << "警告: 命令 '" << cmd.getName() << "' 已存在，将被覆盖" << std::endl;
        }
        
        // 注册主命令
        commands[cmd.getName()] = cmd;
        
        // 注册别名
        for (const auto& alias : cmd.getAliases()) {
            if (!alias.empty() && alias != cmd.getName()) {
                aliasToCommand[alias] = cmd.getName();
            }
        }
        
        // 按分类存储
        categoryToCommands[cmd.getCategory()].push_back(cmd.getName());
        
        return true;
    }
    
    /**
     * @brief 创建并注册命令（便捷方法，不带执行器）
     * @param name 命令名称
     * @param description 命令描述
     * @return 已注册命令定义的引用，可后续设置执行器
     * @warning 返回的引用在命令重新注册后可能失效
     */
    CommandDefinition& createCommand(const std::string& name, 
                                    const std::string& description) {
        CommandDefinition cmd(name, description);
        
        // 存储并返回引用
        commands[name] = cmd;
        categoryToCommands[cmd.getCategory()].push_back(name);
        
        return commands[name];
    }
    
    /**
     * @brief 创建并注册命令（便捷方法，带执行器）
     * @tparam Func 执行函数类型
     * @param name 命令名称
     * @param description 命令描述
     * @param executor 命令执行函数
     * @return 已注册命令定义的引用
     * @warning 返回的引用在命令重新注册后可能失效
     */
    template<typename Func>
    CommandDefinition& createCommand(const std::string& name, 
                                    const std::string& description,
                                    Func executor) {
        CommandDefinition& cmd = createCommand(name, description);
        cmd.setExecutor(executor);
        
        return cmd;
    }
    
    // ========================================================================
    // 命令处理核心方法
    // ========================================================================
    
    /**
     * @brief 处理单个命令（统一入口）
     * @param context 命令上下文
     * @return 执行成功返回true，失败返回false
     * 
     * 处理流程：
     * 1. 检查命令是否存在
     * 2. 处理帮助请求（-h或--help）
     * 3. 验证参数
     * 4. 执行命令
     * 5. 处理执行结果
     */
    bool processCommand(CommandContext& context) {
        std::string cmdName = context.getCommandName();
        
        // 空命令
        if (cmdName.empty()) {
            return true;
        }
        
        // 查找命令
        auto cmdDef = findCommand(cmdName);
        if (!cmdDef) {
            // 命令未找到，显示错误和帮助
            handleUnknownCommand(cmdName);
            return false;
        }
        
        // 检查帮助请求
        if (context.hasFlag("h") || context.hasFlag("help")) {
            std::cout << cmdDef->generateHelp(true) << std::endl;
            return true;
        }
        
        // 验证参数
        std::string validationError;
        if (!cmdDef->validateArguments(context, validationError)) {
            std::cerr << "错误: " << validationError << std::endl;
            if (config.autoHelp) {
                std::cout << "\n使用帮助:\n" << cmdDef->generateHelp() << std::endl;
            }
            return false;
        }
        
        // 执行命令
        try {
            bool success = cmdDef->execute(context);
            if (!success && config.autoHelp) {
                std::cout << "\n命令执行失败，请参考使用说明:\n" 
                         << cmdDef->generateHelp() << std::endl;
            }
            return success;
        } catch (const std::exception& e) {
            std::cerr << "命令执行错误: " << e.what() << std::endl;
            if (config.autoHelp) {
                std::cout << "\n请参考使用说明:\n" << cmdDef->generateHelp() << std::endl;
            }
            return false;
        }
    }
    
    /**
     * @brief 处理字符串命令
     * @param input 命令行字符串
     * @return 执行成功返回true，失败返回false
     */
    bool processString(const std::string& input) {
        CommandContext context(input);
        return processCommand(context);
    }
    
    /**
     * @brief 处理main函数参数
     * @param argc 参数个数
     * @param argv 参数数组
     * @return 执行成功返回true，失败返回false
     */
    bool processArgs(int argc, char* argv[]) {
        if (argc < 1) return true;
        
        CommandContext context(argc, argv);
        return processCommand(context);
    }
    
    /**
     * @brief 循环处理argv中的每个命令
     * @param argc 参数个数
     * @param argv 参数数组
     * @return 所有命令都执行成功返回true，否则返回false
     * 
     * 处理模式：
     * for(遍历argv) {
     *     if(当前参数不是选项) {
     *         创建命令上下文
     *         收集该命令的参数
     *         处理命令
     *     }
     * }
     */
    bool processArgLoop(int argc, char* argv[]) {
        bool allSuccess = true;
        
        for (int i = 1; i < argc; ++i) {
            std::string arg = argv[i];
            
            // 跳过全局选项
            if (arg[0] == '-') {
                continue;
            }
            
            // 创建命令上下文
            CommandContext context;
            context.setCommandName(arg);
            
            // 收集该命令的参数
            std::vector<std::string> cmdArgs;
            while (i + 1 < argc && argv[i + 1][0] != '-') {
                cmdArgs.push_back(argv[++i]);
            }
            
            // 添加参数到上下文
            for (const auto& arg : cmdArgs) {
                context.addArgument(arg);
            }
            
            // 处理命令
            if (!processCommand(context)) {
                allSuccess = false;
            }
        }
        
        return allSuccess;
    }
    
    // ========================================================================
    // 帮助系统方法
    // ========================================================================
    
    /**
     * @brief 显示所有命令的简要帮助
     * @param byCategory 是否按分类显示，默认true
     * 
     * 显示格式：
     * 可用命令:
     * ============================================================
     * 
     * 分类1:
     *   命令1     命令1的描述
     *   命令2     命令2的描述
     * 
     * 分类2:
     *   命令3     命令3的描述
     */
    void showAllCommands(bool byCategory = true) const {
        std::cout << "\n可用命令:\n";
        std::cout << std::string(60, '=') << "\n";
        
        if (byCategory) {
            // 按分类显示
            for (const auto& category : categoryToCommands) {
                std::cout << "\n" << category.first << ":\n";
                for (const auto& cmdName : category.second) {
                    auto it = commands.find(cmdName);
                    if (it != commands.end()) {
                        std::cout << "  " << std::left << std::setw(20) << cmdName
                                 << " " << it->second.getDescription() << "\n";
                    }
                }
            }
        } else {
            // 所有命令按字母排序
            std::vector<std::string> sortedNames;
            for (const auto& cmd : commands) {
                sortedNames.push_back(cmd.first);
            }
            std::sort(sortedNames.begin(), sortedNames.end());
            
            for (const auto& name : sortedNames) {
                auto it = commands.find(name);
                if (it != commands.end()) {
                    std::cout << "  " << std::left << std::setw(20) << name
                             << " " << it->second.getDescription() << "\n";
                }
            }
        }
        
        std::cout << "\n使用 'help <命令名>' 查看详细帮助\n";
        std::cout << std::endl;
    }
    
    /**
     * @brief 显示特定命令的详细帮助
     * @param commandName 命令名称
     * 
     * 显示格式：
     * 命令: 命令名 (别名: 别名1, 别名2)
     * 描述: 命令描述
     * 分类: 命令分类
     * 版本: 1.0.0
     * 作者: 作者名
     * 
     * 用法: 命令名 <参数1> [参数2] [选项...]
     * 
     * 参数:
     *   <参数1>      参数1的描述 [默认: 默认值] (类型)
     * 
     * 选项:
     *   -h, --help   显示帮助信息
     * 
     * 示例:
     *   示例1
     *   示例2
     */
    void showCommandHelp(const std::string& commandName) const {
        auto cmdDef = findCommand(commandName);
        if (cmdDef) {
            std::cout << cmdDef->generateHelp(true) << std::endl;
        } else {
            std::cout << "未找到命令: " << commandName << std::endl;
            showAllCommands();
        }
    }
    
    /**
     * @brief 显示全局帮助
     * 
     * 显示格式：
     * 命令行工具 - 全局帮助
     * ============================================================
     * 全局选项:
     *   -h, --help                显示帮助信息
     * 
     * 特殊命令:
     *   help [命令]               显示帮助信息
     *   list                      列出所有命令
     *   exit                      退出交互模式
     * 
     * 使用示例:
     *   1. 获取命令帮助: help <命令名>
     *   2. 使用命令: <命令名> [参数...] [选项...]
     *   3. 获取帮助: -h 或 --help
     */
    void showGlobalHelp() const {
        std::cout << "\n命令行工具 - 全局帮助\n";
        std::cout << std::string(60, '=') << "\n";
        
        std::cout << "全局选项:\n";
        for (const auto& opt : globalOptions) {
            std::cout << "  " << std::left << std::setw(40) << opt.getUsage()
                     << " " << opt.description << "\n";
        }
        
        std::cout << "\n特殊命令:\n";
        std::cout << "  help [命令]      显示帮助信息\n";
        std::cout << "  list             列出所有命令\n";
        std::cout << "  exit             退出交互模式\n";
        
        std::cout << "\n使用示例:\n";
        std::cout << "  1. 获取命令帮助: help <命令名>\n";
        std::cout << "  2. 使用命令: <命令名> [参数...] [选项...]\n";
        std::cout << "  3. 获取帮助: -h 或 --help\n";
        std::cout << std::endl;
    }
    
    // ========================================================================
    // 交互模式方法
    // ========================================================================
    
    /**
     * @brief 运行交互式命令行模式
     * 
     * 启动一个交互式命令行界面，用户可以输入命令并查看结果。
     * 支持特殊命令：
     *   help - 显示帮助
     *   list - 列出所有命令
     *   exit/quit - 退出
     */
    void runInteractive() {
        std::string input;
        
        std::cout << "ConsoleCommandManager 交互模式\n";
        std::cout << "输入 'help' 查看帮助，'list' 列出命令，'exit' 退出\n\n";
        
        while (true) {
            std::cout << config.prompt;
            
            if (!std::getline(std::cin, input)) {
                break; // EOF
            }
            
            // 跳过空白输入
            if (input.empty()) continue;
            
            // 检查特殊命令
            if (input == "exit" || input == "quit") {
                std::cout << "再见！" << std::endl;
                break;
            } else if (input == "help") {
                showGlobalHelp();
                continue;
            } else if (input == "list") {
                showAllCommands();
                continue;
            }
            
            // 处理命令
            if (!processString(input)) {
                if (config.verboseErrors) {
                    std::cout << "命令执行失败，输入 'help' 查看帮助" << std::endl;
                }
            }
        }
    }
    
    // ========================================================================
    // 查询方法
    // ========================================================================
    
    /**
     * @brief 检查命令是否存在
     * @param name 命令名称或别名
     * @return 命令存在返回true，否则返回false
     */
    bool commandExists(const std::string& name) const {
        return findCommand(name) != nullptr;
    }
    
    /**
     * @brief 获取所有命令名称列表
     * @return 命令名称列表
     */
    std::vector<std::string> getCommandList() const {
        std::vector<std::string> list;
        for (const auto& cmd : commands) {
            list.push_back(cmd.first);
        }
        return list;
    }
    
    /**
     * @brief 按分类获取命令
     * @return 分类到命令列表的映射
     */
    std::map<std::string, std::vector<std::string>> getCommandsByCategory() const {
        return categoryToCommands;
    }
    
private:
    // ========================================================================
    // 私有辅助方法
    // ========================================================================
    
    /**
     * @brief 设置全局选项
     */
    void setupGlobalOptions() {
        globalOptions = {
            OptionDefinition("help", "h", "显示帮助信息", false),
            OptionDefinition("verbose", "v", "详细输出模式", false),
            OptionDefinition("quiet", "q", "安静模式，减少输出", false),
            OptionDefinition("version", "V", "显示版本信息", false),
            OptionDefinition("config", "c", "指定配置文件", true, "", "文件路径")
        };
    }
    
    /**
     * @brief 设置内置命令
     */
    void setupBuiltinCommands() {
        // 内置帮助命令
        CommandDefinition helpCmd("help", "显示帮助信息");
        helpCmd.addParameter(
            ParameterDefinition("command", "命令名称", false, "", TYPE_COMMAND)
        );
        helpCmd.addAlias("?");
        helpCmd.setExecutor([this](const CommandContext& ctx) {
            if (ctx.argumentCount() > 0) {
                // 显示特定命令的帮助
                std::string cmdName = ctx.getArgument(0);
                showCommandHelp(cmdName);
            } else {
                // 显示全局帮助
                showGlobalHelp();
            }
            return true;
        });
        
        helpCmd.addExample("help              # 显示全局帮助");
        helpCmd.addExample("help <命令名>     # 显示特定命令的帮助");
        
        registerCommand(helpCmd);
        
        // 内置列表命令
        CommandDefinition listCmd("list", "列出所有可用命令");
        listCmd.addOption(
            OptionDefinition("category", "c", "按分类显示", false)
        );
        listCmd.setExecutor([this](const CommandContext& ctx) {
            bool byCategory = ctx.hasFlag("c") || ctx.hasFlag("category");
            showAllCommands(byCategory);
            return true;
        });
        
        listCmd.addExample("list              # 列出所有命令");
        listCmd.addExample("list -c           # 按分类列出命令");
        
        registerCommand(listCmd);
    }
    
    /**
     * @brief 查找命令（处理别名）
     * @param name 命令名称或别名
     * @return 命令定义的指针，如果找不到返回nullptr
     */
    const CommandDefinition* findCommand(const std::string& name) const {
        // 直接查找
        auto it = commands.find(name);
        if (it != commands.end()) {
            return &it->second;
        }
        
        // 通过别名查找
        auto aliasIt = aliasToCommand.find(name);
        if (aliasIt != aliasToCommand.end()) {
            auto cmdIt = commands.find(aliasIt->second);
            if (cmdIt != commands.end()) {
                return &cmdIt->second;
            }
        }
        
        return nullptr;
    }
    
    /**
     * @brief 处理未知命令
     * @param cmdName 用户输入的命令名称
     * 
     * 处理流程：
     * 1. 显示错误信息
     * 2. 查找相似命令并提供建议
     * 3. 显示可用命令提示
     */
    void handleUnknownCommand(const std::string& cmdName) const {
        std::cerr << "错误: 未知命令 '" << cmdName << "'" << std::endl;
        
        // 查找相似命令
        std::vector<std::string> suggestions;
        for (const auto& cmd : commands) {
            if (isSimilar(cmdName, cmd.first)) {
                suggestions.push_back(cmd.first);
                if (suggestions.size() >= config.maxSuggestions) {
                    break;
                }
            }
        }
        
        if (!suggestions.empty()) {
            std::cout << "\n您是否想输入以下命令？\n";
            for (const auto& suggestion : suggestions) {
                auto it = commands.find(suggestion);
                if (it != commands.end()) {
                    std::cout << "  " << suggestion << " - " << it->second.getDescription() << "\n";
                }
            }
        } else {
            std::cout << "\n使用 'list' 查看所有可用命令\n";
        }
        
        std::cout << std::endl;
    }
    
    /**
     * @brief 检查字符串相似度（简单实现）
     * @param a 字符串a
     * @param b 字符串b
     * @return 如果相似返回true，否则返回false
     * 
     * 判断标准：
     * 1. 前缀匹配
     * 2. 长度相差不大
     * 3. 字符匹配度超过60%
     */
    bool isSimilar(const std::string& a, const std::string& b) const {
        if (a.empty() || b.empty()) return false;
        
        // 前缀匹配
        if (b.find(a) == 0) return true;
        
        // 编辑距离简单判断
        if (std::abs(static_cast<int>(a.size()) - static_cast<int>(b.size())) > 2) {
            return false;
        }
        
        // 字符相似度
        int matches = 0;
        for (size_t i = 0; i < std::min(a.size(), b.size()); ++i) {
            if (a[i] == b[i]) ++matches;
        }
        
        double similarity = static_cast<double>(matches) / std::max(a.size(), b.size());
        return similarity > 0.6;
    }
};

// ============================================================================
// 便捷工具函数和宏
// ============================================================================

/**
 * @brief 创建命令管理器实例
 * @return 新的CommandManager对象
 */
inline CommandManager createManager() {
    return CommandManager();
}

/**
 * @def DEFINE_COMMAND(name, desc)
 * @brief 定义命令的便捷宏
 * @param name 命令名称
 * @param desc 命令描述
 * @return CommandDefinition对象
 */
#define DEFINE_COMMAND(name, desc) \
    ConsoleCommand::CommandDefinition(name, desc)

/**
 * @def REGISTER_COMMAND(manager, cmdDef)
 * @brief 注册命令的便捷宏
 * @param manager 命令管理器
 * @param cmdDef 命令定义对象
 */
#define REGISTER_COMMAND(manager, cmdDef) \
    manager.registerCommand(cmdDef)

/**
 * @brief 批量注册命令
 * @tparam Commands 命令定义类型（可变参数）
 * @param manager 命令管理器
 * @param commands 要注册的命令定义列表
 */
template<typename... Commands>
void registerCommands(CommandManager& manager, Commands&&... commands) {
    (manager.registerCommand(std::forward<Commands>(commands)), ...);
}

} // namespace ConsoleCommand

#endif // CONSOLE_COMMAND_MANAGER_H
