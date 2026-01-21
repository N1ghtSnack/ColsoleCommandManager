# ConsoleCommandManager

A header-only C++ library providing a complete, flexible, and user-friendly command-line interface management system.

## Features

- **Complete Command Metadata Support**: Each command, parameter, and option has detailed description information
- **Intelligent Help System**: Automatically generates help documentation for commands
- **Multiple Invocation Modes**: Supports interactive CLI, direct parameter passing, batch processing, and more
- **Type Safety**: Parameter validation and type checking
- **Extensibility**: Easy to add new commands and features
- **Error Handling**: Comprehensive error handling with user-friendly error messages

## Quick Start

```cpp
#include "ConsoleCommandManager.h"

int main(int argc, char* argv[]) {
    auto manager = ConsoleCommand::createManager();
    
    manager.createCommand("hello", "Say hello",
        [](const ConsoleCommand::CommandContext& ctx) {
            std::cout << "Hello, " << ctx.getArgument(0, "World") << "!" << std::endl;
            return true;
        })
        .addParameter("name", "Your name", false, "World");
    
    if (argc > 1) {
        return manager.processArgs(argc, argv) ? 0 : 1;
    } else {
        manager.runInteractive();
        return 0;
    }
}
```

## Files

- **ConsoleCommandManager.h**: Complete header-only library (1600+ lines)
- **example.cpp**: SimpleFileManager demonstration with 7 file operations
- **CMakeLists.txt**: Build configuration for C++17

## Architecture

The library is organized in 5 layers:
1. **Infrastructure Layer**: CommandContext (parameter/option parsing)
2. **Parsing Layer**: Parameter and option definitions
3. **Execution Layer**: CommandDefinition (command metadata and execution)
4. **Management Layer**: CommandManager (command registration and lookup)
5. **Application Layer**: User application code

## License

Licensed under MIT License - see LICENSE file for details
