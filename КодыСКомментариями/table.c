#define _CRT_SECURE_NO_WARNINGS
#include "table.h" 
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#define MAX_SIZE 100000

// Функция подсчета слов в файле
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

// Функция для инициализации хеш-таблицы
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

// Функция для вычисления хеша
int calculateHashT(const char* element) {
	int hash = 0;
	for (int i = 0; element[i] != '\0'; i++) {
		hash = 31 * hash + element[i];
	}
	return abs(hash) % MAX_SIZE;
}

// Функция для добавления элемента в хеш-таблицу
void HSET(HashTable* ht, char* key, char* value) {
	int hash = calculateHashT(key); // Вычисление хеша ключа
	NodeHashTable* current = ht->hashTable[hash];
	// Проверка уникальности ключа
	while (current != NULL) {
		if (strcmp(current->element, value) == 0) {
			return;
		}
		current = current->next;
	}
	// Новый узел
	NodeHashTable* newNode = (NodeHashTable*)malloc(sizeof(NodeHashTable));
	newNode->element = _strdup(value); // Копирование элемента в узел
	newNode->hash = hash; // Сохранение хеша в узле
	newNode->next = NULL; // Инициализация следующего узла как NULL
	newNode->prev = NULL; // Инициализация предыдущего узла как NULL
	if (ht->hashTable[hash] == NULL) { // Если список для данного хеша пуст
		ht->hashTable[hash] = newNode; // Установка нового узла как первого в списке
	}
	else {
		current = ht->hashTable[hash]; // Начало списка
		while (current->next != NULL) { // Поиск последнего узла в списке
			current = current->next;
		}
		current->next = newNode; // Добавление нового узла в конец списка
		newNode->prev = current; // Установка предыдущего узла для нового узла
	}
	ht->size++; // Увеличение размера хеш-таблицы
	ht->keys[ht->size - 1] = _strdup(key); // Сохранение ключа в массиве ключей
}

// Функция для получения элемента из хеш-таблицы
char* HGET(HashTable* ht, const char* key) {
	// Если найден элемент
	if (ht->hashTable[calculateHashT(key)] != NULL) {
		return ht->hashTable[calculateHashT(key)]->element;
	}
	return NULL;
}

// Функция для удаления элемента из хеш-таблицы
void HDEL(HashTable* ht, const char* key) {
	// Если найден элемент
	if (ht->hashTable[calculateHashT(key)] != NULL) {
		NodeHashTable* nodeToRemove = ht->hashTable[calculateHashT(key)]; // Узел для удаления
		if (nodeToRemove == ht->head) { // Если узел является головой списка
			ht->head = nodeToRemove->next; // Установка следующего узла как головы
		}
		else {
			if (nodeToRemove->prev != NULL) { // Если у узла есть предыдущий узел
				nodeToRemove->prev->next = nodeToRemove->next; // Удаление узла из списка
			}
		}
		if (nodeToRemove->next != NULL) { // Если у узла есть следующий узел
			nodeToRemove->next->prev = nodeToRemove->prev; // Удаление узла из списка
		}
		free(nodeToRemove->element);
		free(nodeToRemove);
		ht->hashTable[calculateHashT(key)] = NULL;
		ht->size--;
		return;
	}
}

// Функция сохранения изменений в файл
void saveToFileTable(HashTable* hashtable, const char* filename, const char* basename, int* pos1, int* pos2, int* status) {
	FILE* file = fopen(filename, "r");
	FILE* tempFile = fopen("temp.data", "w");
	int ch; // Символ, который требуется записать
	fseek(file, 0, SEEK_SET); // Установка курсора в начало
	fseek(tempFile, 0, SEEK_SET); // Установка курсора в начало
	while ((ch = fgetc(file)) != EOF) { // Цикл, пока не выдаст ошибку при записи
		fputc(ch, tempFile); // Запись символа в поток
		if (ftell(tempFile) == *pos1 - 2 && *status == 2) { // Если пустая БД, то добавляем табуляцию перед элементом с ключом
			fprintf(tempFile, "\t%s\t%s", hashtable->hashTable[calculateHashT(hashtable->keys[0])]->element, hashtable->keys[0]);
		}
		else if (ftell(tempFile) == *pos1) {
			// Проходимся по всем элементам
			for (int i = 0; i < hashtable->size; i++) {
				if (i == hashtable->size - 1) { // Если нет следующего элемента с ключом, то добавляем строку
					fprintf(tempFile, "%s\t%s\n", hashtable->hashTable[calculateHashT(hashtable->keys[i])]->element, hashtable->keys[i]);
				}
				else { // Иначе добавляем после элемента с ключом табуляцию
					fprintf(tempFile, "%s\t%s\t", hashtable->hashTable[calculateHashT(hashtable->keys[i])]->element, hashtable->keys[i]);
				}
			}
			// Добавление новую строку, если БД пуста
			if (*status == 1) {
				fseek(tempFile, *pos1 - 1, SEEK_SET);
				printf("asd");
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

// Функция загрузки БД в структуру
HashTable* loadFromFileTable(const char* filename, const char* basename, int* pos1, int* pos2, int* status) {
	FILE* file = fopen(filename, "r");
	int num_lines = countWordsInFileTable(filename); // Переменная, отвечающая за количество слов
	char** line = malloc(num_lines * sizeof(char*));
	for (int i = 0; i < num_lines; i++) line[i] = malloc(10000 * sizeof(char));
	HashTable* hashtable = initHashTable();
	int tempory = 0; // Переменная-переключатель
	int tempory2 = 0; // Переменная, временно хранящие положение символа
	int temp1 = 0; // Переменная, отвечающая за номер 1 элемента в БД
	int temp2 = 0; // Переменная, отвечающая за номер 2 элемента в БД
	char c = '1'; // Полученный символ
	// Проходимся по всем словам
	for (int i = 0; i < num_lines; ++i) {
		fscanf(file, "%s", line[i]);
		c = getc(file);
		if (c == '\n') {
			tempory2 = ftell(file);
		}
		if (!strcmp(line[i], basename) && (tempory2 == ftell(file) || tempory2 == ftell(file) - strlen(line[i]) - 1 || i == 0)) { // Если наша БД, то делаем с ней операции
			tempory = 1;
			*pos1 = ftell(file);
			*pos2 = strlen(line[i]);
			temp1 = i + 1;
		}
		if (c == '\n' && tempory == 1) { // Конец нашей БД
			temp2 = i;
			*pos2 = ftell(file);
			tempory = 0;
		}
		if (feof(file))
			break;
	}
	if (temp1 + 1 == temp2) *status = 1; // 1 элемент
	if (temp1 == temp2 + 1) *status = 2; // 2 или более элемента
	// Добавление в структуру элементов
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