/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 *
 * The contents of this file are subject to the Netscape Public License
 * Version 1.0 (the "NPL"); you may not use this file except in
 * compliance with the NPL.  You may obtain a copy of the NPL at
 * http://www.mozilla.org/NPL/
 *
 * Software distributed under the NPL is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the NPL
 * for the specific language governing rights and limitations under the
 * NPL.
 *
 * The Initial Developer of this code under the NPL is Netscape
 * Communications Corporation.  Portions created by Netscape are
 * Copyright (C) 1998 Netscape Communications Corporation.  All Rights
 * Reserved.
 */

#include "nsTransactionStack.h"
#include "COM_auto_ptr.h"

nsTransactionStack::nsTransactionStack()
  : mRF(), mQue(mRF)
{
}

nsTransactionStack::~nsTransactionStack()
{
  Clear();
}

nsresult
nsTransactionStack::Push(nsITransaction *aTransaction)
{
  if (!aTransaction)
    return NS_ERROR_NULL_POINTER;

  /* nsDeque's Push() method adds new items at the back
   * of the deque.
   */
  mQue.Push(aTransaction);

  return NS_OK;
}

nsresult
nsTransactionStack::Pop(nsITransaction **aTransaction)
{
  if (!aTransaction)
    return NS_ERROR_NULL_POINTER;

  /* nsDeque is a FIFO, so the top of our stack is actually
   * the back of the deque.
   */
  *aTransaction = (nsITransaction *)mQue.PopBack();

  return NS_OK;
}

nsresult
nsTransactionStack::PopBottom(nsITransaction **aTransaction)
{
  if (!aTransaction)
    return NS_ERROR_NULL_POINTER;

  /* nsDeque is a FIFO, so the bottom of our stack is actually
   * the front of the deque.
   */
  *aTransaction = (nsITransaction *)mQue.Pop();

  return NS_OK;
}

nsresult
nsTransactionStack::Peek(nsITransaction **aTransaction)
{
  if (!aTransaction)
    return NS_ERROR_NULL_POINTER;

  /* nsDeque's End() method returns an iterator that is
   * passed the last entry of the deque. We need to decrement
   * to get to the last entry.
   */
  *aTransaction = (nsITransaction *)(--mQue.End());

  return NS_OK;
}

nsresult
nsTransactionStack::Clear(void)
{
  nsITransaction *tx = 0;
  nsresult result    = NS_OK;

  /* Pop all transactions off the stack and release them. */

  result = Pop(&tx);

  if (! NS_SUCCEEDED(result))
    return result;

  while (tx) {
    NS_IF_RELEASE(tx);

    result = Pop(&tx);

    if (! NS_SUCCEEDED(result))
      return result;
  }

  return NS_OK;
}

nsresult
nsTransactionStack::GetSize(PRInt32 *aStackSize)
{
  if (!aStackSize)
    return NS_ERROR_NULL_POINTER;

  *aStackSize = mQue.GetSize();

  return NS_OK;
}

nsresult
nsTransactionStack::Write(nsIOutputStream *aOutputStream)
{
  /* Call Write() for every transaction on the stack. Start
   * from the top of the stack.
   */

  nsDequeIterator di = mQue.End();
  nsITransaction *tx = (nsITransaction *)--di;

  while (tx) {
    tx->Write(aOutputStream);
    tx = (nsITransaction *)--di;
  }
}


nsresult
nsTransactionRedoStack::Clear(void)
{
  nsITransaction *tx = 0;
  nsresult result    = NS_OK;

  /* When clearing a Redo stack, we have to clear from the
   * bottom of the stack towards the top!
   */

  result = PopBottom(&tx);

  if (! NS_SUCCEEDED(result))
    return result;

  while (tx) {
    NS_IF_RELEASE(tx);

    result = PopBottom(&tx);

    if (! NS_SUCCEEDED(result))
      return result;
  }

  return NS_OK;
}
