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
	ht->nodes = (NodeHashTable**)malloc(MAX_SIZE * sizeof(NodeHashTable*));
	ht->count = 0;
	for (int i = 0; i < MAX_SIZE; i++) {
		ht->nodes[i] = NULL;
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
	int hash = calculateHashT(key);
	// Создаем новый узел для хранения ключа и значения
	NodeHashTable* newNode = (NodeHashTable*)malloc(sizeof(NodeHashTable));
	newNode->key = _strdup(key);
	newNode->element = _strdup(value);
	newNode->next = NULL; // Устанавливаем указатель на следующий узел как NULL
	newNode->prev = NULL; // Устанавливаем указатель на предыдущий узел как NULL
	// Обработка коллизий и проверка на дубликаты ключей
	NodeHashTable* current = ht->nodes[hash]; // Получаем узел по хеш-ключу
	while (current != NULL) {
		if (strcmp(current->key, key) == 0) { // Если ключ уже существует
			// Освобождаем память нового узла
			free(newNode->key);
			free(newNode->element);
			free(newNode);
			return;
		}
		if (current->next == NULL) { // Если достигли конца цепочки
			break;
		}
		current = current->next; // Переходим к следующему узлу
	}
	// Добавление нового узла
	if (current == NULL) { // Если цепочка пуста
		ht->nodes[hash] = newNode; // Устанавливаем новый узел как начало цепочки
	}
	else {
		current->next = newNode; // Добавляем новый узел в конец цепочки
		newNode->prev = current; // Устанавливаем предыдущий узел для нового узла
	}
	ht->count++;
}

// Функция для получения элемента из хеш-таблицы
char* HGET(HashTable* ht, const char* key) {
	int hash = calculateHashT(key);
	NodeHashTable* current = ht->nodes[hash]; // Получаем узел по хеш-ключу
	while (current != NULL) { // Перебираем узлы в цепочке
		if (strcmp(current->key, key) == 0) { // Если ключ совпадает
			return current->element;
		}
		current = current->next; // Переходим к следующему узлу
	}
	return NULL;
}

// Функция для удаления элемента из хеш-таблицы
void HDEL(HashTable* ht, const char* key) {
	int hash = calculateHashT(key);
	NodeHashTable* current = ht->nodes[hash]; // Получаем узел по хеш-ключу
	NodeHashTable* nodeToRemove = NULL;

	while (current != NULL) { // Перебираем узлы в цепочке
		if (strcmp(current->key, key) == 0) { // Если ключ совпадает
			nodeToRemove = current; // Устанавливаем узел для удаления
			break;
		}
		current = current->next; // Переходим к следующему узлу
	}

	if (nodeToRemove != NULL) {
		if (nodeToRemove->prev != NULL) { // Если у узла есть предыдущий узел
			nodeToRemove->prev->next = nodeToRemove->next; // Удаляем узел из цепочки
		}
		else {
			ht->nodes[hash] = nodeToRemove->next; // Устанавливаем следующий узел как начало цепочки
		}
		if (nodeToRemove->next != NULL) { // Если у узла есть следующий узел
			nodeToRemove->next->prev = nodeToRemove->prev; // Устанавливаем предыдущий узел для следующего узла
		}
		// Освобождаем память удаляемого узла
		free(nodeToRemove->key);
		free(nodeToRemove->element);
		free(nodeToRemove);
		ht->count--;
	}
}

void saveToFileTable(HashTable* hashtable, const char* filename, const char* basename, int* pos1, int* pos2, int* status) {
	FILE* file = fopen(filename, "r");
	FILE* tempFile = fopen("temp.data", "w");
	int ch;
	int count = 0;
	fseek(file, 0, SEEK_SET);
	fseek(tempFile, 0, SEEK_SET);
	while ((ch = fgetc(file)) != EOF) {
		fputc(ch, tempFile);
		if (ftell(tempFile) == *pos1 || (ftell(tempFile) == *pos1 - 2 && *status == 2)) {
			// Если пустая БД, то добавляем табуляцию перед элементом с ключом
			if (hashtable->count == 1 && *status == 2) {
				fprintf(tempFile, "\t");
			}
			// Проходимся по всем элементам
			for (int i = 0; i < MAX_SIZE; i++) {
				NodeHashTable* currentNode = hashtable->nodes[i];
				while (currentNode != NULL) {
					fprintf(tempFile, "%s\t%s", currentNode->element, currentNode->key);
					count++;
					if (count != hashtable->count) {
						fprintf(tempFile, "\t"); // Добавляем табуляцию после элемента с ключом
					}
					else {
						fprintf(tempFile, "\n"); // Если нет следующего элемента с ключом, то добавляем строку
						break;
					}
					currentNode = currentNode->next;
				}
			}
			// Добавление новую строку, если БД пуста
			if (*status == 1) {
				fseek(tempFile, *pos1 - 1, SEEK_SET);
				fprintf(tempFile, "\n");
			}
			fseek(file, *pos2, SEEK_SET);
		}
	}
	fclose(file);
	fclose(tempFile);
	remove(filename);
	rename("temp.data", filename);
}

// Функция загрузки БД в структуру
HashTable* loadFromFileTable(const char* filename, const char* basename, int* pos1, int* pos2, int* status) {
	FILE* file = fopen(filename, "r");
	// Проверка наличия файла
	if (file == NULL) {
		return NULL;
	}
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