#include <iostream>
#include <thread>
#include <vector>
#include <mutex>
#include <random>
#include <chrono>
#include <algorithm>

// Minatours present opening

//Node structure for linked list
class Node {
public:
    int data;
    Node* next;
    Node(int value) : data(value), next(nullptr) {}
};

//Thread-safe linked list implementation
class ThreadSafeLinkedList {
private:
    Node* head;
    std::mutex listMutex;

public:
    ThreadSafeLinkedList() : head(nullptr) {}

    //Insert present in sorted order
    void insert(int value) {
        std::lock_guard<std::mutex> lock(listMutex);
        Node** curr = &head;
        while (*curr && (*curr)->data < value) {
            curr = &((*curr)->next);
        }
        Node* newNode = new Node(value);
        newNode->next = *curr;
        *curr = newNode;
    }

    // Remove present and return successs status
    bool remove(int value) {
        std::lock_guard<std::mutex> lock(listMutex);
        Node** curr = &head;
        while (*curr && (*curr)->data != value) {
            curr = &((*curr)->next);
        }
        if (*curr) {
            Node* temp = *curr;
            *curr = (*curr)->next;
            delete temp;
            return true;
        }
        return false;
    }
};

//Function for servant threads
void servantTask(ThreadSafeLinkedList &list, const std::vector<int> &presents, int &thankYouCount, std::mutex &thankYouMutex) {
    for (int present : presents) {
        list.insert(present);
        if (list.remove(present)) {
            std::lock_guard<std::mutex> lock(thankYouMutex);
            thankYouCount++;
        }
    }
}

//Runs Problem 1 - Minotaur's Presents
void runProblem1() {
    ThreadSafeLinkedList list;
    const int NUM_SERVANTS = 4;
    int numPresents;
    int thankYouCount = 0;
    std::mutex thankYouMutex;
    
    //Read user input for number of presents
    std::cout << "Enter number of presents: ";
    std::cin >> numPresents;
    
    std::vector<std::thread> servants;
    std::vector<int> presents(numPresents);
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dist(1, numPresents);
    
    //Generate random presents
    for (int &present : presents) present = dist(gen);
    
    auto startTime = std::chrono::high_resolution_clock::now();
    size_t chunkSize = presents.size() / NUM_SERVANTS;
    std::vector<std::vector<int>> chunks(NUM_SERVANTS);
    
    //Split presents among servant threads//
    for (int i = 0; i < NUM_SERVANTS; i++) {
        auto start = presents.begin() + i * chunkSize;
        auto end = (i == NUM_SERVANTS - 1) ? presents.end() : start + chunkSize;
        chunks[i] = std::vector<int>(start, end);
        servants.emplace_back(servantTask, std::ref(list), std::ref(chunks[i]), std::ref(thankYouCount), std::ref(thankYouMutex));
    }
    
    //Join all servant threads//
    for (auto &servant : servants) servant.join();
    auto endTime = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> duration = endTime - startTime;
    
    //Output results
    std::cout << "Solution 1:\n";
    std::cout << "Thread Count = " << numPresents << "\n";
    std::cout << "Duration = " << duration.count() << " ms\n";
}

// PROBLEM 2 Temprature monitoring

const int NUM_SENSORS = 8;
const int READINGS_PER_HOUR = 60;
std::vector<int> temperatureReadings;
std::mutex tempMutex;

//Function for each sensor to record temperatures
void sensorTask(int sensorID) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dist(-100, 70);
    
    for (int i = 0; i < READINGS_PER_HOUR / NUM_SENSORS; i++) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10)); //Simulating sensor delay
        int temp = dist(gen);
        
        std::lock_guard<std::mutex> lock(tempMutex);
        temperatureReadings.push_back(temp);
    }
}

//Analyzes recorded temprature data
void analyzeTemperatureData() {
    std::lock_guard<std::mutex> lock(tempMutex);
    
    int maxFluctuation = 0;
    for (int i = 0; i <= READINGS_PER_HOUR - 10; i++) {
        int fluctuation = *std::max_element(temperatureReadings.begin() + i, temperatureReadings.begin() + i + 10) -
                          *std::min_element(temperatureReadings.begin() + i, temperatureReadings.begin() + i + 10);
        if (fluctuation > maxFluctuation) {
            maxFluctuation = fluctuation;
        }
    }
    
    //Output largest temperature fluctuation//
    std::cout << "Solution 2:\n";
    std::cout << "Largest Temperature Delta = " << maxFluctuation << " Degrees\n";
}

//Runs Problem 2 - Mars Rover Temperature Monitoring
void runProblem2() {
    std::vector<std::thread> sensors;
    
    //Create 8 sensor threads//
    for (int i = 0; i < NUM_SENSORS; i++) {
        sensors.emplace_back(sensorTask, i);
    }
    
    //Join all sensor threads//
    for (auto &sensor : sensors) {
        sensor.join();
    }
    
    analyzeTemperatureData();
}

int main() {
    runProblem1();  //Execute Problem 1
    runProblem2();  //Execute problem 2
    return 0;
}
