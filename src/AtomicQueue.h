/*
 * A thread-safe Queue that allows pushing from one thread and popping from another.
 *
 * TODO: Add clear() function.
 *
 * https://stackoverflow.com/questions/19101284/implementing-an-atomic-queue-in-qt5
 */

#ifndef ATOMICQUEUE_H
#define ATOMICQUEUE_H

#include <QAtomicPointer>

// Used http://www.drdobbs.com/parallel/writing-lock-free-code-a-corrected-queue/210604448?pgno=2
// as reference
template<class T>
class AtomicQueue
{
     struct QueueNode
     {
          QueueNode( const T& value ) : next( NULL ), data( value ) {}
          ~QueueNode() { /*if ( next ) delete next;*/ }
           QueueNode   *next = 0;
           T           data = 0;
     };

public:
     AtomicQueue()
     {
          m_front = new QueueNode( T() );
          m_tail.store( m_front );
          m_divider.store( m_front );
     }

     AtomicQueue( const AtomicQueue<T>& value ) {

     }

     ~AtomicQueue()
     {
          QueueNode *node = m_front;
          while( node->next )
          {
                QueueNode *n = node->next;
                delete node;
                node = n;
          }
     }

     void push( const T& value )
     {
          m_tail.load()->next = new QueueNode( value );
          m_tail = m_tail.load()->next; // This moves the QueueNode into the atomic pointer, making it safe :)
          while( m_front != m_divider.load() )
          {
                QueueNode *tmp = m_front;
                m_front = m_front->next;
                delete tmp;
          }
     }

     bool peek(T& result)
     {
          if ( m_divider.load() != m_tail.load() )
          {
                // Problem area
                QueueNode *next = m_divider.load()->next;
                if ( next )
                {
                     result = next->data;
                     return true;
                }
          }
          return false;
     }

     bool pop(T& result)
     {
          bool res = this->peek(result);
          if (res)
          {
                m_divider = m_divider.load()->next;
          }
          return res;
     }

private:
     QueueNode                   *m_front;
     QAtomicPointer<QueueNode>   m_divider, m_tail;
};

#endif // ATOMICQUEUE_H
