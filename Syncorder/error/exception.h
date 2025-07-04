/**
 * @macro
 */
#define EXCEPTION(...)                      \
    try {                                   \
        __VA_ARGS__                         \
    } catch (const std::exception& e) {     \
        std::cout << "Error: " << e.what();    \
        return false;                       \
    }