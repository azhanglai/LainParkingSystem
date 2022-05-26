#include "./epollserver.h"

int main() {
    
    EpollServer epollserver(8092, false, 2, 8);
    
    epollserver.Start();

    return 0;
}
