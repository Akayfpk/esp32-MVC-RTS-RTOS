#include "TaskManager.h"

// Category names for display
const char* CATEGORY_NAMES[] = {"Work", "Personal", "Shopping", "Other"};
const char* PRIORITY_NAMES[] = {"", "Low", "Low+", "Med", "High", "Critical"};

TaskManager::TaskManager() : taskCount(0), nextId(1), filterActive(false), filteredCount(0) {
}

bool TaskManager::addTask(const char* title, uint8_t priority, uint8_t category) {
    if (taskCount >= MAX_TASKS || title == nullptr || strlen(title) == 0) {
        return false;
    }
    
    // Validate inputs
    if (priority < 1 || priority > 5) priority = 3;
    if (category > 3) category = 0;
    
    // Create new task
    Task& newTask = tasks[taskCount];
    strncpy(newTask.title, title, 31);
    newTask.title[31] = '\0';
    newTask.priority = priority;
    newTask.category = category;
    newTask.isComplete = false;
    newTask.id = nextId++;
    
    taskCount++;
    rebuildFilteredList();
    return true;
}

bool TaskManager::deleteTask(uint16_t taskId) {
    uint8_t index = findTaskIndex(taskId);
    if (index >= taskCount) return false;
    
    // Shift all tasks after this one back
    for (uint8_t i = index; i < taskCount - 1; i++) {
        tasks[i] = tasks[i + 1];
    }
    
    taskCount--;
    rebuildFilteredList();
    return true;
}

bool TaskManager::toggleTaskComplete(uint16_t taskId) {
    Task* task = getTask(taskId);
    if (task == nullptr) return false;
    
    task->isComplete = !task->isComplete;
    rebuildFilteredList();
    return true;
}

bool TaskManager::editTask(uint16_t taskId, const char* newTitle, 
                          uint8_t newPriority, uint8_t newCategory) {
    Task* task = getTask(taskId);
    if (task == nullptr) return false;
    
    if (newTitle != nullptr) {
        strncpy(task->title, newTitle, 31);
        task->title[31] = '\0';
    }
    
    if (newPriority >= 1 && newPriority <= 5) {
        task->priority = newPriority;
    }
    
    if (newCategory <= 3) {
        task->category = newCategory;
    }
    
    rebuildFilteredList();
    return true;
}

Task* TaskManager::getTask(uint16_t taskId) {
    uint8_t index = findTaskIndex(taskId);
    return (index < taskCount) ? &tasks[index] : nullptr;
}

Task* TaskManager::getTaskByIndex(uint8_t index) {
    if (filterActive) {
        return (index < filteredCount) ? &tasks[filteredIndices[index]] : nullptr;
    } else {
        return (index < taskCount) ? &tasks[index] : nullptr;
    }
}

uint8_t TaskManager::getCompletedCount() const {
    uint8_t completed = 0;
    for (uint8_t i = 0; i < taskCount; i++) {
        if (tasks[i].isComplete) completed++;
    }
    return completed;
}

uint8_t TaskManager::getCompletionPercentage() const {
    if (taskCount == 0) return 0;
    return (getCompletedCount() * 100) / taskCount;
}

void TaskManager::sortTasks(SortBy sortType) {
    // Simple bubble sort (fine for 30 items)
    for (uint8_t i = 0; i < taskCount - 1; i++) {
        for (uint8_t j = 0; j < taskCount - i - 1; j++) {
            bool shouldSwap = false;
            
            switch (sortType) {
                case SORT_BY_PRIORITY:
                    shouldSwap = tasks[j].priority < tasks[j + 1].priority;
                    break;
                case SORT_BY_STATUS:
                    shouldSwap = tasks[j].isComplete && !tasks[j + 1].isComplete;
                    break;
                case SORT_BY_CATEGORY:
                    shouldSwap = tasks[j].category > tasks[j + 1].category;
                    break;
                default:
                    shouldSwap = false;
            }
            
            if (shouldSwap) {
                swapTasks(j, j + 1);
            }
        }
    }
    rebuildFilteredList();
}

const char* TaskManager::getCategoryName(uint8_t category) {
    return (category <= 3) ? CATEGORY_NAMES[category] : "Unknown";
}

const char* TaskManager::getPriorityName(uint8_t priority) {
    return (priority >= 1 && priority <= 5) ? PRIORITY_NAMES[priority] : "Unknown";
}

// Private helper methods
uint8_t TaskManager::findTaskIndex(uint16_t taskId) {
    for (uint8_t i = 0; i < taskCount; i++) {
        if (tasks[i].id == taskId) return i;
    }
    return MAX_TASKS; // Invalid index
}

void TaskManager::rebuildFilteredList() {
    // For now, just show all tasks (filtering logic can be added later)
    filteredCount = taskCount;
    for (uint8_t i = 0; i < taskCount; i++) {
        filteredIndices[i] = i;
    }
}

void TaskManager::swapTasks(uint8_t i, uint8_t j) {
    Task temp = tasks[i];
    tasks[i] = tasks[j];
    tasks[j] = temp;
}

void TaskManager::reset() {
    taskCount = 0;
    nextId = 1;
    filterActive = false;
    filteredCount = 0;
}
std::vector<Task> TaskManager::getAllTasks() {
    std::vector<Task> allTasks;
    // Use filtered or unfiltered depending on your UI
    // Here, return all visible tasks based on current filter:
    for (uint8_t i = 0; i < filteredCount; i++) {
        Task* t = getTaskByIndex(i);
        if (t != nullptr) {
            allTasks.push_back(*t);  // copy Task
        }
    }
    return allTasks;
}
