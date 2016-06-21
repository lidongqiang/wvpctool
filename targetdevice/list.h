#ifndef _LIST_H_
#define _LIST_H_
typedef struct  _ListEntry 
{
    struct _ListEntry *Flink;
    struct _ListEntry *Blink;
} ListEntry;
#define InitializeListHead(ListHead) (\
    (ListHead)->Flink = (ListHead)->Blink = (ListHead) )
#define IsListEmpty(ListHead) (\
    ((ListHead)->Flink == (ListHead)))
#define InitializeListEntry(Entry) (\
    (Entry)->Flink = (Entry)->Blink = 0 )
#define GetHeadList(ListHead) (ListHead)->Flink
#define InsertTailList(a, b) _InsertTailList(a, b)

#define RemoveHeadList(a) _RemoveHeadList(a))

#define CONTAINING_RECORD(address, type, field) ((type *)( \
                                                  (PCHAR)(address) - \
                                                  (ULONG_PTR)(&((type *)0)->field)))

void        _InsertTailList(ListEntry* , ListEntry* );
void        _InsertHeadList (ListEntry* , ListEntry* );
ListEntry   *_RemoveHeadList(ListEntry* );
void        RemoveEntryList (ListEntry* );
BOOL        IsNodeOnList    (ListEntry* , ListEntry* );
#endif