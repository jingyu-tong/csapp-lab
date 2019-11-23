/*
 * mm-naive.c - The fastest, least memory-efficient malloc package.
 * 
 * In this naive approach, a block is allocated by simply incrementing
 * the brk pointer.  A block is pure payload. There are no headers or
 * footers.  Blocks are never coalesced or reused. Realloc is
 * implemented directly using mm_malloc and mm_free.
 *
 * NOTE TO STUDENTS: Replace this header comment with your own header
 * comment that gives a high level description of your solution.
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>

#include "mm.h"
#include "memlib.h"

/*********************************************************
 * NOTE TO STUDENTS: Before you do anything else, please
 * provide your team information in the following struct.
 ********************************************************/
team_t team = {
    /* Team name */
    "ateam",
    /* First member's full name */
    "Harry Bovik",
    /* First member's email address */
    "bovik@cs.cmu.edu",
    /* Second member's full name (leave blank if none) */
    "",
    /* Second member's email address (leave blank if none) */
    ""
};

/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8 //对齐

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~0x7) //向上alignment


#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))

//操作空闲链表的常数和宏
#define WSIZE 4 //单字
#define DSIZE 8 //双字
#define CHUNKSIZE (1<<12) //heap每次扩展大小

#define MAX(x,y)    ((x)>(y)?(x):(y))

#define PACK(size,alloc)    ((size) | (alloc)) //结合大小和已分配位

#define GET(p)  (*(unsigned int *)(p)) //读取和返回p引用的值
#define PUT(p,val)  (*(unsigned int *)(p) = (val)) //对p指向位置进行赋值

#define GET_SIZE(p)  (GET(p) & ~0x7) //返回大小
#define GET_ALLOC(p)    (GET(p) & 0x1) //返回已分配标志

//给块指针，计算头部和尾部
#define HDRP(bp)    ((char *)(bp)-WSIZE) 
#define FTRP(bp)    ((char *)(bp)+GET_SIZE(HDRP(bp))-DSIZE)

//调到下一个块，和上一个块
#define NEXT_BLKP(bp)   ((char *)(bp)+GET_SIZE(((char *)(bp)-WSIZE)))
#define PREV_BLKP(bp)   ((char *)(bp)-GET_SIZE(((char *)(bp)-DSIZE)))



static char* heap_list = 0; //static只在本文件能访问

//一些辅助函数
static void* extend_heap(size_t words);
static void* coalesce(void *bp);
static void *find_fit(size_t asize);
static void place(void *bp,size_t asize);

/* 
 * mm_init - initialize the malloc package.
 */
int mm_init(void)
{
    if( (heap_list = mem_sbrk(4 * WSIZE)) == -1) //创建初始堆
        return -1;
    
    PUT(heap_list, 0);
    PUT(heap_list + (1 * WSIZE), PACK(DSIZE, 1)); //序言块头
    PUT(heap_list + (2 * WSIZE), PACK(DSIZE, 1)); //序言块尾
    PUT(heap_list + (3 * WSIZE), PACK(0, 1)); //结尾块

    heap_list += (2 * WSIZE); //移动到尾部，之后方便插入
    if(extend_heap(CHUNKSIZE / WSIZE) == NULL)
        return -1;
    return 0;
}

