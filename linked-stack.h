#include <stdlib.h>
#include <stdbool.h>

struct linked_stack;
typedef struct linked_stack LinkedStack;

struct linked_stack {
  // private global data
  LinkedStack *_current_fragment; //!< points to the current active stack fragment
  size_t       _stack_size;       //!< current size of the stack (number of elements stored)
  size_t       _stack_max;        //!< current maximum size

  // private local data
  LinkedStack  *_next_fragment; //!< points to the next fragment
  LinkedStack  *_prev_fragment; //!< points to the previous fragment
  void        **_data;          //!< this fragments data base pointer (stack bottom)
  void        **_top;           //!< this fragments data top pointer (last element + 1)
  size_t        _fragment_size; //!< size of this fragment

  // public global members (settings)
  bool auto_shrink;      //!< set to true to enable automatic down sizing when less memory is required
  bool enable_debug_log; //!< set to true if you want some information what is happening behind the scenes

  // public global functions
  bool   (*push)    (LinkedStack *s, void *data);              //!< pushes one element onto the stack
  void * (*pop)     (LinkedStack *s);                          //!< pops one element from the stack
  void * (*peek)    (LinkedStack *s);                          //!< returns the top most element of the stack without removing it
  size_t (*size)    (LinkedStack *s);                          //!< returns the number of elements in the stack
  void   (*foreach) (LinkedStack *s, bool (*fn) (void *data)); //!< calls the given function for every element in the stack
  void   (*destroy) (LinkedStack *s);                          //!< destroys the stack, free'ing all resources
};

LinkedStack *NewLinkedStack (size_t starting_size);
