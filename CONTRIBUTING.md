# Contributing to Pill Dispenser V3

## Development Environment Setup
1. Clone repository
2. Install Arduino IDE with ESP32 support
3. Install required libraries
4. Configure Firebase credentials
5. Test with development mode

## Code Standards
- **Classes**: PascalCase (`ServoDriver`, `FirebaseManager`)
- **Methods**: camelCase (`dispensePill`, `isConnected`)
- **Constants**: UPPER_CASE (`MAX_SCHEDULES`, `DEFAULT_TIMEOUT`)
- **Documentation**: Doxygen-style comments for all public methods

## Component Development Guidelines
1. **Header File**: Define class interface
2. **Implementation File**: Implement functionality
3. **Integration**: Add to main application
4. **Testing**: Implement comprehensive test methods
5. **Documentation**: Update API reference

## Pull Request Process
1. Fork repository
2. Create feature branch
3. Make changes following standards
4. Add tests for new functionality
5. Update documentation
6. Submit pull request