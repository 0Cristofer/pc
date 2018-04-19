///////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2005, 2006, 2007, 2008, 2009
// University of Rochester
// Department of Computer Science
// All rights reserved.
//
// Copyright (c) 2009, 2010
// Lehigh University
// Computer Science and Engineering Department
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
//    * Redistributions of source code must retain the above copyright notice,
//      this list of conditions and the following disclaimer.
//
//    * Redistributions in binary form must reproduce the above copyright
//      notice, this list of conditions and the following disclaimer in the
//      documentation and/or other materials provided with the distribution.
//
//    * Neither the name of the University of Rochester nor the names of its
//      contributors may be used to endorse or promote products derived from
//      this software without specific prior written permission.
//
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.

#include <iostream>
#include <cstdlib>
#include <pthread.h>

#include "LinkedList.hpp"

using namespace bench;

// constructor just makes a sentinel for the data structure
LinkedList::LinkedList() : sentinel(new LLNode()) { }

// simple sanity check:  make sure all elements of the list are in sorted order
bool LinkedList::isSane(void) const
{
    bool sane = true;
    const LLNode* prev(sentinel);
    const LLNode* curr((prev->m_next));

    while (curr != NULL) {
        if ((prev->m_val) >= (curr->m_val)) {
            sane = false;
            break;
        }
        prev = curr;
        curr = (curr->m_next);
    }
    return sane;
}

// extended sanity check, does the same as the above method, but also calls v()
// on every item in the list
bool LinkedList::extendedSanityCheck(verifier v, unsigned long v_param) const
{
    bool sane =  true;
    const LLNode* prev(sentinel);
    const LLNode* curr((prev->m_next));
    while (curr != NULL) {
        if (!v((curr->m_val), v_param) ||
            ((prev->m_val) >= (curr->m_val)))
        {
            sane = false;
            break;
        }
        prev = curr;
        curr = (prev->m_next);
    }
    return sane;
}

/* OLD INSERT
TX_DECL
void LinkedList::insert(int val)
{
    stm::TxThread& tx = *stm::TxThread::Self;
    if (!bench::early_tx_terminate) {
        // traverse the list to find the insertion point
        const LLNode* prev(sentinel);
        const LLNode* curr(TXREAD(prev->m_next));

        while (curr != NULL) {
            if (TXREAD(curr->m_val) >= val)
                break;
            prev = curr;
            curr = TXREAD(prev->m_next);
        }

        // now insert new_node between prev and curr
        if (!curr || (TXREAD(curr->m_val) > val)) {
            LLNode* insert_point = const_cast<LLNode*>(prev);
            TXWRITE(insert_point->m_next,
                     new LLNode(val, const_cast<LLNode*>(curr)));
        }
    }
}
*/

// insert method; find the right place in the list, add val so that it is in
// sorted order; if val is already in the list, exit without inserting
void LinkedList::insert(int val)
{
  // traverse the list to find the insertion point
  const LLNode* prev(sentinel);
  const LLNode* curr(prev->m_next);

  while (curr != NULL) {
    if (curr->m_val >= val)
      break;

    prev = curr;
    curr = prev->m_next;
  }

  // now insert new_node between prev and curr
  if (!curr || curr->m_val > val)) {
    LLNode* insert_point = const_cast<LLNode*>(prev);

    // ESCRITA : REGIÃO CRITICA
    insert_point->m_next = new LLNode(val, const_cast<LLNode*>(curr)));
    // FIM
    }

}

/* OLD search function
TX_DECL
bool LinkedList::lookup(int val) const
{
    stm::TxThread& tx = *stm::TxThread::Self;
    bool found = false;
    if (!bench::early_tx_terminate) {
        const LLNode* curr(sentinel);
        curr = TXREAD(curr->m_next);

        while (curr != NULL) {
            if (TXREAD(curr->m_val) >= val)
                break;
            curr = TXREAD(curr->m_next);
        }

        found = ((curr != NULL) && (TXREAD(curr->m_val) == val));
    }
    return found;
}
*/

// search function
bool LinkedList::lookup(int val) const
{
    bool found = false;

    const LLNode* curr(sentinel);
    curr = curr->m_next;

    while (curr != NULL) {
      if (curr->m_val >= val)
        break;

      curr = curr->m_next;
    }

    found = ((curr != NULL) && (curr->m_val == val));

    return found;
}

