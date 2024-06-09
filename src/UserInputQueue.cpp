#include "UserInputQueue.hpp"

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
    //Limit the keystroke buffer
    if(keystroke_queue.size() < 10)
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