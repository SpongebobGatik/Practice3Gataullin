#define _CRT_SECURE_NO_WARNINGS
#include "table.h" 
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#define MAX_SIZE 100000

int countWordsInFileTable(const char* filename) {
	FILE* file = fopen(filename, "r");
	int count = 0;
	char word[10000];
	while (fscanf(file, "%s", word) != EOF) {
		count++;
	}
	fclose(file);
	return count;
}

HashTable* initHashTable() {
	HashTable* ht = (HashTable*)malloc(sizeof(HashTable));
	ht->head = NULL;
	ht->size = 0;
	ht->tableSize = MAX_SIZE;
	ht->hashTable = (NodeHashTable**)malloc(MAX_SIZE * sizeof(NodeHashTable*));
	ht->keys = (char**)malloc(MAX_SIZE * sizeof(char*));
	for (int i = 0; i < MAX_SIZE; i++) {
		ht->hashTable[i] = NULL;
		ht->keys[i] = NULL;
	}
	return ht;
}

int calculateHashT(const char* element) {
	int hash = 0;
	for (int i = 0; element[i] != '\0'; i++) {
		hash = 31 * hash + element[i];
	}
	return abs(hash) % MAX_SIZE;
}

void HSET(HashTable* ht, char* key, char* value) {
	int hash = calculateHashT(key);
	NodeHashTable* current = ht->hashTable[hash];
	while (current != NULL) {
		if (strcmp(current->element, value) == 0) {
			return;
		}
		current = current->next;
	}
	NodeHashTable* newNode = (NodeHashTable*)malloc(sizeof(NodeHashTable));
	newNode->element = _strdup(value);
	newNode->hash = hash;
	newNode->next = NULL;
	newNode->prev = NULL;
	if (ht->hashTable[hash] == NULL) {
		ht->hashTable[hash] = newNode;
	}
	else {
		current = ht->hashTable[hash];
		while (current->next != NULL) {
			current = current->next;
		}
		current->next = newNode;
		newNode->prev = current;
	}
	ht->size++;
	ht->keys[ht->size - 1] = _strdup(key);
}

char* HGET(HashTable* ht, const char* key) {
	if (ht->hashTable[calculateHashT(key)] != NULL) {
		return ht->hashTable[calculateHashT(key)]->element;
	}
	return NULL;
}

void HDEL(HashTable* ht, const char* key) {
	if (ht->hashTable[calculateHashT(key)] != NULL) {
		NodeHashTable* nodeToRemove = ht->hashTable[calculateHashT(key)];
		if (nodeToRemove == ht->head) {
			ht->head = nodeToRemove->next;
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
		ht->hashTable[calculateHashT(key)] = NULL;
		ht->size--;
		return;
	}
}


void saveToFileTable(HashTable* hashtable, const char* filename, const char* basename, int* pos1, int* pos2, int* status) {
	FILE* file = fopen(filename, "r");
	FILE* tempFile = fopen("temp.data", "w");
	int ch;
	fseek(file, 0, SEEK_SET);
	fseek(tempFile, 0, SEEK_SET);
	while ((ch = fgetc(file)) != EOF) {
		fputc(ch, tempFile);
		if (ftell(tempFile) == *pos1 - 2 && *status == 2) {
			fprintf(tempFile, "\t%s\t%s", hashtable->hashTable[calculateHashT(hashtable->keys[0])]->element, hashtable->keys[0]);
		}
		else if (ftell(tempFile) == *pos1) {
			for (int i = 0; i < hashtable->size; i++) {
				if (i == hashtable->size - 1) {
					fprintf(tempFile, "%s\t%s\n", hashtable->hashTable[calculateHashT(hashtable->keys[i])]->element, hashtable->keys[i]);
				}
				else {
					fprintf(tempFile, "%s\t%s\t", hashtable->hashTable[calculateHashT(hashtable->keys[i])]->element, hashtable->keys[i]);
				}
			}
			if (*status == 1) {
				fseek(tempFile, *pos1 - 1, SEEK_SET);
				fprintf(tempFile, "\n");
			}
			fseek(file, *pos2, SEEK_SET);
		}
	}
	free(hashtable->hashTable);
	free(hashtable->keys);
	free(hashtable);
	fclose(file);
	fclose(tempFile);
	remove(filename);
	rename("temp.data", filename);
}

HashTable* loadFromFileTable(const char* filename, const char* basename, int* pos1, int* pos2, int* status) {
	FILE* file = fopen(filename, "r");
	int num_lines = countWordsInFileTable(filename);
	char** line = malloc(num_lines * sizeof(char*));
	for (int i = 0; i < num_lines; i++) line[i] = malloc(10000 * sizeof(char));
	HashTable* hashtable = initHashTable();
	int tempory = 0;
	int tempory2 = 0;
	int count = 0;
	int temp1 = 0;
	int temp2 = 0;
	char c = '1';
	for (int i = 0; i < num_lines; ++i) {
		fscanf(file, "%s", line[i]);
		c = getc(file);
		if (c == '\n') {
			tempory2 = ftell(file);
		}
		if (!strcmp(line[i], basename) && (tempory2 == ftell(file) || tempory2 == ftell(file) - strlen(line[i]) - 1 || i == 0)) {
			tempory = 1;
			*pos1 = ftell(file);
			*pos2 = strlen(line[i]);
			temp1 = i + 1;
		}
		if (c == '\n' && tempory == 1) {
			temp2 = i;
			*pos2 = ftell(file);
			tempory = 0;
			count++;
		}
		if (feof(file))
			break;
	}
	if (temp1 + 1 == temp2) *status = 1;
	if (temp1 == temp2 + 1) *status = 2;
	while (temp1 < temp2) {
		char* value = line[temp1];
		char* key = line[temp1 + 1];
		HSET(hashtable, key, value);
		temp1 += 2;
	}
	fclose(file);
	for (int i = 0; i < num_lines; i++) {
		free(line[i]);
	}
	free(line);
	return hashtable;
}