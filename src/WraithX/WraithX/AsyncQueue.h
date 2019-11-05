#pragma once

#include <list>
#include <mutex>
#include <cstdint>
#include <vector>
#include <atomic>

template <typename T>
// A class that handles storing and reading items in async
class AsyncQueue
{
public:
    // Constructor
    AsyncQueue()
    {
        // Set the initial queue size of 0 which has no limit
        IsQueueActive = true;
    }

    // Deconstructor with safe queue close
    ~AsyncQueue()
    {
        // Aquire a lock
        std::lock_guard<std::mutex> Lock(QueueMutex);

        // Set the queue to inactive
        IsQueueActive = false;
    }

    // Set this when we want the queue to stop
    std::atomic<bool> IsQueueActive;

private:
    // A list of items in the queue
    std::list<T> ItemsList;
    // A mutex to handle read and add operations
    std::mutex QueueMutex;
    // The maximum queue size before halting
    uint32_t MaximumQueueSize;

public:

    // Add an item to the queue
    void Add(T Item)
    {
        // We need to lock the queue
        std::lock_guard<std::mutex> Lock(QueueMutex);

        // Add the item
        ItemsList.push_back(Item);
    }

    // Adds a list of items to the queue
    void AddRange(const std::vector<T>& Range)
    {
        // We need to lock the queue
        std::lock_guard<std::mutex> Lock(QueueMutex);

        // Add the items
        for (auto Item : Range)
        {
            // Add it
            ItemsList.emplace_back(Item);
        }
    }

    // Gets an item from the queue and removes it
    bool Remove(T& Result)
    {
        // We need to lock the queue
        std::lock_guard<std::mutex> Lock(QueueMutex);
        
        // Check if we can even get an item
        if (ItemsList.empty())
        {
            // Can't get one
            return false;
        }
        else
        {
            // We can get one
            T Item = ItemsList.front();
            // Remove it
            ItemsList.pop_front();
            // Set it
            Result = Item;
        }
        
        // Success
        return true;
    }

    // Clears the queue
    void Clear()
    {
        // We need to lock the queue
        std::lock_guard<std::mutex> Lock(QueueMutex);

        // Clear it
        ItemsList.clear();
    }

    // Returns the amount of items in the queue
    uint64_t GetSize()
    {
        // We need to lock the queue
        std::lock_guard<std::mutex> Lock(QueueMutex);

        // Return result
        return ItemsList.size();
    }
};