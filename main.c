#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>  // for explicit sleep() function

static int N;  // число болтунов

struct arguements {
    pthread_t thread;  // текущий поток (нужен был преимущественно для отладки)
    pthread_mutex_t* mutexes;  // ссылка на массив всех мьютексов
    int conversation_duration;  // продолжительность разговора в секундах
    FILE* output;  // для ввода данных в файл
};
typedef struct arguements Arguements;  // чтобы не писать каждый раз struct arguements


void* call(void* arguements) {
    Arguements *args = (Arguements *)arguements;  // явное преобразование типов

    int hasTalked = 0;  // переменная, которая показывает, поговорил ли данный болтун
    for (int i = 0; i < N; ++i) {
        if (pthread_mutex_trylock(args->mutexes + i) == 0) {
            printf("Начинаем разговор\n");
            fprintf(args->output, "%s", "Начинаем разговор\n");
            sleep(args->conversation_duration);
            printf("Заканчиваем разговор\n");
            fprintf(args->output, "%s", "Заканчиваем разговор\n");
            hasTalked = 1;
            break;
        }  // тут срабатывает неявный else
        // болтун с mutexes + i занят, звоним другому
        printf("Абонент (др.болтун) занят! Звоним другому!\n");
        fprintf(args->output, "%s", "Абонент (др.болтун) занят! Звоним другому!\n");
    }

    if (hasTalked == 1) {
        printf("Текущий болтун поговорил\n");
    } else {
        printf("Все абоненты заняты!\n");
    }
    return 0;
}

int main(int argc, char* argv[]) {
    if (argc != 5) {  // argv[0] is the name of executed file
        printf("Command line must have 1-st arguement (int (bool) value - 1, 2 or 3) - 1 - if user wants to input a number himself, 2 - generate a double number, 3 - take a number from input file; 2-nd & 3-rd parameters - full names of input & output files. The fourth arguement is double value: in 1-st case it is a number, in the 2-nd - seed of the generator, in the 3-rd - any value.\n");
        return 1;
    }
    FILE *input, *output;
    int seed;  // для генератора случ. чисел

    /* INPUT begin */
	if (atoi(argv[1]) == 1) {  // working with users's input number
		N = strtod(argv[4], 0);  // введенное пользователем число
    } else if (atoi(argv[1]) == 2) {  // working with generated number 
        seed = atoi(argv[4]);
		srand(seed);
		N = (double) (rand() % 7  + 2);
        // теперь N - псевдослучайное действит. число
    } else {  // take a number from input file
        input = fopen(argv[2], "r");
        fscanf(input, "%d", &N);
        fclose(input);
        // теперь N - число, взятое из файла
    }
    /* INPUT end */

    output = fopen(argv[3], "w");
	// fprintf(output, "%s", result);
	

    // поток - это болтун
    // модель:
    // n болтунов, n потоков, n мьютексов
    // scanf("%d", &N);
    pthread_t threads[N];  // массив всех потоков
    pthread_mutex_t mutexes[N];  // массив всех мьютексов
    for (int i = 0; i < N; ++i) {
        pthread_mutex_init(mutexes + i, NULL);  // инициализация всех мьютексов
    }

    Arguements args;  // структура аргументов, которые будем передавать в "функцию потока"
    args.output = output;
    for (int i = 0; i < N; ++i) {
        args.thread = threads[i];  // передаем ссылку на потоки
        args.mutexes = mutexes;  // передаем ссылку на мьютексы
        args.conversation_duration = 3;  // время разговора в секундах
        if (pthread_create(threads + i, NULL, &call, (void *)&args) != 0) {
            return i + 1;
        } else {
            printf("Звонок пошёл!\n");
            fprintf(output, "%s", "Звонок пошёл!\n");
        }
    }
    for (int i = 0; i < N; ++i) {
        if (pthread_join(threads[i], NULL) != 0) {  // wait until all our threads end
            return i + 5;
        } else {
            printf("Текущий болтун наговорился и отложил телефон.\n");
            fprintf(output, "%s", "Текущий болтун наговорился и отложил телефон.\n");
        }
    }
    
    for (int i = 0; i < N; ++i) {
        pthread_mutex_destroy(mutexes + i);  // чистим программу от мьютексов
    }
    fclose(output);
    return 0;
}
