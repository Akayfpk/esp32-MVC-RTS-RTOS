#ifndef TASKMANAGER_H
#define TASKMANAGER_H

#include <Arduino.h>
#include <vector>

// Task data structure
struct Task {
    char title[32];           // Task title (31 chars + null terminator)
    uint8_t priority;         // 1-5 (1=lowest, 5=highest)
    bool isComplete;          // true/false
    uint8_t category;         // 0-3 (Work, Personal, Shopping, Other)
    uint16_t id;              // Unique task ID
    
    // Constructor for easy task creation
    Task() : priority(3), isComplete(false), category(0), id(0) {
        title[0] = '\0';
    }
    
    Task(const char* taskTitle, uint8_t taskPriority = 3, uint8_t taskCategory = 0) 
        : priority(taskPriority), isComplete(false), category(taskCategory), id(0) {
        strncpy(title, taskTitle, 31);
        title[31] = '\0';
    }
};

// Category definitions
enum TaskCategory {
    WORK = 0,
    PERSONAL = 1,
    SHOPPING = 2,
    OTHER = 3
};

// Sorting options
enum SortBy {
    SORT_BY_PRIORITY,
    SORT_BY_STATUS,
    SORT_BY_CATEGORY,
    SORT_BY_CREATION_ORDER
};

class TaskManager {
public:
    TaskManager();
    std::vector<Task> getAllTasks();

    // Core task operations
    bool addTask(const char* title, uint8_t priority = 3, uint8_t category = 0);
    bool deleteTask(uint16_t taskId);
    bool toggleTaskComplete(uint16_t taskId);
    bool editTask(uint16_t taskId, const char* newTitle = nullptr, 
                  uint8_t newPriority = 0, uint8_t newCategory = 255);
    
    // Task retrieval
    Task* getTask(uint16_t taskId);
    Task* getTaskByIndex(uint8_t index);
    uint8_t getTaskCount() const { return taskCount; }
    uint8_t getCompletedCount() const;
    uint8_t getCompletionPercentage() const;
    
    // Filtering and sorting
    void sortTasks(SortBy sortType);

    
    // Category helpers
    static const char* getCategoryName(uint8_t category);
    static const char* getPriorityName(uint8_t priority);
    
    // Debug and utilities
    void printAllTasks();
    void reset();
    
private:
    static const uint8_t MAX_TASKS = 30;
    Task tasks[MAX_TASKS];
    uint8_t taskCount;
    uint16_t nextId;
    
    // Filtering state
    bool filterActive;
    uint8_t filteredIndices[MAX_TASKS];
    uint8_t filteredCount;
    
    // Helper methods
    uint8_t findTaskIndex(uint16_t taskId);
    void rebuildFilteredList();
    void swapTasks(uint8_t i, uint8_t j);
};

#endif