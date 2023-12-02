#ifndef STACK_H
#define STACK_H

typedef struct NodeStack {
    char* element;
    struct NodeStack* next;
} NodeStack;

typedef struct Stack {
    NodeStack* top;
} Stack;

Stack* initStack();

void SPUSH(Stack* stack, char* element);

char* SPOP(Stack* stack);

void saveToFileStack(Stack* stack, const char* filename, const char* basename, int *pos1, int *pos2, int *status);

Stack* loadFromFileStack(const char* filename, const char* basename, int *pos1, int *pos2, int *status);

#endif
