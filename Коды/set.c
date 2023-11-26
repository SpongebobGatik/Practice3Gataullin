#define _CRT_SECURE_NO_WARNINGS
#include "set.h" 
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#define MAX_SIZE 100000 

int countWordsInFileSet(const char* filename) {
    FILE* file = fopen(filename, "r");
    int count = 0;
    char word[10000];
    while (fscanf(file, "%s", word) != EOF) {
        count++;
    }
    fclose(file);
    return count;
}

Set* initSet() {
    Set* set = (Set*)malloc(sizeof(Set));
    set->head = NULL;
    set->size = 0;
    set->tableSize = MAX_SIZE;
    set->hashTable = (Node**)malloc(MAX_SIZE * sizeof(Node*));
    set->emptySlots = (int*)malloc(MAX_SIZE * sizeof(int));
    for (int i = 0; i < MAX_SIZE; i++) {
        set->hashTable[i] = NULL;
        set->emptySlots[i] = 1;
    }
    return set;
}

int calculateHashS(const char* element) {
    int hash = 0;
    for (int i = 0; element[i] != '\0'; i++) {
        hash = 31 * hash + element[i];
    }
    return abs(hash) % MAX_SIZE;
}

void SADD(Set* set, char* element) {
    int hash = calculateHashS(element);
    if (set->hashTable[hash] != NULL) {
        return;
    }
    Node* newNode = (Node*)malloc(sizeof(Node));
    newNode->element = _strdup(element);
    newNode->hash = hash;
    newNode->next = set->head;
    if (set->head != NULL) {
        set->head->prev = newNode;
    }
    set->head = newNode;
    set->hashTable[hash] = newNode;
    set->size++;
}

void SREM(Set* set, const char* element) {
    int hash = calculateHashS(element);
    if (set->hashTable[hash] != NULL && strcmp(set->hashTable[hash]->element, element) == 0) {
        Node* nodeToRemove = set->hashTable[hash];
        if (nodeToRemove == set->head) {
            set->head = nodeToRemove->next;
        }
        else {
            if (nodeToRemove->prev != NULL) {
                nodeToRemove->prev->next = nodeToRemove->next;
            }
        }
        if (nodeToRemove->next != NULL) {
            nodeToRemove->next->prev = nodeToRemove->prev;
        }
        free(nodeToRemove->element);
        free(nodeToRemove);
        set->hashTable[hash] = NULL;
        set->size--;
        return;
    }
}

int SISMEMBER(Set* set, const char* element) {
    int hash = calculateHashS(element);
    if (set->hashTable[hash] != NULL && strcmp(set->hashTable[hash]->element, element) == 0) {
        return 1;
    }
    return 0;
}

void saveToFileSet(Set* set, const char* filename, const char* basename, int* pos1, int* pos2, int* status) {
    FILE* file = fopen(filename, "r");
    FILE* tempFile = fopen("temp.data", "w");
    int ch;
    fseek(file, 0, SEEK_SET);
    fseek(tempFile, 0, SEEK_SET);
    char** elements = (char**)malloc(set->size * sizeof(char*));
    Node* current = set->head;
    int i = 0;
    while (current != NULL) {
        elements[i] = current->element;
        current = current->next;
        i++;
    }
    while ((ch = fgetc(file)) != EOF) {
        fputc(ch, tempFile);
        if (ftell(tempFile) == *pos1 - 2 && *status == 2)
            fprintf(tempFile, "\t%s", set->head->element);
        else if (ftell(tempFile) == *pos1) {
            for (int j = set->size - 1; j >= 0; j--) {
                fprintf(tempFile, "%s", elements[j]);
                if (j > 0)
                    fprintf(tempFile, "\t");
                else
                    fprintf(tempFile, "\n");
            }
            if (*status == 1) {
                fseek(tempFile, *pos1 - 1, SEEK_SET);
                fprintf(tempFile, "\n");
            }
            fseek(file, *pos2, SEEK_SET);
        }
    }
    free(elements);
    free(set->hashTable);
    free(set->emptySlots);
    free(set);
    fclose(file);
    fclose(tempFile);
    remove("2.data");
    rename("temp.data", "2.data");
}

Set* loadFromFileSet(const char* filename, const char* basename, int* pos1, int* pos2, int* status) {
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        return NULL;
    }
    int num_lines = countWordsInFileSet(filename);
    char** line = malloc(num_lines * sizeof(char*));
    for (int i = 0; i < num_lines; i++) line[i] = malloc(10000 * sizeof(char));
    Set* set = initSet();
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
        SADD(set, line[temp1]);
        temp1++;
    }
    fclose(file);
    for (int i = 0; i < num_lines; i++) {
        free(line[i]);
    }
    free(line);
    return set;
}