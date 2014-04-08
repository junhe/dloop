#include <stdio.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <linux/types.h>

struct mpair {
    blkcnt_t vblock;
    blkcnt_t rblock;
};

struct mtable {
    struct mpair* pairs;
    blkcnt_t      first_free_block;
    blkcnt_t      count; /* num of pairs */
};

static void mtable_print(struct mtable *tb)
{
    blkcnt_t i;
    struct mpair *mp;

    mp = tb->pairs;    
    for ( i = 0; i < tb->count; i++ ) {
        printf("%lu (%lu, %lu); ", 
                i, (mp+i)->vblock, (mp+i)->rblock);
    }
    printf("\n");
}

static struct mtable *mtable_create(size_t pair_count)
{
    struct mtable *tb;
    unsigned long size;

    tb = (struct mtable *)malloc(sizeof(struct mtable));
    if ( tb == NULL ) {
        /* print something */
        return NULL;
    }

    tb->count = pair_count;
    tb->first_free_block = 0;

    size = sizeof(struct mpair) * pair_count;
    tb->pairs = (struct mpair *)malloc(size);

    if ( tb->pairs == NULL ) {
        /* print error */
        return NULL;
    } 
    /* zero them if necessary */
    memset(tb->pairs, 0xff, size);
    return tb;
}

/* given key, this function returns the value,
 * if nothing found, pos will be set. That's the 
 * place where we should insert the vblock if 
 * we want to.
 * return:
 * 0: found vblock, rblock and pos are set
 * -1: cannot found vblock, still have free space,
 *     pos is set
 * -2: cannot found vblock, not free space to insert 
 *     new
 */
static int mtable_lookup(struct mtable *tb, 
                         blkcnt_t vblock, 
                         blkcnt_t *rblock,
                         unsigned long *pos)
{
    blkcnt_t i;
    struct mpair *mp;

    if ( tb->count == 0 ) {
        return -2;
    }

    /* count how many lookups we have done. 
     * prevent from dead loop
     */
    blkcnt_t lookup_cnt = 0; 

    i = vblock * 7 % tb->count;
    mp = tb->pairs + i;
    while( mp->vblock != ULONG_MAX && lookup_cnt < tb->count ) {
        /* invariant: mp point to tb[i]*/
        if ( mp->vblock != vblock ) {
            /* not this one, try next one */
            i = (i + 1) % tb->count;
            mp = tb->pairs + i;
        } else {
            /* found it */
            if ( pos != NULL )
                *pos = i;
            *rblock = mp->rblock;
            return 0;
        }
        lookup_cnt++;
    }
    if (lookup_cnt >= tb->count) {
        /* not space left for new key */
        return -2;
    }
    if ( pos != NULL )
        *pos = i;
    /* vblock is not in the table,
     * you can insert at pos
     */
    return -1;
}

static void mtable_write(struct mtable *tb, unsigned long pos, 
                        blkcnt_t vblock,   blkcnt_t rblock )
{
    struct mpair *p;

    p = tb->pairs + pos;
    p->vblock = vblock;
    p->rblock = rblock;
    return ;
}

static int mtable_add_or_overwrite(struct mtable *tb, blkcnt_t vblock, blkcnt_t rblock)
{
    unsigned long pos;
    blkcnt_t ret_rblock;
    int ret;
    
    ret = mtable_lookup(tb, vblock, &ret_rblock, &pos);
    if ( ret == -2 ) {
        return -1;
    }

    /*printf("need to insert/overwrite %ld at %ld\n", vblock, pos);*/
    /* add or overwrite */
    mtable_write(tb, pos, vblock, rblock);
    return 0;
}

/*
 * This function first tries to find the corresponding block
 * of vblock, if it cannot, it will create a new mapping
 */
static int mtable_get_rblock (struct mtable *tb, 
                                  blkcnt_t vblock,
                                  blkcnt_t *rblock)
{
    unsigned long pos;
    blkcnt_t ret_rblock;
    int ret;
    
    ret = mtable_lookup(tb, vblock, &ret_rblock, &pos);
    if ( ret == 0 ) {
        /* found rblock */
        *rblock = ret_rblock;
        return 0;
    } else if ( ret == -1 ) {
        /* need to add a new mapping */
        blkcnt_t new_rblock;

        new_rblock = tb->first_free_block;
        tb->first_free_block++;

        mtable_write(tb, pos, vblock, new_rblock);
        *rblock = new_rblock;
        return 0;
    } else if ( ret == -2 ) {
        /* full table, no chance adding new mapping */
        return -2;
    }
}

static void mtable_release(struct mtable *tb)
{
    free(tb->pairs);
    free(tb);
}

int main()
{
    struct mtable *tb = mtable_create(21);

    /*printf("%lu %X %lu\n", tb->pairs->vblock, tb->pairs->vblock, ULONG_MAX);*/
    /*printf("%lu", sizeof(unsigned long));*/
    /*if ( tb->pairs->vblock == ULONG_MAX ) {*/
        /*printf("equal\n");*/
    /*} else {*/
        /*printf("not\n");*/
    /*}*/

    /*return 0;*/

    blkcnt_t i;
    for ( i = 0; i < 20; i++ ) {
        blkcnt_t rblock;
        int ret = mtable_get_rblock(tb, i, &rblock);
    }
    mtable_print(tb);
    for ( i = 22; i >= 0; i-- ) {
        blkcnt_t rblock;
        int ret = mtable_get_rblock(tb, i, &rblock);
        if ( ret == 0 ) {
            printf("%lu %lu\n", i, rblock);
        } else {
            printf("cannot find.\n");
        }
    }

    free(tb);
}



