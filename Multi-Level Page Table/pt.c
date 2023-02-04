#include "os.h"

#define NUMBER_OF_LEVELS 5
#define NUMBER_OF_ROWS 512


void page_table_update(uint64_t pt, uint64_t vpn, uint64_t ppn);
uint64_t page_table_query(uint64_t pt, uint64_t vpn);
uint64_t get_PMLi_bits(uint64_t vpn, int level);





void page_table_update(uint64_t pt, uint64_t vpn, uint64_t ppn) {
    /* << 12 is because we receive a 64-bit address to represent 52 bits of a ppn (pt)*/
    uint64_t *node = (uint64_t *) phys_to_virt(pt << 12);
    uint64_t pte;
    int i;

    for (i = 0; i < NUMBER_OF_LEVELS; i++) {
        pte = get_PMLi_bits(vpn, i) & 0x1ff; /*getting the 9 bits needed for current trie node, 0x1ff = 111111111*/

        /*create virtual memory mapping*/
        if (ppn != NO_MAPPING) {
            /*this is the last node, node[pte] = ppn*/
            if (i == NUMBER_OF_LEVELS - 1) {
                node[pte] = (ppn << 12) + 1; /*+1 is for the valid bit*/
                return;
            } else {
                /*need to create a new frame */
                if (node[pte] == NO_MAPPING || node[pte] == 0) {
                    uint64_t new_frame = alloc_page_frame();
                    node[pte] = (new_frame << 12) + 1; /*+1 is for the valid bit*/
                }
            }
        } else {/*destroy virtual memory mapping*/
            /*no need to destroy - it does not exist */
            if (node[pte] == NO_MAPPING || node[pte] == 0)
                return;

            /*destroy here is making it invalid*/
            if (i == NUMBER_OF_LEVELS - 1) {
                node[pte] = ((node[pte] >> 1) << 1);/*node[pte] valid bit = 0*/
                return;
            }
        }
        node = phys_to_virt(node[pte] & 0xFFFFFFFFFFFFF000); /*continue pagewalk : 0xFFFFFFFFFFFFF000 - 64 bits : upper 52 equal 1, lower 12 equal 0*/
    }
}


uint64_t page_table_query(uint64_t pt, uint64_t vpn){

    uint64_t *node = (uint64_t *) phys_to_virt(pt << 12);
    uint64_t pte;
    int i;
    for (i = 0; i < NUMBER_OF_LEVELS; i++) {
        pte = get_PMLi_bits(vpn, i) & 0x1ff; /*getting the 9 bits needed for current trie node, 0x1ff = 111111111*/

        if (node[pte] == 0 || node[pte] == NO_MAPPING){/*a pte is not mapped while doing pagewalk*/
            return NO_MAPPING;
        }
        if (i == NUMBER_OF_LEVELS - 1){ /*last level of trie nodes*/
            if ((node[pte] & 1) == 0) /*not valid!*/
                return NO_MAPPING;
            return (node[pte] >> 12);/*the ppn that vpn is mapped to*/
        }
        /*continuing pagewalk to the next table*/
        node = phys_to_virt(node[pte] & 0xFFFFFFFFFFFFF000);/*0xFFFFFFFFFFFFF000 - 64 bits : upper 52 equal 1, lower 12 equal 0*/
    }
    return NO_MAPPING;
}

uint64_t get_PMLi_bits(uint64_t vpn, int level) {
    return (vpn >> ((NUMBER_OF_LEVELS-1-level) * 9));
}
