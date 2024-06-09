#ifndef USER_INPUT_QUEUE_HPP
#define USER_INPUT_QUEUE_HPP

#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <raylib.h>

class UserInputQueue {
    private:
        static UserInputQueue* pinstance_;
        static std::mutex mutex_;

        std::mutex queue_mtx;
        std::queue<KeyboardKey> keystroke_queue;
    protected:
        UserInputQueue() {};
        ~UserInputQueue() {};
    public:
        UserInputQueue(UserInputQueue &other) = delete;
        void operator=(const UserInputQueue &) = delete;
        static UserInputQueue *GetInstance();

        void addKeyPressToQueue(KeyboardKey key);
        KeyboardKey getKeyPressFromQueue();
};

#endif