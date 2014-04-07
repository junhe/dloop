#include <stdio.h>
#include <stdlib.h>
#include <linux/types.h>

#define TABLE_PAIR_COUNT 100

struct mpair {
    blkcnt_t vblock;
    blkcnt_t rblock;
};

static size_t MPAIR_SIZE = sizeof(struct mpair);

static void mtable_print(struct mpair *tb, blkcnt_t count)
{
    blkcnt_t i;
    for ( i = 0; i < count; i++ ) {
        printf("%ld (%ld, %ld); ", 
                i, (tb+i)->vblock, (tb+i)->rblock);
    }
    printf("\n");
}

static struct mpair *mtable_create(size_t pair_count)
{
    struct mpair *ret;
    ret = (struct mpair *)malloc(sizeof(struct mpair)*pair_count);
    /* zero them if necessary */
    return ret;
}

/* given key, this function returns the value,
 * if nothing found, pos will be set. That's the 
 * place where we should insert the vblock if 
 * we want to.
 */
static int mtable_lookup(struct mpair *tb, blkcnt_t vblock, blkcnt_t *pos)
{
    blkcnt_t i;
    struct mpair *mp;
    /* count how many lookups we have done. 
     * prevent from dead loop
     */
    blkcnt_t lookup_cnt = 0; 

    i = vblock * 7 % TABLE_PAIR_COUNT;
    mp = tb + i;
    while( mp->vblock != 0 && lookup_cnt < TABLE_PAIR_COUNT ) {
        /* invariant: mp point to tb[i]*/
        if ( mp->vblock != vblock ) {
            /* not this one, try next one */
            i = (i + 1) % TABLE_PAIR_COUNT;
            mp = tb + i;
        } else {
            /* found it */
            if ( pos != NULL )
                *pos = i;
            return mp->rblock;
        }
        lookup_cnt++;
    }
    if (lookup_cnt >= TABLE_PAIR_COUNT) {
        /* not space left for new key */
        return -2;
    }
    if ( pos != NULL )
        *pos = i;
    /* vblock is not in the table */
    return -1;
}

static int mtable_add_or_overwrite(struct mpair *tb, blkcnt_t vblock, blkcnt_t rblock)
{
    blkcnt_t pos;
    int ret;
    
    ret = mtable_lookup(tb, vblock, &pos);
    if ( ret == -2 ) {
        return -1;
    }

    printf("need to insert/overwrite %ld at %ld\n", vblock, pos);
    struct mpair *p;
    p = tb + pos;
    p->vblock = vblock;
    p->rblock = rblock;
    return 0;
}


int main()
{
    struct mpair *tb = mtable_create(TABLE_PAIR_COUNT);

    blkcnt_t i;
    for ( i = 1; i <= TABLE_PAIR_COUNT+8; i++ ) {
        int ret = mtable_add_or_overwrite(tb, i, i*10);
        if (ret == -1 ) {
            printf("failed to insert\n");
        }
        mtable_print(tb, TABLE_PAIR_COUNT);
    }
    for ( i = 1; i <= TABLE_PAIR_COUNT+8; i++ ) {
        int ret = mtable_lookup(tb, i, NULL);
        printf("%ld %d\n", i, ret);
        /*mtable_print(tb, TABLE_PAIR_COUNT);*/
    }
    free(tb);
}



