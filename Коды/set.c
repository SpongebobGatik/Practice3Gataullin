#define _CRT_SECURE_NO_WARNINGS
#include "set.h" 
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#define MAX_SIZE 100000 

// Функция подсчета слов в файле
int countWordsInFileSet(const char* filename) {
    FILE* file = fopen(filename, "r");
    int count = 0;  // Счетчик слов
    char word[10000];  // Массив для хранения слов
    while (fscanf(file, "%s", word) != EOF) {
        count++;
    }
    fclose(file);
    return count;
}

// Функция для инициализации множества
Set* initSet() {
    Set* set = (Set*)malloc(sizeof(Set));
    set->head = NULL; // Инициализация головы множества
    set->size = 0; // Инициализация размера множества
    set->tableSize = MAX_SIZE; // Инициализация размера хеш-таблицы
    set->hashTable = (Node**)malloc(MAX_SIZE * sizeof(Node*));
    set->emptySlots = (int*)malloc(MAX_SIZE * sizeof(int));
    for (int i = 0; i < MAX_SIZE; i++) {
        set->hashTable[i] = NULL; // Инициализация элемента хеш-таблицы
        set->emptySlots[i] = 1; // Инициализация элемента массива пустых слотов
    }
    return set; // Возврат указателя на множество
}

// Функция для вычисления первого хеша
int calculateHashS(const char* element) {
    int hash = 0;
    for (int i = 0; element[i] != '\0'; i++) {
        hash = 31 * hash + element[i];
    }
    return abs(hash) % MAX_SIZE;
}

// Функция для вычисления второго хеша
int calculateHash2S(const char* element) {
    int hash = 0;
    for (int i = 0; element[i] != '\0'; i++) {
        hash = 17 * hash + element[i];
    }
    return abs(hash) % MAX_SIZE;
}

// Функция для добавления элемента в множество
void SADD(Set* set, char* element) {
    int hash = calculateHashS(element); // Вычисление первого хеша элемента
    int step = calculateHash2S(element); // Вычисление второго хеша элемента
    // Объединение хешов
    while (set->hashTable[hash] != NULL && strcmp(set->hashTable[hash]->element, element) != 0) {
        hash = (hash + step) % MAX_SIZE;
    }
    // Проверка повтора элемента
    if (set->hashTable[hash] != NULL && strcmp(set->hashTable[hash]->element, element) == 0) {
        return;
    }
    // Новый узел
    Node* newNode = (Node*)malloc(sizeof(Node));
    newNode->element = _strdup(element); // Копирование элемента
    newNode->hash = hash; // Сохранение хеша элемента
    newNode->next = set->head; // Установка следующего узла после нового узла
    if (set->head != NULL) { // Если голова множества не пуста
        set->head->prev = newNode; // Установка предыдущего узла
    }
    set->head = newNode; // Обновление головы списка
    set->hashTable[hash] = newNode; // Вставка узла в хеш-таблицу
    set->size++; // Увеличение размера множества
}

// Функция для удаления элемента из множества
void SREM(Set* set, const char* element) {
    int hash = calculateHashS(element); // Вычисление первого хеша элемента
    int step = calculateHash2S(element); // Вычисление второго хеша элемента
    // Объединение хешов
    while (set->hashTable[hash] != NULL && strcmp(set->hashTable[hash]->element, element) != 0) {
        hash = (hash + step) % MAX_SIZE;
    }
    // Если найден элемент
    if (set->hashTable[hash] != NULL && strcmp(set->hashTable[hash]->element, element) == 0) {
        Node* nodeToRemove = set->hashTable[hash]; // Получение узла для удаления
        if (nodeToRemove == set->head) { // Если узел для удаления является головой множества
            set->head = nodeToRemove->next; // Установка следующего узла в качестве головы множества
        }
        else {
            if (nodeToRemove->prev != NULL) { // Если у узла для удаления есть предыдущий узел
                nodeToRemove->prev->next = nodeToRemove->next; // Установка следующего узла после предыдущего узла
            }
        }
        if (nodeToRemove->next != NULL) { // Если у узла для удаления есть следующий узел
            nodeToRemove->next->prev = nodeToRemove->prev; // Установка предыдущего узла перед следующим узлом
        }
        free(nodeToRemove->element); // Освобождение памяти, занятой элементом
        free(nodeToRemove); // Освобождение памяти, занятой узлом
        set->hashTable[hash] = NULL; // Удаление узла из хеш-таблицы
        set->size--; // Уменьшение размера множества на 1
        return;
    }
}

