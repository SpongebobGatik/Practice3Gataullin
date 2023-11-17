#define _CRT_SECURE_NO_WARNINGS
#include "stack.h" 
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 

int countWordsInFileStack(const char* filename) {
    FILE* file = fopen(filename, "r");
    int count = 0;
    char word[10000];
    while (fscanf(file, "%s", word) != EOF) {
        count++;
    }
    fclose(file);
    return count;
}

Stack* initStack() {
    Stack* stack = (Stack*)malloc(sizeof(Stack));
    stack->top = NULL;
    return stack;
}

void SPUSH(Stack* stack, char* element) {
    NodeStack* newNode = (NodeStack*)malloc(sizeof(NodeStack));
    newNode->element = _strdup(element);
    newNode->next = stack->top;
    stack->top = newNode;
}

char* SPOP(Stack* stack) {
    if (stack->top == NULL) {
        return ("The stack is empty");
    }
    NodeStack* poppedNode = stack->top;
    stack->top = poppedNode->next;
    char* element = poppedNode->element;
    free(poppedNode);
    return element;
}

void saveToFileStack(Stack* stack, const char* filename, const char* basename, int* pos1, int* pos2, int* status) {
    FILE* file = fopen(filename, "r");
    FILE* tempFile = fopen("temp.data", "w");
    int ch;
    fseek(file, 0, SEEK_SET);
    fseek(tempFile, 0, SEEK_SET);
    while ((ch = fgetc(file)) != EOF) {
        fputc(ch, tempFile);
        if (ftell(tempFile) == *pos1 - 2 && *status == 2)
            fprintf(tempFile, "\t%s", stack->top->element);
        else if (ftell(tempFile) == *pos1) {
            NodeStack* currentNode = stack->top;
            NodeStack* prevNode = NULL;
            while (currentNode != NULL) {
                NodeStack* nextNode = currentNode->next;
                currentNode->next = prevNode;
                prevNode = currentNode;
                currentNode = nextNode;
            }
            currentNode = prevNode;
            while (currentNode != NULL) {
                if (currentNode->next == NULL)
                    fprintf(tempFile, "%s\n", currentNode->element);
                else
                    fprintf(tempFile, "%s\t", currentNode->element);
                currentNode = currentNode->next;
            }
            if (*status == 1) {
                fseek(tempFile, *pos1 - 1, SEEK_SET);
                fprintf(tempFile, "\n");
            }
            fseek(file, *pos2, SEEK_SET);
        }
    }
    free(stack->top);
    free(stack);
    fclose(file);
    fclose(tempFile);
    remove(filename);
    rename("temp.data", filename);
}

Stack* loadFromFileStack(const char* filename, const char* basename, int* pos1, int* pos2, int* status) {
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        return NULL;
    }
    int num_lines = countWordsInFileStack(filename);
    char** line = malloc(num_lines * sizeof(char*));
    for (int i = 0; i < num_lines; i++) line[i] = malloc(10000 * sizeof(char));
    Stack* stack = initStack();
    int tempory = 0;
    int pos3 = 0;
    int temp1 = 0;
    int temp2 = 0;
    char c = '1';
    for (int i = 0; i < num_lines; ++i) {
        fscanf(file, "%s", line[i]);
        c = getc(file);
        pos3 = ftell(file);
        if (!strcmp(line[i], basename)) {
            fseek(file,  -3 - strlen(line[i]), SEEK_CUR);
            if (getc(file) == '\n' || i == 0) {
                fseek(file, pos3, SEEK_SET);
                tempory = 1;
                *pos1 = ftell(file);
                temp1 = i + 1;
            }
            else fseek(file, pos3, SEEK_SET);
        }
        if (c == '\n' && tempory == 1) {
            temp2 = i;
            *pos2 = ftell(file);
            tempory = 0;
        }
        if (feof(file))
            break;
    }
    if (temp1 == temp2)
        *status = 1;
    if (temp1 == temp2 + 1)
        *status = 2;
    while (temp1 < temp2 + 1) {
        SPUSH(stack, line[temp1]);
        temp1++;
    }
    fclose(file);
    for (int i = 0; i < num_lines; i++) {
        free(line[i]);
    }
    free(line);
    return stack;
}