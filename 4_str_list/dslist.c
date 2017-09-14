#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "dslist.h"



char * read_line() {
    size_t str_len = 0;
    size_t buf_size = 16; // some initial size
    char *buf = malloc(buf_size * sizeof(char));
    if (buf == NULL) {
        return NULL;
    }

    for (;;) {
        // read next part
        if (fgets(buf + str_len, buf_size - str_len, stdin) == NULL) {
            if (feof(stdin) && str_len > 0) {
                // read all given input
                break;
            } else {
                // no input at all or some error
                free(buf);
                return NULL;
            }
        }

        size_t added_len = strlen(buf + str_len);
        if (added_len == 0) {
            // there is some input but its length is zero
            // (e.g. "\0foo\n")
            // treat it as error
            free(buf);
            return NULL;
        }
        str_len += added_len;
        assert(str_len + 1 <= buf_size);
        assert(buf[str_len] == '\0');

        // check if we finished with reading
        if ((str_len + 1 < buf_size) ||     // read less than maximum
            (buf[str_len - 1] == '\n')) {   // read '\n' as last character
            break;
        }

        // otherwise resize buffer for reading next part
        size_t new_buf_size = buf_size * 2;
        if (new_buf_size <= buf_size) {
            // integer overflow, we will be out of memory
            free(buf);
            return NULL;
        }
        char *new_buf = realloc(buf, new_buf_size);
        if (new_buf == NULL) {
            free(buf);
            return NULL;
        }
        buf_size = new_buf_size;
        buf = new_buf;
    }

    assert(str_len > 0);

    // trim trailing '\n' if any
    if (buf[str_len - 1] == '\n') {
        buf[str_len - 1] = '\0';
    }

    // cut buffer to actual length
    char *result = realloc(buf, str_len + 1);
    if (result == NULL) {
        free(buf);
        return NULL;
    }
    return result;
}



static int check_list(void *list){
    return list != NULL;
}

static void swap_data(DLNode *a, DLNode *b){
    Pointer tmp = a->data;
    a->data = b->data;
    b->data = tmp;
}

static void free_data(DLNode *list){
    if(check_list(list))
    {
        free(list->data);
    }
}

DLNode *dslist_first(DLList *list){
    if(!check_list(list) || list->count == 0){
        return NULL;
    }
    return list->first;
}

static DLList *empty_list(DLNode *node){

    DLList *nl = (DLList*) malloc(sizeof(DLList));
    if(nl == NULL){
        return NULL;
    }

    nl->count = (node != NULL) ? 1 : 0;
    nl->last = node;
    nl->first = node;
    return nl;
}

static DLNode *empty_node(){

    DLNode *nn = (DLNode*) malloc(sizeof(DLNode));
    if(nn == NULL){
        return NULL;
    }

    nn->data = NULL;
    nn->next = NULL;
    nn->prev = NULL;
    return nn;
}

DLNode* dslist_last(DLList *list){
    if(!check_list(list) || list->count == 0){
        return NULL;
    }
    return list->last;
}

DLList *dslist_append(DLList *list, Pointer data){

    DLNode *nn = empty_node();
    if(nn == NULL){
        return NULL;
    }
    nn->data = data;

    if(list == NULL){
        DLList *tmp = empty_list(nn);
        if(tmp == NULL){
            free(nn);
            return NULL;
        }
        return tmp;
    }
    if(list->count > 0)
    {
        nn->prev = list->last;
        list->last->next = nn;
    }
    else{
        list->first = nn;
    }
    list->last = nn;
    list->count++;

    return list;
}

DLList *dslist_prepend(DLList *list, Pointer data){
    DLNode *nn = empty_node();
    if(nn == NULL){
        return NULL;
    }
    nn->data = data;

    if(list == NULL){
        DLList *tmp = empty_list(nn);
        if(tmp == NULL){
            free(nn);
            return NULL;
        }
        return tmp;
    }
    if(list->count > 0)
    {
        nn->next = list->first;
        list->first->prev = nn;
    }
    else{
        list->last = nn;
    }
    list->first = nn;
    list->count++;

    return list;
}

int dslist_insert(DLList *list, DLNode *sibling, Pointer data){

    if(!check_list(list) || sibling == NULL){
        return 0;
    }
    if( list->last == sibling){
        return dslist_append(list, data) != NULL;
    }
    DLNode *nn = empty_node();
    if(nn == NULL){
        return 0;
    }
    nn->data = data;
    nn->next = sibling->next;
    sibling->next = nn;
    nn->prev = sibling;

    return 1;

}

static void dslist_remove_raw(DLNode *list, int freedata){
    if(!check_list(list)){
        return ;
    }
    DLNode *prev = list->prev;
    DLNode *next = list->next;
    if(freedata == 1){
        free_data(list);
    }
    free(list);

    if(next != NULL){
        next->prev = prev;
    }
    if(prev != NULL){
        prev->next = next;
    }
}

