#include "aml.h"
#include "hashmap.h"
#include "helpers.h"
#include "host.h"
#include "namespace.h"
#include "opcodes.h"
#include "tables.h"
#include "types.h"

#include <stdint.h>
#include <stdio.h>

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
      case DATA_UNINIT:
        {
          printf("%s\n", variable->string);
          break;
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
          aml_log("sleep state %d unimplemented\n", state);
          AML_EXIT();
        }
    }

  aml_ptr_t* obj = hm_get(root()->children, name);
  if (!obj)
    {
      aml_log("Could not locate sleep object %s\n", name);
      AML_EXIT();
    }
  aml_log("%d\n", obj->type);
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
                  aml_package_t* pkg = variable->package;
                  // todo write proper type checks for these since it WILL cause
                  // issues later
                  aml_variable_t* var_slp_typa = pkg->elements[0]->data;
                  aml_variable_t* var_slp_typb = pkg->elements[1]->data;
                  aml_log(
                    "TYPA %d TYPB %d",
                    var_slp_typa->data_type,
                    var_slp_typb->data_type
                  );
                  slp_typa = variable_to_int(var_slp_typa);
                  slp_typb = variable_to_int(var_slp_typb);
                  if (slp_typa == UINT64_MAX || slp_typb == UINT64_MAX)
                    {
                      aml_log(
                        "Failed to cast aml_variable_t to type uint64_t\n"
                      );
                      AML_EXIT();
                    }
                  break;
                }
              default:
                {
                  aml_log("unknown data_type %d\n", variable->data_type);
                  AML_EXIT();
                }
            }
          break;
        }
      default:
        {
          aml_log("unknown type %d\n", obj->type);
          AML_EXIT();
        }
    }
  aml_ptr_t* _pts = hm_get(root()->children, "_PTS");
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
