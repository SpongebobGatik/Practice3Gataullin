#define _CRT_SECURE_NO_WARNINGS
#include "queue.h" 
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 

int countWordsInFileQueue(const char* filename) {
    FILE* file = fopen(filename, "r");
    int count = 0;
    char word[10000];
    while (fscanf(file, "%s", word) != EOF) {
        count++;
    }
    fclose(file);
    return count;
}

Queue* initQueue() {
    Queue* queue = (Queue*)malloc(sizeof(Queue));
    queue->front = NULL;
    queue->rear = NULL;
    return queue;
}

void QPUSH(Queue* queue, char* element) {
    NodeQueue* newNode = (NodeQueue*)malloc(sizeof(NodeQueue));
    newNode->element = _strdup(element);
    newNode->next = NULL;
    if (queue->front == NULL) {
        queue->front = newNode;
        queue->rear = newNode;
    }
    else {
        queue->rear->next = newNode;
        queue->rear = newNode;
    }
}

char* QPOP(Queue* queue) {
    if (queue->front == NULL) {
        return ("The queue is empty");
    }
    NodeQueue* poppedNode = queue->front;
    char* element = poppedNode->element;
    queue->front = poppedNode->next;
    if (queue->front == NULL) {
        queue->rear = NULL;
    }
    free(poppedNode);
    return element;
}

void saveToFileQueue(Queue* queue, const char* filename, const char* basename, int* pos1, int* pos2, int* status) {
    FILE* file = fopen(filename, "r");
    FILE* tempFile = fopen("temp.data", "w");
    int ch;
    fseek(file, 0, SEEK_SET);
    fseek(tempFile, 0, SEEK_SET);
    while ((ch = fgetc(file)) != EOF) {
        fputc(ch, tempFile);
        if (ftell(tempFile) == *pos1 - 2 && *status == 2)
            fprintf(tempFile, "\t%s", queue->front->element);
        else if (ftell(tempFile) == *pos1) {
            NodeQueue* currentNode = queue->front;
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
    free(queue->rear);
    free(queue->front);
    free(queue);
    fclose(file);
    fclose(tempFile);
    remove(filename);
    rename("temp.data", filename);
}

Queue* loadFromFileQueue(const char* filename, const char* basename, int* pos1, int* pos2, int* status) {
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        return NULL;
    }
    int num_lines = countWordsInFileQueue(filename);
    char** line = malloc(num_lines * sizeof(char*));
    for (int i = 0; i < num_lines; i++) line[i] = malloc(10000 * sizeof(char));
    Queue* queue = initQueue();
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
            fseek(file, -3 - strlen(line[i]), SEEK_CUR);
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
        QPUSH(queue, line[temp1]);
        temp1++;
    }
    fclose(file);
    for (int i = 0; i < num_lines; i++) {
        free(line[i]);
    }
    free(line);
    return queue;
}