## Description

c-linked-stack is a C99 implementation of a stack, that grows by doubling its
size when it is full.
Doubling the capacity does not copy existing items around. Instead it creates a
new stack fragment that is appended to the first one - like a list of stacks.

This is merely a project out of curiosity. For efficiency reasons, a traditional
array like stack is by far preferable for small applications.
Additionally the (by default disabled) shrinking mechanism is not yet optimized.
Any stack-pop resulting in an empty fragment basically triggers a free, which
can result in quite heavy usage of alloc (e.g. worst case "flickering" push+pop
on the boundary of one fragment = each push creates a new fragment, each pop
destroys a fragment).
This could be circumvented by waiting a certain period before removing a
fragment. For example, it could be implemented to wait until half of the
preceeding fragment is empty, before executing the pending removal.

## Usage Example

```c
#include "linked-stack.h"
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

bool print_stack (void *i) {
  printf("%d\n", *(int *)i);
  return true;
}

int main () {

  LinkedStack *stack = NewLinkedStack(10);
  stack->auto_shrink = true;               // delete empty stack fragments
  stack->enable_debug_log = true;

  printf("\nPush initial values (-10 to 10):\n");
  int *ints = malloc(21 * sizeof(*ints));
  for (int i=-10, j=0; i<11; i++, j++) {
    ints[j] = i;
    printf("pushing %d\n", ints[j]);
    stack->push(stack, ints+j);
  }

  printf("\nPrint stack using foreach:\n");
  stack->foreach(stack, print_stack);

  printf("\nPop 5 values and print them:\n");
  for (int i=0; i<5; i++) {
    printf("popped: %d\n", *(int *)stack->pop(stack));
  }

  printf("\nPrint stack using foreach:\n");
  stack->foreach(stack, print_stack);

  printf("\nPush some more values (20 to 29):\n");
  int *ints2 = malloc(10 * sizeof(*ints));
  for (int i=20, j=0; i<30; i++, j++) {
    ints2[j] = i;
    printf("pushing %d\n", ints2[j]);
    stack->push(stack, ints2+j);
  }

  printf("\nPrint stack using foreach:\n");
  stack->foreach(stack, print_stack);
  printf("\n");

  stack->destroy(stack);

  return 0;
}
```

## Configuration

Setting the initial stack size heavily influences the stack growth behaviour.

```c
LinkedStack *stack = NewLinkedStack(INITIAL_STACK_SIZE);
```

Setting the auto-shrink setting will result in the stack reducing its size when
items are popped from the stack. This feature is off by default.

```c
stack->auto_shrink = true;
```

Enabling the debug log will result in some log messages regarding stack
expansion and shrinking. This feature is off by default as well.

```c
stack->enable_debug_log = true;
```

## License

c-linked-stack is licensed under the GNU GPLv3.
Please see the attached License.txt file for details.
Different license terms can be arranged on request.
