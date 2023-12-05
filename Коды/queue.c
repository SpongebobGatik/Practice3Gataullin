#define _CRT_SECURE_NO_WARNINGS
#include "queue.h" 
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 

// Функция подсчета слов в файле
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

// Функция инициализации очереди
Queue* initQueue() {
    Queue* queue = (Queue*)malloc(sizeof(Queue));
    queue->front = NULL;  // Установка указателя на начало очереди в NULL
    queue->rear = NULL;  // Установка указателя на конец очереди в NULL
    return queue;   // Возврат указателя на очередь
}

// Функция добавления элемента в очередь
void QPUSH(Queue* queue, char* element) {
    // Новый узел
    NodeQueue* newNode = (NodeQueue*)malloc(sizeof(NodeQueue));
    newNode->element = _strdup(element);  // Копирование элемента в узел
    newNode->next = NULL;  // Установка указателя на следующий узел в NULL
    if (queue->front == NULL) {  // Если очередь пуста
        queue->front = newNode;  // Установка указателя на начало очереди на новый узел
        queue->rear = newNode;  // Установка указателя на конец очереди на новый узел
    }
    else {  // Если очередь не пуста
        queue->rear->next = newNode;  // Добавление нового узла в конец очереди
        queue->rear = newNode;  // Установка указателя на конец очереди на новый узел
    }
}

// Функция удаления элемента из очереди
char* QPOP(Queue* queue) {
    // Проверка пустоты очереди
    if (queue->front == NULL) {
        return ("The queue is empty");
    }
    NodeQueue* poppedNode = queue->front;  // Указатель на удаляемый узел
    char* element = poppedNode->element;  // Указатель на удаляемый элемент
    queue->front = poppedNode->next;   // Сдвиг указателя на начало очереди на следующий узел
    if (queue->front == NULL) {  // Если очередь стала пустой
        queue->rear = NULL;  // Установка указателя на конец очереди в NULL
    }
    free(poppedNode);  // Освобождение памяти от удаляемого узла
    return element;  // Возврат указателя на удаляемый элемент
}

// Функция сохранения изменений в файл
void saveToFileQueue(Queue* queue, const char* filename, const char* basename, int* pos1, int* pos2, int* status) {
    FILE* file = fopen(filename, "r");
    FILE* tempFile = fopen("temp.data", "w");
    int ch; // Символ, который требуется записать
    fseek(file, 0, SEEK_SET); // Установка курсора в начало
    fseek(tempFile, 0, SEEK_SET); // Установка курсора в начало
    while ((ch = fgetc(file)) != EOF) { // Цикл, пока не выдаст ошибку при записи
        fputc(ch, tempFile); // Запись символа в поток
        if (ftell(tempFile) == *pos1 - 2 && *status == 2)  // Если пустая БД, то добавляем табуляцию перед элементом
            fprintf(tempFile, "\t%s", queue->front->element);
        else if (ftell(tempFile) == *pos1) {
            NodeQueue* currentNode = queue->front; // Ставим указатель на начало
            // Пока не закончатся добавляем элементы
            while (currentNode != NULL) {
                if (currentNode->next == NULL) // Если нет следующего элемента, то добавляем строку
                    fprintf(tempFile, "%s\n", currentNode->element);
                else // Иначе добавляем после элемента табуляцию
                    fprintf(tempFile, "%s\t", currentNode->element);
                currentNode = currentNode->next; // Переход к следующему элементу
            }
            // Добавление новую строку, если БД пуста
            if (*status == 1) {
                fseek(tempFile, *pos1 - 1, SEEK_SET);
                fprintf(tempFile, "\n");
            }
            fseek(file, *pos2, SEEK_SET);
        }
    }
    NodeQueue* currentNode = queue->front;
    while (currentNode != NULL) {
        NodeQueue* nextNode = currentNode->next;
        free(currentNode->element);
        free(currentNode);
        currentNode = nextNode;
    }
    free(queue);
    fclose(file);
    fclose(tempFile);
    remove(filename);
    rename("temp.data", filename);
}

// Функция загрузки БД в структуру
Queue* loadFromFileQueue(const char* filename, const char* basename, int* pos1, int* pos2, int* status) {
    FILE* file = fopen(filename, "r");
    // Проверка наличия файла
    if (file == NULL) {
        return NULL;
    }
    int num_lines = countWordsInFileQueue(filename); // Переменная, отвечающая за количество слов
    char** line = malloc(num_lines * sizeof(char*));
    for (int i = 0; i < num_lines; i++) line[i] = malloc(10000 * sizeof(char));
    Queue* queue = initQueue();
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