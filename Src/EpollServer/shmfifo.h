#ifndef __SHMFIFO_H
#define __SHMFIFO_H

#include "./define.h"
#include <iostream>
using namespace std;

typedef struct ShmHead {
    int r_index;
    int w_index;
    int blocks;
    int block_size;
} SHM_HEAD_T;

class ShmFifo {
public:
    SHM_HEAD_T* shm_start_addr;
    char* shm_data_add;
    int shm_id;

    ShmFifo(int key, int blocks, int block_size) {
        int size = sizeof(SHM_HEAD_T) + blocks * block_size;
        // 1. shm_get --> shm_id 
        this->shm_id = shmget(key, size, IPC_CREAT | IPC_EXCL | 0666);
        if (this->shm_id == -1) {
            this->shm_id = shmget(key, size, IPC_CREAT | 0666);
            // 2. shmat --> shm_addr
            this->shm_start_addr = (SHM_HEAD_T*)shmat(this->shm_id, NULL, 0);
            this->shm_data_add = (char*)(this->shm_start_addr + 1);
            this->shm_start_addr->blocks = blocks;
            this->shm_start_addr->block_size = block_size;
            this->shm_start_addr->w_index = 0;
            this->shm_start_addr->r_index = 0;
        } else {
            this->shm_start_addr = (SHM_HEAD_T*)shmat(this->shm_id, NULL, 0);
            this->shm_data_add = (char*)(this->shm_start_addr + 1);
        }
    }

    char *shm_read() {
        char* buf = (char*)malloc(this->shm_start_addr->block_size);
        // 把共享内存对应的位置数据拷贝到buf
        memcpy(buf, this->shm_data_add + this->shm_start_addr->r_index * this->shm_start_addr->block_size, this->shm_start_addr->block_size);

        this->shm_start_addr->r_index = (this->shm_start_addr->r_index + 1) % this->shm_start_addr->blocks;

        return buf;
    }

    void shm_write(char* buf) {
        memcpy(this->shm_data_add + this->shm_start_addr->w_index * this->shm_start_addr->block_size, buf, this->shm_start_addr->block_size);

        this->shm_start_addr->w_index = (this->shm_start_addr->w_index + 1) % this->shm_start_addr->blocks;
    }

};

#endif /* __SHMFIFO_H */

