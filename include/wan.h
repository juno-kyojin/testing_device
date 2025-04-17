#ifndef WAN_H
#define WAN_H

#include "types.h"

void wan_view_config(Instruction *instr);
void wan_view_status(Instruction *instr);
void wan_pppoe_create(Instruction *instr);
void wan_ipoe_create(Instruction *instr);
void wan_bridge_create(Instruction *instr);
void wan_pppoe_edit(Instruction *instr);
void wan_ipoe_edit(Instruction *instr);
void wan_bridge_edit(Instruction *instr);
void wan_remove(Instruction *instr);

#endif