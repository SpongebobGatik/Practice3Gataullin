#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include "stack.h"
#include "set.h"
#include "queue.h"
#include "table.h"
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#pragma comment(lib, "ws2_32.lib")
#define PORT 6379 // Определение порта для сервера
#define BACKLOG 10 // Определение максимального количества ожидающих подключений
#define BUFFER_SIZE 104857600 // Определение размера буфера

HANDLE mutex; // Объявление мьютекса
DWORD WINAPI handle_client(LPVOID lpParam); // Объявление функции обработки клиента

int main() {
    WSADATA wsa_data;
    int result = WSAStartup(MAKEWORD(2, 2), &wsa_data); // Инициализация библиотеки сокетов
    SOCKET listen_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP); // Создание сокета для прослушивания
    struct sockaddr_in server_address; // Структура для хранения адреса сервера
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = htonl(INADDR_ANY);
    server_address.sin_port = htons(PORT);
    result = bind(listen_socket, (struct sockaddr*)&server_address, sizeof(server_address)); // Привязка сокета к адресу
    result = listen(listen_socket, BACKLOG); // Начало прослушивания сокета
    printf("The server is running and waiting for connections on port %d\n", PORT);
    mutex = CreateMutex(NULL, FALSE, NULL); // Создание мьютекса
    while (1) {
        SOCKET client_socket = accept(listen_socket, NULL, NULL); // Принятие подключения от клиента
        CreateThread(NULL, 0, handle_client, (LPVOID)client_socket, 0, NULL); // Создание потока для обработки клиента
    }
    closesocket(listen_socket); // Закрытие сокета прослушивания
    WSACleanup(); // Очистка библиотеки сокетов
    return 0;
}

