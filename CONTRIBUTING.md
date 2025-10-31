# Contributing to Pill Dispenser V3

We welcome contributions to the Pill Dispenser V3 project. This document provides guidelines for contributing to the project.

## Code of Conduct

By participating in this project, you agree to abide by our code of conduct:

- Use welcoming and inclusive language
- Be respectful of differing viewpoints and experiences
- Gracefully accept constructive criticism
- Focus on what is best for the community
- Show empathy towards other community members

## Getting Started

### Prerequisites

- Arduino IDE 2.0 or later
- ESP32 board support package
- Git for version control
- Basic knowledge of C++ and Arduino framework

### Development Environment Setup

1. Clone the repository:
   ```bash
   git clone https://github.com/qppd/Pill-Dispenser-V3.git
   cd Pill-Dispenser-V3
   ```

2. Install required Arduino libraries:
   - Adafruit PWM Servo Driver Library
   - LiquidCrystal I2C
   - Firebase ESP32 Client
   - RTC by Makuna

3. Configure your development board and upload the firmware

## How to Contribute

### Reporting Bugs

Before creating bug reports, please check the existing issues to avoid duplicates. When creating a bug report, include:

- **Clear title and description**
- **Steps to reproduce** the issue
- **Expected behavior** vs actual behavior
- **Hardware configuration** (ESP32 board, components)
- **Software versions** (Arduino IDE, library versions)
- **Serial output** or error messages
- **Additional context** like environment conditions

### Suggesting Enhancements

Enhancement suggestions are welcome. Please provide:

- **Clear title and description** of the enhancement
- **Use case** explaining why this enhancement would be useful
- **Detailed explanation** of how it should work
- **Mockups or examples** if applicable

### Pull Requests

1. **Fork the repository** and create your branch from `main`
2. **Make your changes** following our coding standards
3. **Add tests** for new functionality
4. **Update documentation** as needed
5. **Ensure tests pass** and code compiles
6. **Submit a pull request** with a clear description

## Coding Standards

### C++ Style Guide

#### Naming Conventions

- **Classes**: PascalCase (`ServoDriver`, `FirebaseManager`)
- **Methods**: camelCase (`dispensePill`, `isConnected`)
- **Variables**: camelCase (`sensorValue`, `currentTime`)
- **Constants**: UPPER_CASE (`MAX_SERVOS`, `DEFAULT_TIMEOUT`)
- **Private members**: camelCase with underscore prefix (`_isInitialized`)

#### Code Structure

```cpp
// Header file structure
#ifndef CLASS_NAME_H
#define CLASS_NAME_H

#include <Arduino.h>
// Other includes...

class ClassName {
private:
    // Private members
    
public:
    // Public interface
    ClassName();                    // Constructor
    bool begin();                   // Initialization
    void methodName();              // Public methods
    bool isConnected();             // Status methods
    void testClassName();           // Test methods
};

#endif
```

#### Documentation

All public methods must include documentation:

```cpp
/**
 * @brief Dispense a pill of specified size
 * @param channel Servo channel (0-15)
 * @param size Pill size ("small", "medium", "large")
 * @return true if successful, false otherwise
 */
bool dispensePill(uint8_t channel, String size);
```

### Component Development Guidelines

#### Creating New Components

1. **Header File** (`ComponentName.h`):
   ```cpp
   #ifndef COMPONENT_NAME_H
   #define COMPONENT_NAME_H
   
   class ComponentName {
   private:
       // Private implementation
   public:
       ComponentName();
       bool begin();
       void testComponent();
   };
   
   #endif
   ```

2. **Implementation File** (`ComponentName.cpp`):
   ```cpp
   #include "ComponentName.h"
   
   ComponentName::ComponentName() {
       // Constructor implementation
   }
   
   bool ComponentName::begin() {
       // Initialization code
       return true;
   }
   
   void ComponentName::testComponent() {
       // Test implementation
   }
   ```

3. **Integration**:
   - Add include to main file
   - Create instance in component section
   - Add initialization in `setup()`
   - Add serial commands in command parser

#### Required Methods

All component classes must implement:

- **Constructor**: Initialize member variables
- **begin()**: Hardware initialization and setup
- **isConnected()** or **isReady()**: Status checking
- **test[Component]()**: Comprehensive testing method

