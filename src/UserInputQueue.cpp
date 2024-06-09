#include "UserInputQueue.hpp"
#include <iostream>
#include <stdio.h>

UserInputQueue* UserInputQueue::pinstance_{nullptr};
std::mutex UserInputQueue::mutex_;

UserInputQueue *UserInputQueue::GetInstance()
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (pinstance_ == nullptr)
    {
        pinstance_ = new UserInputQueue();
    }
    return pinstance_;
}

void UserInputQueue::addKeyPressToQueue(KeyboardKey key)
{
    // Limit the keystroke buffer
    // NOTE:: We have a problem where the keystrokes are very laggy
    // And only sending one is not good enough, this is probably due to the
    // performance bottleneck from using mutexes
    // Now we sent continuous input from client and rate limit it to 8 inputs
    // This is hacky solution but works for now
    if(keystroke_queue.size() < 8)
    {
        std::lock_guard<std::mutex> lock(queue_mtx);
        keystroke_queue.push(key);
    }
}

KeyboardKey UserInputQueue::getKeyPressFromQueue()
{
    if(keystroke_queue.empty())
        return KEY_NULL;

    KeyboardKey key = keystroke_queue.front();
    keystroke_queue.pop();
    return key;
}