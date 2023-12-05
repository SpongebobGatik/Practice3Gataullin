#define _CRT_SECURE_NO_WARNINGS
#include "stack.h" 
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 

// Функция подсчета слов в файле
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

// Функция для инициализации стека
Stack* initStack() {
    Stack* stack = (Stack*)malloc(sizeof(Stack));
    stack->top = NULL;
    return stack;
}

// Функция для добавления элемента в стек
void SPUSH(Stack* stack, char* element) {
    NodeStack* newNode = (NodeStack*)malloc(sizeof(NodeStack)); // Выделение памяти под новый узел
    newNode->element = _strdup(element); // Копирование элемента в узел
    newNode->next = stack->top; // Установка следующего узла после нового узла
    stack->top = newNode; // Установка нового узла в качестве вершины стека
}

// Функция для удаления элемента из стека
char* SPOP(Stack* stack) {
    // Проверка пустоты стека
    if (stack->top == NULL) {
        return ("The stack is empty");
    }
    NodeStack* poppedNode = stack->top; // Получение узла для удаления
    stack->top = poppedNode->next; // Установка следующего узла в качестве вершины стека
    char* element = poppedNode->element; // Получение элемента из узла
    free(poppedNode); // Освобождение памяти, занятой узлом
    return element; // Возврат элемента
}

// Функция сохранения изменений в файл
void saveToFileStack(Stack* stack, const char* filename, const char* basename, int* pos1, int* pos2, int* status) {
    FILE* file = fopen(filename, "r");
    FILE* tempFile = fopen("temp.data", "w");
    int ch; // Символ, который требуется записать
    fseek(file, 0, SEEK_SET); // Установка курсора в начало
    fseek(tempFile, 0, SEEK_SET); // Установка курсора в начало
    while ((ch = fgetc(file)) != EOF) { // Цикл, пока не выдаст ошибку при записи
        fputc(ch, tempFile); // Запись символа в поток
        if (ftell(tempFile) == *pos1 - 2 && *status == 2)  // Если пустая БД, то добавляем табуляцию перед элементом
            fprintf(tempFile, "\t%s", stack->top->element);
        else if (ftell(tempFile) == *pos1) {
            NodeStack* currentNode = stack->top; // Ставим указатель на вершину стека
            NodeStack* prevNode = NULL; // Временная переменная-указатель
            // Цикл для перевёртывания стека
            while (currentNode != NULL) {
                NodeStack* nextNode = currentNode->next;
                currentNode->next = prevNode;
                prevNode = currentNode;
                currentNode = nextNode;
            }
            currentNode = prevNode;
            while (currentNode != NULL) { // Цикл пока не закончатся элементы
                if (currentNode->next == NULL)
                    fprintf(tempFile, "%s\n", currentNode->element); // Если нет следующего элемента, то добавляем строку
                else // Иначе добавляем после элемента табуляцию
                    fprintf(tempFile, "%s\t", currentNode->element);
                currentNode = currentNode->next;
            }
            // Добавление новую строку, если БД пуста
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

// Функция загрузки БД в структуру
Stack* loadFromFileStack(const char* filename, const char* basename, int* pos1, int* pos2, int* status) {
    FILE* file = fopen(filename, "r");
    // Проверка наличия файла
    if (file == NULL) {
        return NULL;
    }
    int num_lines = countWordsInFileStack(filename); // Переменная, отвечающая за количество слов
    char** line = malloc(num_lines * sizeof(char*));
    for (int i = 0; i < num_lines; i++) line[i] = malloc(10000 * sizeof(char));
    Stack* stack = initStack();
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