#### Error Handling

Use consistent error handling patterns:

```cpp
bool ComponentName::initialize() {
    if (!checkPreconditions()) {
        Serial.println("ComponentName: Preconditions failed");
        return false;
    }
    
    if (!performInitialization()) {
        Serial.println("ComponentName: Initialization failed");
        return false;
    }
    
    Serial.println("ComponentName: Initialized successfully");
    return true;
}
```

### Testing Requirements

#### Unit Testing

Each component must include comprehensive test methods:

```cpp
void ComponentName::testComponent() {
    Serial.println("ComponentName: Starting test sequence");
    
    // Test 1: Basic functionality
    if (testBasicFunction()) {
        Serial.println("ComponentName: Basic function test PASSED");
    } else {
        Serial.println("ComponentName: Basic function test FAILED");
    }
    
    // Test 2: Edge cases
    if (testEdgeCases()) {
        Serial.println("ComponentName: Edge case test PASSED");
    } else {
        Serial.println("ComponentName: Edge case test FAILED");
    }
    
    Serial.println("ComponentName: Test sequence complete");
}
```

#### Integration Testing

Test component interactions:

```cpp
void testComponentIntegration() {
    // Test component A + B interaction
    componentA.performAction();
    delay(100);
    
    if (componentB.verifyResult()) {
        Serial.println("Integration test PASSED");
    } else {
        Serial.println("Integration test FAILED");
    }
}
```

### Documentation Requirements

#### Code Documentation

- **Header comments** for all files
- **Method documentation** for public interfaces
- **Inline comments** for complex logic
- **Example usage** in component documentation

#### README Updates

When adding new features:

1. Update feature list
2. Add API reference entries
3. Include configuration instructions
4. Add troubleshooting section
5. Update command reference

## Development Workflow

### Branch Management

- **main**: Stable release branch
- **develop**: Development integration branch
- **feature/feature-name**: New feature development
- **bugfix/issue-number**: Bug fixes
- **hotfix/critical-fix**: Critical production fixes

### Commit Messages

Use clear, descriptive commit messages:

```
feat: add new servo speed control functionality
fix: resolve I2C communication timeout issue
docs: update API reference for FirebaseManager
test: add comprehensive sensor testing suite
refactor: improve error handling in ServoDriver
```

### Pull Request Process

1. **Create Feature Branch**:
   ```bash
   git checkout -b feature/new-component
   ```

2. **Make Changes** following coding standards

3. **Test Thoroughly**:
   - Compile without warnings
   - Test on actual hardware
   - Verify existing functionality

4. **Update Documentation**:
   - Update README if needed
   - Add inline documentation
   - Update API reference

5. **Submit Pull Request**:
   - Clear title and description
   - Reference related issues
   - Include testing evidence

### Code Review Checklist

Reviewers will check for:

- [ ] Code follows style guidelines
- [ ] All tests pass
- [ ] Documentation is updated
- [ ] No breaking changes to existing API
- [ ] Memory usage is optimized
- [ ] Error handling is comprehensive
- [ ] Serial output is appropriate
- [ ] Hardware compatibility maintained

## Hardware Contributions

### PCB Design

If contributing PCB designs:

- Use KiCad for schematic and layout
- Include bill of materials (BOM)
- Provide fabrication files (Gerbers)
- Document assembly instructions

### 3D Models

For mechanical components:

- Use standard CAD formats (STEP, STL)
- Include source files when possible
- Provide assembly instructions
- Consider printability for 3D printing

## Communication

### Channels

- **GitHub Issues**: Bug reports and feature requests
- **GitHub Discussions**: General questions and ideas
- **Pull Requests**: Code contributions and reviews

### Response Times

We aim to respond to:
- **Critical bugs**: Within 24 hours
- **General issues**: Within 1 week
- **Feature requests**: Within 2 weeks
- **Pull requests**: Within 1 week

## Recognition

Contributors will be recognized in:
- README acknowledgments
- Release notes
- GitHub contributor graphs
- Project documentation

## Questions?

If you have questions about contributing:

1. Check existing issues and documentation
2. Create a GitHub Discussion for general questions
3. Open an issue for specific problems
4. Contact maintainers for urgent matters

Thank you for contributing to Pill Dispenser V3!