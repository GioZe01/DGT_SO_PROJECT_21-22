#include <stdio.h>
/*Local Imports*/
#include "headers/book_master_shm.h"
#include "headers/glob.h"
/* Helper functions */
/**
 * Copy the block2 in block1
 * @param block1 transactions block vector
 * @param block2 transactions block vector
 */
void copy_trasaction_blocks(struct Transaction block1 [SO_BLOCK_SIZE], struct Transaction block2[SO_BLOCK_SIZE]);
int shm_book_master_create(struct shm_book_master *self) {
    int i;
    for (i = 0; i < SO_REGISTRY_SIZE; i++) {
        self->blocks[i][1].index = i;
    }
    self->to_fill = 0;
    return 0;
}

int insert_block(struct shm_book_master * self, struct Transaction block_t[SO_BLOCK_SIZE]){
    if(self!=NULL){
        copy_trasaction_blocks(self->blocks[self->to_fill][0].block_t, block_t);
        self->blocks[self->to_fill][1].index=self->to_fill;
        self->to_fill+=1;
        return 0;
    }
    return -1;
}

void copy_trasaction_blocks(struct Transaction block1 [SO_BLOCK_SIZE], struct Transaction block2[SO_BLOCK_SIZE]){
    int i;
    for(i=0; i<SO_BLOCK_SIZE; i++){
        block1[i]=block2[i];
    }
}
