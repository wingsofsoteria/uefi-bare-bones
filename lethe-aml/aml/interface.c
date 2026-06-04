#include "aml.h"
#include "hashmap.h"
#include "helpers.h"
#include "host.h"
#include "namespace.h"
#include "opcodes.h"
#include "tables.h"
#include "types.h"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

aml_ptr_t* get_object(char* static_key)
{
  unimplemented(static_key[0] != '\\');
  size_t len = strlen(static_key) - 1;
  char*  key = malloc(len);
  memcpy(key, static_key + 1, len);
  aml_ptr_t* ptr;
  if (len == 4) { ptr = hash_map_get(root()->children, key, NULL); }
  else
    {
      char* scope_name = malloc(len - 4);
      memcpy(scope_name, key, len - 4);
      aml_name_t       scope_key = { len, scope_name };
      aml_namespace_t* scope     = get_scope(root(), scope_key);
      free(scope_name);
      scope_key = (aml_name_t){ 0 };
      assert(scope);
      memmove(key, key + len - 4, 4);
      ptr = hash_map_get(scope->children, key, NULL);
    }
  free(key);
  return ptr;
}

static uint64_t variable_to_int(aml_variable_t* variable)
{
  switch (variable->data_type)
    {
      case DATA_BYTE:
        {
          return variable->byte;
        }
      case DATA_INT:
        {
          return variable->integer;
        }
      case DATA_SHORT:
        {
          return variable->short_int;
        }
      case DATA_LONG:
        {
          return variable->long_int;
        }
    }
  return UINT64_MAX;
}

// TODO other sleep types
void execute_sleep(int state)
{
  char* name;
  switch (state)
    {
      case 5:
        {
          name = "_S5_";
          break;
        }
      default:
        {
          printf("sleep state %d unimplemented\n", state);
          AML_EXIT();
        }
    }

  aml_ptr_t* obj = hash_map_get(root()->children, name, NULL);
  if (!obj)
    {
      printf("Could not locate sleep object %s\n", name);
      AML_EXIT();
    }
  printf("%d\n", obj->type);
  uint64_t slp_typa = 0;
  uint64_t slp_typb = 0;
  switch (obj->type)
    {
      case TYPE_NAME:
        {
          aml_variable_t* variable = obj->data;
          switch (variable->data_type)
            {
              case DATA_PKG:
                {
                  aml_package_t*  pkg          = variable->package;
                  aml_variable_t* var_slp_typa = pkg->elements[0];
                  aml_variable_t* var_slp_typb = pkg->elements[1];
                  slp_typa                     = variable_to_int(var_slp_typa);
                  slp_typb                     = variable_to_int(var_slp_typb);
                  if (slp_typa == UINT64_MAX || slp_typb == UINT64_MAX)
                    {
                      printf(
                        "Failed to cast aml_variable_t to type uint64_t\n"
                      );
                      AML_EXIT();
                    }
                  break;
                }
              default:
                {
                  printf("unknown data_type %d\n", variable->data_type);
                  AML_EXIT();
                }
            }
          break;
        }
      default:
        {
          printf("unknown type %d\n", obj->type);
          AML_EXIT();
        }
    }
  aml_ptr_t* _pts = hash_map_get(root()->children, "_PTS", NULL);
  if (_pts)
    { // execute _PTS
    }

  // TODO proper checks for x_pm1x_cnt_blk
  acpi_fadt_t* fadt = scan_tables("FACP", 0);
  unimplemented(fadt->flags & 1 << 20);
  outw(fadt->pm1a_control_block, slp_typa << 10 | ACPI_SLEEP);
  outw(fadt->pm1b_control_block, slp_typb << 10 | ACPI_SLEEP);
}

void execute_method(aml_method_t* method, ...)
{ parse_termlist(method->scope, method->code, method->code + method->len); }