static DLList *dslist_remove_few(DLList *list, Pointer data, int count){
    if(!check_list(list) || list->count < 1){
        return 0;
    }
    DLNode *runner = list->first;
    int pos = 0;
    while(runner != NULL){
        if(runner->data == data){
            if(list->first == runner){
                list->first = runner->next;
            }
            if(list->last == runner){
                list->last = runner->prev;
            }
            dslist_remove_raw(runner, 0);
            list->count--;
            pos++;
            if((pos >= count && count > 0) || list->count < 1)
            {
                return list;
            }
        }
    }
    return list;
}

int dslist_remove(DLList *list, Pointer data){
    return dslist_remove_few(list, data, 1) == list;
}

int dslist_remove_all(DLList *list, Pointer data){
    return dslist_remove_few(list, data, -1) == list;
}

Pointer dslist_remove_next(DLList *list, DLNode *sibling){
    if(!check_list(sibling) ||  !check_list(sibling->next)
            || !check_list(list) || list->count < 1){
        return NULL;
    }
    Pointer data= sibling->next->data;
    if(list->first == sibling){
        list->first = sibling->next;
    }
    if(list->last == sibling){
        list->last = sibling->prev;
    }
    dslist_remove_raw(sibling->next, 0);
    list->count--;
    return data;
}

void dslist_free(DLList *list){
    if(!check_list(list)){
        return ;
    }
    DLNode *tmp = list->first, *tmp2;
    while(tmp->next != NULL){
        tmp2 = tmp;
        tmp = tmp->next;
        dslist_remove_raw(tmp2, 0);
    }
    free(tmp); // free last el
    free(list);
}

size_t dslist_length(DLList *list){
    if(!check_list(list)){
        return 0;
    }
    return list->count;
}

int dslist_reverse(DLList *list){
    if(!check_list(list)){
        return 0;
    }

    DLNode *head = list->first;
    DLNode *tail = list->last;

    if(head == tail){
        return 1;
    }

    while(head != tail && (head != NULL) && (tail != NULL)){
        swap_data(head, tail);
        head = head->next;
        tail = tail->prev;
    }

    return 1;
}

DLList *dslist_concat(DLList *list1, DLList *list2){
    if(!check_list(list1)){
        return list2;
    }
    if(!check_list(list2)){
        return list1;
    }
    DLNode *tail1 = list1->last;
    DLNode *head2 = list2->first;

    list1->count += list2->count;
    list1->last = list2->last;

    tail1->next = head2;
    head2->prev = tail1;
    return list1;
}

void dslist_foreach(DLList *list,
                    void (*func)(Pointer data, Pointer user_data), Pointer user_data){
    if(!check_list(list) || func == NULL){
        return ;
    }
    DLNode *head = list->first;
    while(head != NULL){
        func(head->data, user_data);
        head = head->next;
    }
}

DLNode *dslist_find_custom(DLList *haystack, Pointer needle,
                           int (*compare_func)(Pointer a, Pointer b)){
    if(!check_list(haystack) || compare_func == NULL){
        return NULL;
    }
    DLNode *head = dslist_first(haystack);
    while(head != NULL){
        if(!compare_func(head->data, needle)){
            return head;
        }
        head = head->next;
    }
    return NULL;
}

int dslist_position(DLList *list, DLNode *el){
    if(!check_list(list) || !check_list(el)){
        return -1;
    }
    DLNode *head = list->first;
    int pos = 0;
    while(head != NULL){
        if(head == el){
            return pos;
        }
        head = head->next;
        pos++;
    }
    return -1;

}

static void foreach_for_copy(Pointer data, Pointer user_data){
    if(!check_list(data) || !check_list(user_data)){
        return ;
    }

    dslist_append((DLList*) user_data, data);
}

DLList *dslist_copy(DLList *list){

    DLList* nl = empty_list(NULL);
    if(nl == NULL){
        free(nl);
        return NULL;
    }
    dslist_foreach(list, foreach_for_copy, nl);
    return nl;
}

DLNode *dslist_find(DLList *haystack, Pointer needle){
    if(!check_list(haystack)){
        return NULL;
    }
    DLNode *head = haystack->first;
    while(head != NULL){
        if(head->data == needle){
            return head;
        }
        head = head->next;
    }
    return NULL;

}

DLNode *dslist_nth(DLList *list, int n){

    if(!check_list(list)){
        return NULL;
    }
    DLNode *runner;
    if(n > 0)
    {
        runner = list->first;
    }
    else{
        runner = list->last;
    }

    if(list->first == list->last){
        return (abs(n) == 1) ? runner : NULL;
    }
    int pos = 1;
    while(runner != NULL){
        if(abs(n) == pos){
            return runner;
        }
        if(n > 0)
        {
            runner = runner->next;
        }
        else{
            runner = runner->prev;
        }
        pos++;

    }
    return NULL;
}
