#include <stdio.h>
#include <stdlib.h>

#define TABLE_PAIR_COUNT 10

struct mpair {
    int vblock;
    int rblock;
};

static size_t MPAIR_SIZE = sizeof(struct mpair);

static void mtable_print(struct mpair *tb, int count)
{
    int i;
    for ( i = 0; i < count; i++ ) {
        printf("%d (%d, %d); ", 
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
static int mtable_lookup(struct mpair *tb, int vblock, int *pos)
{
    int i;
    struct mpair *mp;
    /* count how many lookups we have done. 
     * prevent from dead loop
     */
    int lookup_cnt = 0; 

    i = vblock * 7 % TABLE_PAIR_COUNT;
    mp = tb + i;
    while( mp->vblock != 0 && lookup_cnt <= TABLE_PAIR_COUNT ) {
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
    if ( pos != NULL )
        *pos = i;
    /* vblock is not in the table */
    return -1;
}

static int mtable_add_or_overwrite(struct mpair *tb, int vblock, int rblock)
{
    int pos;
    int ret;
    
    mtable_lookup(tb, vblock, &pos);

    printf("need to insert/overwrite %d at %d\n", vblock, pos);
    struct mpair *p;
    p = tb + pos;
    p->vblock = vblock;
    p->rblock = rblock;
    return 0;
}


int main()
{
    struct mpair *tb = mtable_create(TABLE_PAIR_COUNT);

    int i;
    for ( i = 0; i < TABLE_PAIR_COUNT; i++ ) {
        mtable_add_or_overwrite(tb, 2, 88);
        mtable_print(tb, TABLE_PAIR_COUNT);
    }

    free(tb);
}



