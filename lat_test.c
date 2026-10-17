#include <stdio.h>
#include <stdlib.h>
#include <time.h>

typedef struct Node {
    struct Node *next;
} Node;

// 构建随机链表
static Node *make_chain(size_t bytes)
{
    size_t count = bytes / sizeof(Node);
    Node *mem = malloc(bytes);
    if (!mem) return NULL;

    Node **arr = malloc(count * sizeof(Node*));
    size_t i;
    for(i=0;i<count;i++) arr[i] = mem + i;

    //简单洗牌
    for(i=count-1; i>0; i--){
        size_t r = rand() % (i+1);
        Node *tmp = arr[i];
        arr[i] = arr[r];
        arr[r] = tmp;
    }
    for(i=0;i<count-1;i++) arr[i]->next = arr[i+1];
    arr[count-1]->next = arr[0];
    free(arr);
    return mem;
}

//遍历链表多次，计算延迟
static double measure_latency(Node *chain, size_t loops)
{
    struct timespec t0,t1;
    Node * volatile p = chain; // volatile关键：防止优化
    clock_gettime(CLOCK_MONOTONIC, &t0);
    for(size_t i=0;i<loops;i++){
        p = p->next;
    }
    clock_gettime(CLOCK_MONOTONIC, &t1);
    double sec = (t1.tv_sec - t0.tv_sec) + (t1.tv_nsec - t0.tv_nsec)/1e9;
    double ns_per_access = sec * 1e9 / loops;
    return ns_per_access;
}

int main(int argc,char** argv)
{
    if(argc !=3){
        printf("Usage: %s mem_size_kb loops\n",argv[0]);
        printf("Example: %s 8192 1000000\n",argv[0]);
        printf("mem_size_kb: buffer size(KB), larger than L2 will hit DDR\n");
        printf("loops: iteration count\n");
        return 1;
    }
    size_t size_kb = atoi(argv[1]);
    size_t loops = atoll(argv[2]);
    size_t bytes = size_kb * 1024;

    Node *chain = make_chain(bytes);
    if(!chain){
        printf("malloc fail\n");
        return -1;
    }

    double lat = measure_latency(chain, loops);
    printf("Buffer: %zu KB, Random read latency: %.2f ns\n", size_kb, lat);

    free(chain);
    return 0;
}