/* OLD findmax function
TX_DECL
int LinkedList::findmax() const
{
    stm::TxThread& tx = *stm::TxThread::Self;
    int max = -1;
    const LLNode* curr(sentinel);
    while (curr != NULL) {
        max = TXREAD(curr->m_val);
        curr = TXREAD(curr->m_next);
    }
    return max;
}
*/

// findmax function
int LinkedList::findmax() const
{
    int max = -1;
    const LLNode* curr(sentinel);
    while (curr != NULL) {
        max = curr->m_val;
        curr = curr->m_next;
    }
    return max;
}

/* OLD findmin function
TX_DECL
int LinkedList::findmin() const
{
    stm::TxThread& tx = *stm::TxThread::Self;
    int min = -1;
    const LLNode* curr(sentinel);
    curr = TXREAD(curr->m_next);
    if (curr != NULL)
        min = TXREAD(curr->m_val);
    return min;
}
*/

// findmin function
int LinkedList::findmin() const
{
    int min = -1;
    const LLNode* curr(sentinel);
    curr = curr->m_next;
    if (curr != NULL)
        min = curr->m_val;
    return min;
}

//OLD remove a node if its value == val
TX_DECL
void LinkedList::remove(int val)
{
    stm::TxThread& tx = *stm::TxThread::Self;
    if (!bench::early_tx_terminate) {
        // find the node whose val matches the request
        const LLNode* prev(sentinel);
        const LLNode* curr(TXREAD(prev->m_next));
        while (curr != NULL) {
            // if we find the node, disconnect it and end the search
            if (TXREAD(curr->m_val) == val) {
                LLNode* mod_point = const_cast<LLNode*>(prev);
                TXWRITE(mod_point->m_next, TXREAD(curr->m_next));

                // delete curr...
                tx_free(const_cast<LLNode*>(curr));
                break;
            }
            else if (TXREAD(curr->m_val) > val) {
                // this means the search failed
                break;
            }
            prev = curr;
            curr = TXREAD(prev->m_next);
        }
    }
}


// remove a node if its value == val
void LinkedList::remove(int val)
{
  // find the node whose val matches the request
  const LLNode* prev(sentinel);
  const LLNode* curr(prev->m_next);
  while (curr != NULL) {
    // if we find the node, disconnect it and end the search
    if (curr->m_val == val) {
      LLNode* mod_point = const_cast<LLNode*>(prev);

      // ESCRITA : REGIÃO CRITICA
      mod_point->m_next = curr->m_next;

      // delete curr...
      free(const_cast<LLNode*>(curr));
      // FIM
      break;
    }
    else if (curr->m_val > val) {
      // this means the search failed
      break;
    }
    prev = curr;
    curr = prev->m_next;
  }
}

// print the list
void LinkedList::print() const
{
    const LLNode* curr(sentinel);
    curr = (curr->m_next);
    std::cout << "list :: ";
    while (curr != NULL) {
        std::cout << (curr->m_val) << "->";
        curr = (curr->m_next);
    }
    std::cout << "NULL" << std::endl;
}

/*OLD search function
TX_DECL
void LinkedList::overwrite(int val)
{
    stm::TxThread& tx = *stm::TxThread::Self;
        if (!bench::early_tx_terminate) {
            const LLNode* curr(sentinel);
            curr = TXREAD(curr->m_next);

            while (curr != NULL) {
                if (TXREAD(curr->m_val) >= val)
                    break;
                LLNode* wcurr = const_cast<LLNode*>(curr);
                TXWRITE(wcurr->m_val, TXREAD(wcurr->m_val));
                curr = TXREAD(wcurr->m_next);
            }
        }
}
*/

// search function
void LinkedList::overwrite(int val)
{
  const LLNode* curr(sentinel);
  curr = curr->m_next;

  while (curr != NULL) {
    if (curr->m_val >= val)
      break;

    LLNode* wcurr = const_cast<LLNode*>(curr);

    // ESCRITA : REGIÃO CRITICA
    wcurr->m_val = wcurr->m_val;
    // FIM

    curr = wcurr->m_next;
  }
}