/* 
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size)
{
    size_t asize; //调整块大小后的大小
    size_t extendsize; //如果找不到，申请新堆的大小
    char *bp;

    if(size ==0) return NULL;

    if(size <= DSIZE){ //调整，块大小至少要2*DISZE
        asize = 2*(DSIZE);
    }else{
        asize = (DSIZE)*((size+(DSIZE)+(DSIZE-1)) / (DSIZE));
    }

    //搜索适合的块
    if((bp = find_fit(asize))!= NULL){
        place(bp,asize);
        return bp;
    }

    //扩展大小，asize和CHUNKSIZE的最大值
    extendsize = MAX(asize,CHUNKSIZE);
    if((bp = extend_heap(extendsize/WSIZE))==NULL){
        return NULL;
    }
    place(bp,asize);
    return bp;
}

/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *ptr)
{
    if(ptr == NULL) return;

    size_t size = GET_SIZE(HDRP(ptr));

    //改变头尾节点，即释放
    PUT(HDRP(ptr), PACK(size, 0));
    PUT(FTRP(ptr), PACK(size, 0));
    
    //尝试合并
    coalesce(ptr);
}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *ptr, size_t size)
{
    size_t oldsize;
    void *newptr;

    //size为0，释放
    if(size == 0) {
        mm_free(ptr);
        return 0;
    }

    //没有指针，malloc即可
    if(ptr == NULL) {
        return mm_malloc(size);
    }

    //否则，申请新的，拷贝，然后释放掉旧的
    newptr = mm_malloc(size);

    /* If realloc() fails the original block is left untouched  */
    if(!newptr) {
        return 0;
    }

    /* Copy the old data. */
    oldsize = GET_SIZE(HDRP(ptr));
    if(size < oldsize) oldsize = size;
    memcpy(newptr, ptr, oldsize);

    /* Free the old block. */
    mm_free(ptr);

    return newptr;
}

//向内存系统请求words大小的堆空间
//words向上舍入为DSIZE的倍数
static void* extend_heap(size_t words) {
    char* bp;
    size_t size;

    //alignment
    size = (words % 2) ? (words + 1) * WSIZE : words * WSIZE;
    if( (bp = mem_sbrk(size)) == -1)
        return NULL;

    //初始化块头和块尾
    //这部分挺有意思，因为堆是向上连续申请的，新申请的在结尾块后面
    //这里把之前的结尾块当做新申请的头部，申请最后字节当做尾部
    PUT(HDRP(bp), PACK(size, 0));
    PUT(FTRP(bp), PACK(size, 0));
    PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 1));

    //尝试合并bp前的块
    return coalesce(bp);
}

//合并空闲块，两种情况被调用:
//1. 向内存系统申请额外堆空间
//2. 初始化的时候
static void* coalesce(void *bp){
    //获取前后分配状态
    size_t  prev_alloc = GET_ALLOC(FTRP(PREV_BLKP(bp)));
    size_t  next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
    size_t size = GET_SIZE(HDRP(bp));

    if(prev_alloc && next_alloc) { //都分配，直接返回
        return bp;
    }else if(prev_alloc && !next_alloc){ //合并前块
            size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
            PUT(HDRP(bp), PACK(size,0));
            PUT(FTRP(bp), PACK(size,0));
    }else if(!prev_alloc && next_alloc){ //合并后块
        size += GET_SIZE(HDRP(PREV_BLKP(bp)));
        PUT(FTRP(bp),PACK(size,0));
        PUT(HDRP(PREV_BLKP(bp)),PACK(size,0));
        bp = PREV_BLKP(bp);
    }else { //前后中一起合并
        size +=GET_SIZE(FTRP(NEXT_BLKP(bp)))+ GET_SIZE(HDRP(PREV_BLKP(bp)));
        PUT(FTRP(NEXT_BLKP(bp)),PACK(size,0));
        PUT(HDRP(PREV_BLKP(bp)),PACK(size,0));
        bp = PREV_BLKP(bp);
    }
    return bp;
}

//采用首次适配方式
static void *find_fit(size_t asize) {
    void *bp;
    
    //遍历所有块
    for(bp = heap_list; GET_SIZE(HDRP(bp))>0; bp = NEXT_BLKP(bp)){
        if(!GET_ALLOC(HDRP(bp)) && (asize <= GET_SIZE(HDRP(bp)))){
            return bp;
        }
    }
    return NULL;
}

//更改空闲块大小
//分成两部分
static void place(void *bp,size_t asize){
    size_t csize = GET_SIZE(HDRP(bp));

    //剩余大于最小块大小
    if((csize-asize)>=(2*DSIZE)){
        PUT(HDRP(bp),PACK(asize,1));
        PUT(FTRP(bp),PACK(asize,1));
        bp = NEXT_BLKP(bp);
        PUT(HDRP(bp),PACK(csize-asize,0));
        PUT(FTRP(bp),PACK(csize-asize,0));
    }else{ //否则，形成内部碎片
        PUT(HDRP(bp),PACK(csize,1));
        PUT(FTRP(bp),PACK(csize,1));
    }
}