// Функция для проверки существования элемента
int SISMEMBER(Set* set, const char* element) {
    int hash = calculateHashS(element); // Вычисление первого хеша элемента
    int step = calculateHash2S(element); // Вычисление второго хеша элемента
    // Объединение хешов
    while (set->hashTable[hash] != NULL && strcmp(set->hashTable[hash]->element, element) != 0) {
        hash = (hash + step) % MAX_SIZE;
    }
    // Если найден элемент
    if (set->hashTable[hash] != NULL && strcmp(set->hashTable[hash]->element, element) == 0) {
        return 1;
    }
    return 0;
}

// Функция сохранения изменений в файл
void saveToFileSet(Set* set, const char* filename, const char* basename, int* pos1, int* pos2, int* status) {
    FILE* file = fopen(filename, "r");
    FILE* tempFile = fopen("temp.data", "w");
    int ch; // Символ, который требуется записать
    fseek(file, 0, SEEK_SET); // Установка курсора в начало
    fseek(tempFile, 0, SEEK_SET); // Установка курсора в начало
    char** elements = (char**)malloc(set->size * sizeof(char*));
    Node* current = set->head; // Ставим указатель на голову
    int i = 0; // Переменная, отвечающая за порядок
    // Добавление элементов в структуру
    while (current != NULL) {
        elements[i] = current->element;
        current = current->next;
        i++;
    }
    while ((ch = fgetc(file)) != EOF) {  // Цикл, пока не выдаст ошибку при записи
        fputc(ch, tempFile); // Запись символа в поток
        if (ftell(tempFile) == *pos1 - 2 && *status == 2) // Если пустая БД, то добавляем табуляцию перед элементом
            fprintf(tempFile, "\t%s", set->head->element);
        else if (ftell(tempFile) == *pos1) {
            // Пока не закончатся добавляем элементы
            for (int j = set->size - 1; j >= 0; j--) {
                fprintf(tempFile, "%s", elements[j]);
                if (j > 0) // Если есть следующий элемент, то добавляем табуляцию после элемента
                    fprintf(tempFile, "\t");
                else // Иначе добавляем новую строку
                    fprintf(tempFile, "\n");
            }
            // Добавление новую строку, если БД пуста
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
    remove(filename);
    rename("temp.data", filename);
}

// Функция загрузки БД в структуру
Set* loadFromFileSet(const char* filename, const char* basename, int* pos1, int* pos2, int* status) {
    FILE* file = fopen(filename, "r");
    // Проверка наличия файла
    if (file == NULL) {
        return NULL;
    }
    int num_lines = countWordsInFileSet(filename); // Переменная, отвечающая за количество слов
    char** line = malloc(num_lines * sizeof(char*));
    for (int i = 0; i < num_lines; i++) line[i] = malloc(10000 * sizeof(char));
    Set* set = initSet();
    int tempory = 0; // Переменная-переключатель
    int pos3 = 0; // Временная переменная, которая запоминает исходную позицию курсора
    int temp1 = 0; // Переменная, отвечающая за номер 1 элемента в БД
    int temp2 = 0; // Переменная, отвечающая за номер 2 элемента в БД
    char c = '1'; // Полученный символ
    // Проходимся по всем словам
    for (int i = 0; i < num_lines; ++i) {
        fscanf(file, "%s", line[i]);
        c = getc(file);
        pos3 = ftell(file);
        if (!strcmp(line[i], basename)) { // Если наша БД, то делаем с ней операции
            fseek(file, -3 - strlen(line[i]), SEEK_CUR); // Временно переносим курсор на предыдущии позиции
            if (getc(file) == '\n' || i == 0) {
                fseek(file, pos3, SEEK_SET); // Возврат курсора
                tempory = 1;
                *pos1 = ftell(file);
                temp1 = i + 1;
            }
            else fseek(file, pos3, SEEK_SET); // Возврат курсора
        }
        if (c == '\n' && tempory == 1) { // Конец нашей БД
            temp2 = i;
            *pos2 = ftell(file);
            tempory = 0;
        }
        if (feof(file))
            break;
    }
    if (temp1 == temp2)
        *status = 1; // 1 элемент
    if (temp1 == temp2 + 1)
        *status = 2; // 2 или более элемента
    // Добавление в структуру элементов
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