#ifndef _CONFIG_H_
#define _CONFIG_H_

enum {
    ConfigType_None,
    ConfigType_Bool,
    ConfigType_Int,
    ConfigType_String,
    ConfigType_NumTypes
};

struct ConfigOption {
    const char* OptionName;
    int ConfigType;
    void* VariablePtr;
};

#endif
