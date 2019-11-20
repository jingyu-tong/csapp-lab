# 缓存
这次实验分为两部分，主要是实现缓存，并且利用缓存实现一个转置算法。这题makefile里gcc参数添加了Werror，因此警告也会报错，需要注意。

## 1.缓存
这部分让实现的是基于LRU置换算法的缓存，我们的缓存block结构为：
```c
typedef struct
{
    unsigned tag_; //标记位
    unsigned time_; //时间轴，0代表无效
} block;
```
为了实现LRU，我们需要一个time_来作为判断替换的依据，tag_作为标记位。

接着，我们首先来看主函数：
```c
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
```
整个流程大体是处理参数，然后申请缓存，接着从文件里不断读取出指令进行缓存更新，并且统计三个次数的个数，输出后结束。

参数主要是调用`getopt`，这里就不在赘述了，感兴趣的可以去看代码。申请缓存时，根据定义，我们block的个数是E<<s。然后就是调用`fopen`和`fsacnf`循环处理，更新参数，那么重点就是update该怎么实现。

update代码如下：
```c
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
```
我们首先根据定义算出tag是address的高m - s - b位，index是中间s为，接着就是在set内进行循环了，循环的同时，我们更新最久没被访问过的block，储存在`eviction_block`中，如果需要置换，则把它进行置换。

本部分代码均在traces文件夹下的csim.c文件中，感兴趣的可以去看，测试结果如下：
![cache2](/assets/cache2.png)

## 2.利用缓存
第二部分希望利用缓存实现矩阵的转置，缓存参数为(s = 5, E = 1, b = 5)，也就是每组只有一个block的直接映射，那么缓存的大小为2^(s + b)也就是1k。题目要求的最小为32 * 32的，都存不下，因此我们需要一种子矩阵的方式。

### 32X32
一个block是可以储存32个字节，也就是8个int，对于32*32的，我们分为8*8，这样可以充分的利用缓存，结果如下：
![cache3](/assets/cache3.png)

可以看到，还是超出了300个miss，产生这种主要是因为访问对角线的时候，A和B实际上访问的是同一行，但是他两映射到缓存里是同一个set，那么就会增加miss的次数(来回进入缓存，然后再miss)，我们干脆直接处理对角线上的分块。
这样，我们直接把A全部用缓存访问完，再访问B，避免上述的问题。

### 64X64
64X64的，如果还用8X8的分块的话，第五行开始就超过了32个block，会和前面的发生冲突，增加miss。改成4X4的就不会冲突了，但是没有充分利用缓存，miss次数还是不能达到要求。

我们可以还用8X8的块进行处理，然后把它分成4X4的块进行处理，关系如下:
* B左上角=A左上角的转置，B右上角=A右上角转置
* 存储右上角一行
* 交换左下角一列和右上角一行
* B的右下角=A的右下角的转置

### 61X67
这题要求比较宽松，直接尝试即可，16X16可以满足。

### 结果
本部分代码在traces下的trans.c下，就不贴出了，运行测试程序driver.py，结果如下：
![cache4](/assets/cache4.png)
