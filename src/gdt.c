#include "header/cpu/gdt.h"

/**
 * global_descriptor_table, predefined GDT.
 * Initial SegmentDescriptor already set properly according to Intel Manual & OSDev.
 * Table entry : [{Null Descriptor}, {Kernel Code}, {Kernel Data (variable, etc)}, ...].
 */
struct GlobalDescriptorTable global_descriptor_table = {
    .table = {
        {
            0, // segment_low
            0, // base_low
            0, // base_mid
            0, // type_bit
            0, // non_system
            0, // desc_priv_lvl
            0, // segment_present
            0, // segment_mid
            0, // available
            0, // L
            0, // default_op_size
            0, // granularity
            0  //base high
        },
        {
            0xFFFF, // segment_low
            0,      // base_low
            0,      // base_mid
            0xA,    // type_bit
            1,      // non_system
            0,      // desc_priv_lvl
            1,      // segment_present
            0xF,    // segment_mid
            0,      // available
            0,      // L
            1,      // default_op_size
            1,      // granularity
            0       //base high
        },
        {
            0xFFFF, // segment_low
            0,      // base_low
            0,      // base_mid
            0x2,    // type_bit
            1,      // non_system
            0,      // desc_priv_lvl
            1,      // segment_present
            0xF,    // segment_mid
            0,      // available
            0,      // L
            1,      // default_op_size
            1,      // granularity
            0       //base high
        }
    }
};

/**
 * _gdt_gdtr, predefined system GDTR. 
 * GDT pointed by this variable is already set to point global_descriptor_table above.
 * From: https://wiki.osdev.org/Global_Descriptor_Table, GDTR.size is GDT size minus 1.
 */
struct GDTR _gdt_gdtr = {
    sizeof(global_descriptor_table) - 1, &global_descriptor_table
};
