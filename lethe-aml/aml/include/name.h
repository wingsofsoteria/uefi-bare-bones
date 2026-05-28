#pragma once
#include "types.h"
aml_name_t  resolve_name(aml_namespace_t* ns, aml_name_t key);
aml_name_t* parse_namestring(aml_namespace_t* ns);
aml_name_t* trim_name(aml_name_t* name);
