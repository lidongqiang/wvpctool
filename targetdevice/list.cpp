#include "stdafx.h"
#include <Setupapi.h>
#include "list.h"




/*---------------------------------------------------------------------------
 *            _InsertTailList()
 *---------------------------------------------------------------------------
 *
 * Synopsis:  Insert an entry at the tail of the list specified by head.
 *
 * Return:    void
 */
void _InsertTailList(ListEntry* head, ListEntry* entry)
{
  //Assert(entry != head->Blink);
  entry->Flink = head;
  entry->Blink = head->Blink;
  head->Blink->Flink = entry;
  head->Blink = entry;
  //Assert(IsNodeConnected(entry));

}

/*---------------------------------------------------------------------------
 *            _InsertHeadList()
 *---------------------------------------------------------------------------
 *
 * Synopsis:  Insert an entry at the head of the list specified by head.
 *
 * Return:    void
 */
void _InsertHeadList(ListEntry* head, ListEntry* entry)
{
  entry->Flink = head->Flink;
  entry->Blink = head;
  head->Flink->Blink = entry;
  head->Flink = entry;
  //Assert(IsNodeConnected(entry));

}

/*---------------------------------------------------------------------------
 *            _RemoveHeadList()
 *---------------------------------------------------------------------------
 *
 * Synopsis:  Remove the first entry on the list specified by head.
 *
 * Return:    void
 */
ListEntry* _RemoveHeadList(ListEntry* head)
{
  ListEntry* first;

  first = head->Flink;
  first->Flink->Blink = head;
  head->Flink = first->Flink;
  //Assert(IsListCircular(head));
  return(first);

}

/*---------------------------------------------------------------------------
 *           RemoveEntryList()
 *---------------------------------------------------------------------------
 *
 * Synopsis:  Remove the given entry from the list.
 *
 * Return:    void
 *
 */
void RemoveEntryList(ListEntry* entry)
{
  //Assert(IsListCircular(entry));
  entry->Blink->Flink = entry->Flink;
  entry->Flink->Blink = entry->Blink;
  //Assert(IsListCircular(entry->Blink));
  InitializeListEntry(entry);

}

/*---------------------------------------------------------------------------
 *            IsNodeOnList()
 *---------------------------------------------------------------------------
 *
 * Synopsis:  Determine if an entry is on the list specified by head.
 *
 * Return:    TRUE - the entry is on the list.
 *            FALSE - the entry is not on the list.
 */
BOOL IsNodeOnList(ListEntry* head, ListEntry* node)
{
  ListEntry* tmpNode;

  //Assert(IsListCircular(head));
  tmpNode = GetHeadList(head);

  while (tmpNode != head)
  {
    if (tmpNode == node)
      return(TRUE);

    tmpNode = tmpNode->Flink;
  }
  return(FALSE);

}