DWORD WINAPI handle_client(LPVOID lpParam) {
    WaitForSingleObject(mutex, INFINITE); // Получение мьютекса
    SOCKET client_socket = (SOCKET)lpParam; // Получение сокета клиента
    char* buffer = (char*)malloc(BUFFER_SIZE * sizeof(char));
    int bytes_received;
    // Получение данных от клиента, пока клиент не закроет подключение
    while ((bytes_received = recv(client_socket, buffer, BUFFER_SIZE, 0)) > 0) {
        buffer[bytes_received] = '\0'; // Добавление нуль-терминатора
        // Удаление символов возврата каретки и новой строки
        for (int i = 0; i < bytes_received; ++i) {
            if (buffer[i] == '\r' || buffer[i] == '\n') {
                buffer[i] = '\0';
                break;
            }
        }
        char** argv = NULL;
        int argc = 0;
        char* token = strtok(buffer, " "); // Разбиение полученных данных на токены
        while (token != NULL) {
            char** temp = realloc(argv, sizeof(char*) * (argc + 1));
            argv = temp;
            argv[argc++] = token;
            token = strtok(NULL, " ");
        }
        // Обработка полученных данных и выполнение соответствующих операций
        char* filename = NULL; // Имя файла
        char* query = NULL;
        char* key = NULL; // Ключ (Объект)
        char* basename = NULL; // Имя БД
        char* item = NULL; // Объект
        int temp; // Переменная, отвечающая за номер аргумента
        char* result = NULL;
        // Проверка количества аргументов
        if (argc < 4 || argc > 7) {
            result = malloc(100);
            sprintf(result, "Error.\n");
            goto skip;
        }
        // Цикл по аргументам
        for (int i = 0; i < argc; i++) {
            if (strcmp(argv[i], "--file") == 0 && i + 1 < argc) {
                filename = argv[i + 1];
            }
            else if (strcmp(argv[i], "--query") == 0 && i + 1 < argc) {
                query = argv[i + 1];
                temp = i + 1;
                basename = argv[i + 2];
                if (i + 5 > argc) key = argv[i + 3];
                else {
                    item = argv[i + 3];
                    key = argv[i + 4];
                    // Проверка наличия ключа и элемента
                    if (key == NULL || item == NULL) {
                        result = malloc(100);
                        sprintf(result, "Error.\n");
                        goto skip;
                    }
                }
            }
        }
        int pos1 = 0; // Переменная, отвечающая за позицию начала строки.
        int pos2 = 0; // Переменная, отвечающая за позицию конца строки.
        int status = 0; // Переменная-переключатель.
        // Обработка каждой команды
        if (filename != NULL && query != NULL) {
            FILE* file = fopen(filename, "r");
        if (!file) {
            FILE* file = fopen(filename, "w");
        }
        if (strcmp(argv[temp], "SPUSH") == 0) {
            if (key == NULL) {
                result = malloc(100);
                sprintf(result, "Error.\n");
                goto skip;
            }
            Stack* stack = loadFromFileStack(filename, basename, &pos1, &pos2, &status);
            if (stack == NULL) {
                result = malloc(100);
                sprintf(result, "Error when opening a file!\n");
                fclose(file);
            }
            else {
                if (pos1 + pos2 == 0) {
                    result = malloc(100);
                    sprintf(result, "Such a database, alas, does not exist!\n");
                    fclose(file);
                }
                else {
                    SPUSH(stack, key);
                    result = malloc(strlen(key) + 5);
                    sprintf(result, "-> %s\n", key);
                    if (status == 1) status = 0;
                    fclose(file);
                    saveToFileStack(stack, filename, basename, &pos1, &pos2, &status);
                }
            }
        }
        if (strcmp(argv[temp], "SPOP") == 0) {
            Stack* stack = loadFromFileStack(filename, basename, &pos1, &pos2, &status);
            if (stack == NULL) {
                result = malloc(100);
                sprintf(result, "Error when opening a file!\n");
                fclose(file);
            }
            else {
                if (pos1 + pos2 == 0) {
                    result = malloc(40);
                    sprintf(result, "Such a database, alas, does not exist!\n");
                    fclose(file);
                }
                else {
                    char* element = SPOP(stack);
                    result = malloc(strlen(element) + 5);
                    sprintf(result, "-> %s\n", element);
                    if (status == 2) status = 0;
                    fclose(file);
                    saveToFileStack(stack, filename, basename, &pos1, &pos2, &status);
                }
            }
        }
        if (strcmp(argv[temp], "SADD") == 0) {
            if (key == NULL) {
                result = malloc(100);
                sprintf(result, "Error.\n");
                goto skip;
            }
            Set* set = loadFromFileSet(filename, basename, &pos1, &pos2, &status);
            if (set == NULL) {
                result = malloc(100);
                sprintf(result, "Error when opening a file!\n");
                fclose(file);
            }
            else {
                if (pos1 + pos2 == 0) {
                    result = malloc(100);
                    sprintf(result, "Such a database, alas, does not exist!\n");
                    fclose(file);
                }
                else {
                    SADD(set, key);
                    result = malloc(strlen(key) + 5);
                    sprintf(result, "-> %s\n", key);
                    if (status == 1) status = 0;
                    fclose(file);
                    saveToFileSet(set, filename, basename, &pos1, &pos2, &status);
                }
            }
        }
        if (strcmp(argv[temp], "SREM") == 0) {
            if (key == NULL) {
                result = malloc(100);
                sprintf(result, "Error.\n");
                goto skip;
            }
            Set* set = loadFromFileSet(filename, basename, &pos1, &pos2, &status);
            if (set == NULL) {
                result = malloc(100);
                sprintf(result, "Error when opening a file!\n");
                fclose(file);
            }
            else {
                if (pos1 + pos2 == 0) {
                    result = malloc(100);
                    sprintf(result, "Such a database, alas, does not exist!\n");
                    fclose(file);
                }
                else {
                    SREM(set, key);
                    result = malloc(strlen(key) + 5);
                    sprintf(result, "-> %s\n", key);
                    if (status == 2) status = 0;
                    fclose(file);
                    saveToFileSet(set, filename, basename, &pos1, &pos2, &status);
                }
            }
        }
        if (strcmp(argv[temp], "SISMEMBER") == 0) {
            if (key == NULL) {
                result = malloc(100);
                sprintf(result, "Error.\n");
                goto skip;
            }
            Set* set = loadFromFileSet(filename, basename, &pos1, &pos2, &status);
            if (set == NULL) {
                result = malloc(100);
                sprintf(result, "Error when opening a file!\n");
                fclose(file);
            }
            else {
                if (pos1 + pos2 == 0) {
                    result = malloc(100);
                    sprintf(result, "Such a database, alas, does not exist!\n");
                    fclose(file);
                }
                else {
                    result = malloc(100);
                    if (SISMEMBER(set, key)) sprintf(result, "-> True\n");
                    else sprintf(result, "-> False\n");
                    fclose(file);
                }
            }
        }
        if (strcmp(argv[temp], "QPUSH") == 0) {
            if (key == NULL) {
                result = malloc(100);
                sprintf(result, "Error.\n");
                goto skip;
            }
            Queue* queue = loadFromFileQueue(filename, basename, &pos1, &pos2, &status);
            if (queue == NULL) {
                result = malloc(100);
                sprintf(result, "Error when opening a file!\n");
                fclose(file);
            }
            else {
                if (pos1 + pos2 == 0) {
                    result = malloc(100);
                    sprintf(result, "Such a database, alas, does not exist!\n");
                    fclose(file);
                }
                else {
                    QPUSH(queue, key);
                    result = malloc(strlen(key) + 5);
                    sprintf(result, "-> %s\n", key);
                    if (status == 1) status = 0;
                    fclose(file);
                    saveToFileQueue(queue, filename, basename, &pos1, &pos2, &status);
                }
            }
        }
        if (strcmp(argv[temp], "QPOP") == 0) {
            Queue* queue = loadFromFileQueue(filename, basename, &pos1, &pos2, &status);
            if (queue == NULL) {
                result = malloc(100);
                sprintf(result, "Error when opening a file!\n");
                fclose(file);
            }
            else {
                if (pos1 + pos2 == 0) {
                    result = malloc(100);
                    sprintf(result, "Such a database, alas, does not exist!\n");
                    fclose(file);
                }
                else {
                    char* element = QPOP(queue);
                    result = malloc(strlen(element) + 5);
                    sprintf(result, "-> %s\n", element);
                    if (status == 2) status = 0;
                    fclose(file);
                    saveToFileQueue(queue, filename, basename, &pos1, &pos2, &status);
                }
            }
        }
        if (strcmp(argv[temp], "HSET") == 0) {
            if (key == NULL || item == NULL) {
                result = malloc(100);
                sprintf(result, "Error.\n");
                goto skip;
            }
            HashTable* hashtable = loadFromFileTable(filename, basename, &pos1, &pos2, &status);
            if (hashtable == NULL) {
                result = malloc(100);
                sprintf(result, "Error when opening a file!\n");
                fclose(file);
            }
            else {
                if (pos1 + pos2 == 0) {
                    result = malloc(100);
                    sprintf(result, "Such a database, alas, does not exist!\n");
                    fclose(file);
                }
                else {
                    HSET(hashtable, key, item);
                    result = malloc(strlen(item) + strlen(key) + 20);
                    sprintf(result, "-> %s %s\n", item, key);
                    if (status == 1) status = 0;
                    fclose(file);
                    saveToFileTable(hashtable, filename, basename, &pos1, &pos2, &status);
                }
            }
        }
        if (strcmp(argv[temp], "HDEL") == 0) {
            if (key == NULL) {
                result = malloc(100);
                sprintf(result, "Error.\n");
                goto skip;
            }
            HashTable* hashtable = loadFromFileTable(filename, basename, &pos1, &pos2, &status);
            if (hashtable == NULL) {
                result = malloc(100);
                sprintf(result, "Error when opening a file!\n");
                fclose(file);
            }
            else {
                if (pos1 + pos2 == 0) {
                    result = malloc(100);
                    sprintf(result, "Such a database, alas, does not exist!\n");
                    fclose(file);
                }
                else {
                    HDEL(hashtable, key);
                    result = malloc(strlen(key) + 5);
                    sprintf(result, "-> %s\n", key);
                    if (status == 2) status = 0;
                    fclose(file);
                    saveToFileTable(hashtable, filename, basename, &pos1, &pos2, &status);
                }
            }
        }
        if (strcmp(argv[temp], "HGET") == 0) {
            if (key == NULL) {
                result = malloc(100);
                sprintf(result, "Error.\n");
                goto skip;
            }
            HashTable* hashtable = loadFromFileTable(filename, basename, &pos1, &pos2, &status);
            if (hashtable == NULL) {
                result = malloc(100);
                sprintf(result, "Error.\n");
                fclose(file);
            }
            else {
                if (pos1 + pos2 == 0) {
                    result = malloc(100);
                    sprintf(result, "Error.\n");
                    fclose(file);
                }
                else {
                    if (HGET(hashtable, key) != NULL) {
                        result = malloc(strlen(HGET(hashtable, key)) + 15);
                        sprintf(result, "%s\n", HGET(hashtable, key));
                    }
                    else {
                        result = malloc(100);
                        sprintf(result, "-> False\n");
                    }
                    fclose(file);
                }
            }
        }
    }
    else {
        result = malloc(100);
        sprintf(result, "Error.\n");
    }
    skip: {
    if (result == NULL) {
        result = malloc(100);
        sprintf(result, "Error.\n");
    }
    int bytes_sent = send(client_socket, result, strlen(result), 0); // Отправка результата клиенту
    }
    free(argv);
    free(result);
    }
    free(buffer);
    closesocket(client_socket); // Закрытие сокета клиента
    ReleaseMutex(mutex); // Освобождение мьютекса
}