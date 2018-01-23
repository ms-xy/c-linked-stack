#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include "linked-stack.h"

static bool push (LinkedStack *s, void *data);
static void * pop (LinkedStack *s);
static void * peek (LinkedStack *s);
static size_t size (LinkedStack *s);
static void foreach (LinkedStack *s, bool (*fn) (void *data));
static void destroy (LinkedStack *s);
static bool extend_stack (LinkedStack *s);
static void drop_current_fragment (LinkedStack *s);

static inline void debug(LinkedStack *s, const char *format, ...) {
  if (s->enable_debug_log) {
    va_list arglist;
    va_start( arglist, format );
    vprintf( format, arglist );
    va_end( arglist );
  }
}

/**
 * NewLinkedStack allocates data for a new stack struct, as well as for the
 * pointer array.
 *
 * @param starting_size The initial number of free stack slots to be created
 * @param data_size     The size of one single stack slot
 *
 * @return The newly created stack
 */
LinkedStack * NewLinkedStack (size_t starting_size) {
  if (starting_size == 0) {
    return NULL;
  }
  LinkedStack *s = calloc(1, sizeof(*s));
  if (s) {
    void **data = calloc(starting_size, sizeof(*data));
    if (!data) {
      free(s);
      return NULL;
    }
    s->_data = s->_top = data;
    s->_stack_size = 0;
    s->_stack_max = s->_fragment_size = starting_size;
    s->_current_fragment = s;
    s->push = push;
    s->pop = pop;
    s->peek = peek;
    s->size = size;
    s->foreach = foreach;
    s->destroy = destroy;
    s->auto_shrink = false;
    s->enable_debug_log = false;
  }
  return s;
}

/**
 * Saves the data found at void *data into the data array of the stack
 * additionally it extends the stack if necessary (by doubling it).
 *
 * @param s        The stack to work on
 * @param data_ptr The pointer to be saved on the stack
 *
 * @return false if the stack is NULL or extending failed, true otherwise
 */
static bool push (LinkedStack *s, void *data_ptr) {
  if (!s) {
    return false;
  }

  // stack full -> extend
  if (s->_stack_size == s->_stack_max) {
    debug(s, "<debug> full fragment [%p, stack_size=%lu]\n",
      s->_current_fragment, s->_stack_size);
    if (!extend_stack(s)) {
      return false;
    }
  }

  *s->_current_fragment->_top = data_ptr;

  s->_stack_size++;
  s->_current_fragment->_top++;

  return true;
}

/**
 * Gets the top most element, removes it from the stack and returns it, if
 * there is one. It shrinks the stack if possible (by halving it).
 *
 * @param s The stack to work on
 *
 * @return NULL if the stack is empty or NULL, the top most data ptr otherwise
 */
static void * pop (LinkedStack *s) {
  if (!s || s->_stack_size == 0) {
    return NULL;
  }

  s->_stack_size--;

  // drop the current fragment / switch to the previous if current is empty
  if (s->_current_fragment->_top == s->_current_fragment->_data) {
    debug(s, "<debug> empty fragment [%p, frag_size=%lu, stack_size=%lu]\n",
      s->_current_fragment, s->_fragment_size, s->_stack_size);
    drop_current_fragment(s);
  }

  // decrement current top pointer to access the top most element
  s->_current_fragment->_top--;

  return *s->_current_fragment->_top;
}

/**
 * Returns the top most element (if exists, otherwise null), but does not
 * remove it (unlike pop).
 *
 * @param s The stack to work on
 *
 * @return NULL if the stack is empty or NULL, the top most data ptr otherwise
 */
static void * peek (LinkedStack *s) {
  if (!s || s->_stack_size == 0) {
    return NULL;
  }
  if (s->_current_fragment->_top == s->_current_fragment->_data) {
    return *(s->_current_fragment->_prev_fragment->_top - 1);
  }
  return *(s->_current_fragment->_top - 1);
}

/**
 * Returns the current number of elements stored inside of the stack.
 *
 * @param s The stack to work on
 *
 * @return The current number of elements stored in the stack
 */
static size_t size (LinkedStack *s) {
  return s->_stack_size;
}

/**
 * Iterates all stored elements, calling the given function for each of them.
 *
 * @param s The stack to work on
 * @param fn The function to call for each element
 */
