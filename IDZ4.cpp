#include <pthread.h>
#include <ctime>
#include <iostream>
#include <vector>
#include <unistd.h>
#include <fstream>

using namespace std;

const size_t flowers_number = 40; // кол-во потоков
const size_t gardeners_number = 2; // кол-во потоков
vector<int> garden = vector<int>(); // буфер для чисел
size_t time_count = 0;
int deathFlower = 0;

pthread_mutex_t mutex; // мьютекс для синхронизации вывода

bool write_to_file = false; // записывать ли в файл
string input_name; // имена файлов
string output_name;

const string FlowerState[] = { "FRESH", "WATERED","WILTED","DEAD"}; // 0  - FRESH, 1 - WATERED, 2 - WILTED, 3 - DEAD

// функция для генерации соятоний цветков
void generate_flowers(vector<int>& vector) {
    for (int i = 0; i < flowers_number; ++i) {
        vector.push_back(rand() % 3); // 0  - FRESH, 1 - WATERED, 2 - WILTED, 3 - DEAD
    }
}

// вывод сада в данный поток
void garden_to_stream(const vector<int>& vector, ostream& stream) {
    stream << "Garden: ";
    string mean = "";
    for (int elem : vector) {
        if (elem == 0) mean = "FRESH";
        else if (elem == 1) mean = "WATERED";
        else if (elem == 2) mean = "WILTED";
        else if (elem == 3) mean = "DEAD";
        stream << mean << " ";
    }
    stream << endl;
}

// функция для красивого вывода буфера
void print_garden(const vector<int> garden) {
    garden_to_stream(garden, cout);

    if (write_to_file) {
        ofstream output(output_name, ios_base::app);
        garden_to_stream(garden, output);
        output.close();
    }
}

// функция потоков-цветов
void* funcFlower(void* param) {
    int fNum = *((int*)param);
    int index = rand() % garden.size();

    // пока лимит времени не закончен
    while (time_count < 500) {
        ++time_count;
        sleep(1);
        pthread_mutex_lock(&mutex); //защита операции записи

        int changesProb = 1 + rand() % 2; // Вероятность того, что цветок поменяет состояние 50/50.

        if (changesProb != 1) {
            // Симулируем изменение состояния цветка
            if (garden[index] == 0) {
                garden[index] = 1;
            }
            else if (garden[index] == 1) {
                garden[index] = 2;
            }
            else if (garden[index] == 2) {
                garden[index] = 3;
            }
        }
        

        // уведомление об операции
        printf("Flower %d: has changed its state #%i = %i \n", fNum, index, garden[index]);

        if (write_to_file) {
            FILE* output = fopen(&output_name[0], "a");
            fprintf(output, "Flower %d: has changed its state #%i = %i \n", fNum, index, garden[index]);
            fclose(output);
        }

        pthread_mutex_unlock(&mutex); //конец критической секции
    }

    return nullptr;
}

// функция потоков-садовников
void* funcGardener(void* param) {
    int gNum = *((int*)param);

    // пока лимит времени не закончился
    while (time_count < 500) {
        ++time_count;
        sleep(1);
        pthread_mutex_lock(&mutex); //защита операции записи

        // полив цветов 
        for (int i = 0; i < garden.size(); i++) {
            if (garden[i] == 2) {

                // уведомление об операции
                printf("Gardener %d: watering flower #%i \n", gNum, i);

                if (write_to_file) {
                    FILE* output = fopen(&output_name[0], "a");
                    fprintf(output, "Gardener % d: watering flower # %i \n", gNum, i);
                    fclose(output);
                }

                garden[i] = 0;
            }
        }
        pthread_mutex_unlock(&mutex); //конец критической секции
    }

    return nullptr;
}

void handle_input() {
    ifstream input;
    do {
        // спрашиваем пользователя название файла
        cout << "Enter input file name: ";
        cin >> input_name;
        input.open(input_name);
        if (!input.is_open()) {
            cout << "Could not open the file - '" << input_name << "'" << endl;
        }
    } while (!input.is_open());

    input.close();
}

void handle_output() {
    cout << "Enter output file name: "; // спрашиваем пользователя, читать ли из файла
    cin >> output_name;
    write_to_file = true;

    // очистка файла
    ofstream output(output_name, ofstream::out | ofstream::trunc);
    output.close();
}

int main() {
    char answer;
    // спрашиваем пользователя, читать ли из файла
    cout << "Should we read from file? (Y/n) ";
    cin >> answer;
    if (answer == 'Y') {
        handle_input();
    }
    else {
        generate_flowers(garden);
    }

    // спрашиваем пользователя, писать ли в файл
    cout << "Should we write to file? (Y/n) ";
    cin >> answer;
    if (answer == 'Y') {
        handle_output();
    }

    // инициализация мьютекса
    pthread_mutex_init(&mutex, nullptr);

    // массивы потоков
    pthread_t threadF[flowers_number];
    int flowers[flowers_number];
    pthread_t threadG[gardeners_number];
    int gardeners[gardeners_number];

    // создание потоков
    for (int i = 0; i < flowers_number; ++i) {
        flowers[i] = i + 1;
        pthread_create(&threadF[i], NULL, funcFlower, (void*)(flowers + i));
    }
    for (int i = 0; i < gardeners_number; ++i) {
        gardeners[i] = i + 1;
        pthread_create(&threadG[i], NULL, funcGardener, (void*)(gardeners + i));
    }

    // соединяем снова все потоки
    for (int i = 0; i < flowers_number; ++i) {
        pthread_join(threadF[i], nullptr);
    }
    for (int i = 0; i < gardeners_number; ++i) {
        pthread_join(threadG[i], nullptr);
    }

    // результат
    cout << endl << "Result: " << endl;

    if (write_to_file) {
        ofstream output(output_name, ios_base::app);
        output << endl << "Result: " << endl;
        output.close();
    }

    print_garden(garden);
    for (int i =0; i<flowers_number;i++){
        if(garden[i]==3) deathFlower++;
    }
    printf("Total flowers deaths = %i", deathFlower);
    
    if (write_to_file) {
        ofstream output(output_name, ios_base::app);
        output << endl << "Total flowers deaths = " << deathFlower << endl;
        output.close();
    }
    return 0;
}


