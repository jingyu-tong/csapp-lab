#include <getopt.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

#include "cachelab.h"

#define HIT 0
#define MISS 1
#define EVICTION 2

int getOpt(int argc, char** argv);
void printMenu();
void update(char option, unsigned addr, unsigned size, int time);
void cacheState(char op,int type);

int v = 0; //0 不显示 1 显示
//page 426 from csapp3e
int s; //sets的位数
int E; //set中的行数
int b; //block的位数
char tracefile[100]; //trace路径

//命中，未命中，置换
int hits = 0;
int miss = 0;
int eviction = 0;

typedef struct 
{
    unsigned tag_; //标记位
    unsigned time_; //时间轴，0代表无效
} block;
block* cache;


int main(int argc, char** argv)
{
    char option[10];
    int addr, size;
    int time = 0; //时间轴

    if(getOpt(argc, argv) != 0) return 0;

    //申请缓存，然后清0
    cache = (block*)malloc(sizeof(block) * E<<s); 
    memset(cache, 0, sizeof(block) * E<<s);

    //接下来开始读入文件，更新缓存，并且统计hits miss eviction的次数
    FILE* fp = fopen(tracefile, "r");
    while(fscanf(fp, "%s%x,%d", option, &addr, &size) > 0) {
        if(v) printf("%s %x,%d", option, addr, size);
        switch(option[0]) {
            case 'M': ++hits;
            case 'L':
            case 'S': update(option[0], addr, size, ++time);
        }
    } 
    printSummary(hits, miss, eviction);
    fclose(fp);
    free(cache);
    return 0;
}

int getOpt(int argc, char** argv) {
    int parameter;
    while( (parameter = getopt(argc, argv, "hvs:E:b:t:")) != -1) {
        switch(parameter) {
            case 'h': printMenu(); break;
            case 'v': v = 1; break;
            case 's': s = atoi(optarg); break;
            case 'E': E = atoi(optarg); break;
            case 'b': b = atoi(optarg); break;
            case 't': strcpy(tracefile, optarg); break;
            default: printf("inupt error\n"); return -1;
        }
    }
    return 0;
}

void printMenu(){
    printf("Usage: ./csim [-hv] -s <num> -E <num> -b <num> -t <file>\n");
    printf("Options:\n");
    printf("-h         Print this help message.\n");
    printf("-v         Optional verbose flag.\n");
    printf("-s <num>   Number of set index bits.\n");
    printf("-E <num>   Number of lines per set.\n");
    printf("-b <num>   Number of block offset bits.\n");
    printf("-t <file>  Trace file.\n\n\n");
    printf("Examples:\n");
    printf("linux>  ./csim -s 4 -E 1 -b 4 -t traces/yi.trace\n");
    printf("linux>  ./csim -v -s 8 -E 2 -b 4 -t traces/yi.trace\n");
}

void update(char option, unsigned addr, unsigned size, int time) {
    unsigned tag = addr >> s >> b; //m - (b + s)标记block
    unsigned index = addr >> b & ( (1 << s) - 1); //index是中间s位
    block* cache_cur = cache + E * index; //找到所在set
    block* eviction_block = cache_cur; //for LRU

    for(int i = 0; i < E; ++i) {
        if(cache_cur[i].time_ > 0 && cache_cur[i].tag_ == tag) { //命中
            cache_cur[i].time_ = time;
            ++hits;
            if(v) cacheState(option, HIT);
            return;
        } else if(!cache_cur[i].time_) { //找到为无效还未命中，说明不在cache里，miss
            ++miss;
            cache_cur[i].time_ = time;
            cache_cur[i].tag_ = tag;
            if(v) cacheState(option, MISS);
            return;
        } else if(cache_cur[i].time_ < eviction_block->time_) { //更新要被置换的页面
            eviction_block = cache_cur + i;
        }
    }
    
    //置换页面
    ++miss;
    ++eviction;
    eviction_block->tag_ = tag;
    eviction_block->time_ = time;
    if(v) cacheState(option, EVICTION);
}
void cacheState(char op,int type){
    switch(type){
        case HIT: //hit
            switch(op){
                case 'L':
                case 'S':printf("hit\n");break;
                case 'M':printf("hit hit\n");break;
            }break;

        case MISS: //miss
            switch(op){
                case 'L':
                case 'S':printf("miss\n");break;
                case 'M':printf("miss hit\n");break;
            }
        case EVICTION: //eviction
            switch(op){
                case 'L':
                case 'S':printf("miss eviction\n");break;
                case 'M':printf("miss eviction hit\n");break;
            }break;
    }
}