static void foreach (LinkedStack *s, bool (*fn) (void *data)) {
  size_t i, max;
  LinkedStack *c = s;
  while(c) {
    max = (c->_next_fragment) ? c->_fragment_size : (c->_top - c->_data);
    for (i=0; i<max; i++) {
      if (!fn(*(c->_data + i))) {
        return;
      }
    }
    c = c->_next_fragment;
  }
}

/**
 * Extends the stack by doubling it.
 * Returns false if the extension failed.
 *
 * @param s The stack to work on
 *
 * @return Returns true if the stack extension succeeded, false otherwise
 */
static bool extend_stack (LinkedStack *s) {
  // if the current fragment already has a next, re-use it
  if (s->_current_fragment->_next_fragment) {
    debug(s, "<debug> re-using existing next-fragment "
      "[cur_frag=%p, next_frag=%p, cur_frag_size=%lu, next_frag_size=%lu, "
      "old_stack_max=%lu, new_stack_max=%lu]\n",
      s->_current_fragment, s->_current_fragment->_next_fragment,
      s->_current_fragment->_fragment_size, s->_current_fragment->_next_fragment->_fragment_size,
      s->_stack_max, s->_stack_max + s->_current_fragment->_next_fragment->_fragment_size);

    s->_current_fragment = s->_current_fragment->_next_fragment;
    s->_stack_max += s->_current_fragment->_fragment_size;
    return true;
  }
  // allocate struct and ptr array
  LinkedStack *new_fragment = calloc(1, sizeof(*new_fragment));
  if (!new_fragment) {
    return false;
  }
  void *data = calloc(s->_stack_max, sizeof(*s->_data));
  if (!data) {
    free(new_fragment);
    return false;
  }
  // set data
  new_fragment->_data = new_fragment->_top = data;
  new_fragment->_fragment_size = s->_stack_max;
  // update links
  new_fragment->_prev_fragment = s->_current_fragment;
  s->_current_fragment->_next_fragment = new_fragment;
  s->_current_fragment = new_fragment;
  // update stack size
  s->_stack_max += new_fragment->_fragment_size;

  debug(s, "<debug> appended new fragment "
    "[%p, frag_size=%lu, old_stack_max=%lu, new_stack_max=%lu]\n",
    s->_current_fragment,
    s->_current_fragment->_fragment_size,
    s->_stack_max - new_fragment->_fragment_size, s->_stack_max);
  return true;
}

/**
 * Drops the current stack frame, effectively halving the entire stack space.
 *
 * @param s The stack to work on
 */
static void drop_current_fragment (LinkedStack *s) {
  LinkedStack *drop_fragment = s->_current_fragment;
  // tackle auto-shrink = false (default)
  if (!s->auto_shrink) {
    s->_current_fragment = drop_fragment->_prev_fragment;
    s->_stack_max -= drop_fragment->_fragment_size;

    debug(s, "<debug> switching to previous fragment "
      "[cur_frag=%p, prev_frag=%p, "
      "cur_frag_size=%lu, prev_frag_size=%lu, "
      "old_stack_max=%lu, new_stack_max=%lu]\n",
      drop_fragment, s->_current_fragment,
      drop_fragment->_fragment_size, s->_current_fragment->_fragment_size,
      s->_stack_max, s->_stack_max - drop_fragment->_fragment_size);
    return;
  }
  // recursively drop child fragments first
  if (drop_fragment->_next_fragment) {
    drop_current_fragment(drop_fragment->_next_fragment);
  }
  // then drop this one
  s->_current_fragment = drop_fragment->_prev_fragment;
  s->_current_fragment->_next_fragment = NULL;
  s->_stack_max -= drop_fragment->_fragment_size;

  debug(s, "<debug> dropping fragment "
    "[%p, new_cur_frag=%p, "
    "drop_frag_size=%lu, new_cur_frag_size=%lu, "
    "old_stack_max=%lu, new_stack_max=%lu]\n",
    drop_fragment, s->_current_fragment,
    drop_fragment->_fragment_size, s->_current_fragment->_fragment_size,
    s->_stack_max + drop_fragment->_fragment_size, s->_stack_max);
  free(drop_fragment->_data);
  free(drop_fragment);
}

/**
 * Destroys the entire stack, free'ing all occupied resources.
 */
static void destroy (LinkedStack *s) {
  LinkedStack *drop;
  for (; s->_next_fragment; s = s->_next_fragment);
  while ((drop = s) != NULL) {
    s = s->_prev_fragment;
    free(drop->_data);
    free(drop);
  }
}
