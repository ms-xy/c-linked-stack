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
  stack->auto_shrink = true;
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
