## General ##

TODO

## Style Guide ## 

This application is built on GNU C++20 and follows typical C++ conventions. 

Classes/Structs:

PascalCase: 

Enums: 
The enum name should be in PascalCase. The enumerators should be in CAPITALIZED_SNAKE_CASE. 
Enum classes are the recommended implementation for public facing enums. Regular enums for internal implementation are fine. 
For regular enums prefix with an identifier such as "PAGE_GAME", for enum classes the type means we dont need this. If an enum is case to a different variable type, use a comment to mention it.  

Variables: 
Variables should be camelCase. Do not use hungarian notation. Avoid single letters except for loop logic. Small abreviations are fine for commonly used variables (dt for deltaTime). However, abreviations should generally be not used. Acronyms are fine. 
For booleans, use a binary conditional prefix (is, contains, etc). 

Methods: 
Methods should use camelCase. Methods should use /***/ comments with @param & @return (if applicable) on all public facing methods. 
Methods should contain "Async" as a suffix if operation uses multithreading. 
Private methods can use regular // comments 

Namespaces: 
WIP
should be a single word and lowercase? 

Logging:
Always use a new line for logging commands, ex/

GG_LOG_INFO(
    LOG_SCENE,
    "Test log at '%s',
    logPath.c_str()
);

Always use '' where strings will be, and [] for values. Give content to types (ex/ ms for milliseconds, MB for megabytes, etc)

Current Issues

This application is filled with inconsistent programing styles.