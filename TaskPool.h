#pragma once
#include <functional>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <vector>

namespace glaiel {
    //thread safe list-of-tasks to perform
    class TaskPool {
        private:
            struct Task {
                std::function<void()> task;        //executed on the worker thread
                std::function<void()> on_complete; //executed on the main thread
            };

            //no copying or assignment of task pools
            TaskPool& operator=(const TaskPool& rhs);
            TaskPool(const TaskPool& rhs);
        public:
            TaskPool();
            ~TaskPool();

            //USER FACING INTERFACES, CALLED FROM MAIN THREAD
            void SpinThreads(int nThreads); //initiate the threads
            void Wait();  //waits for all tasks to complete, does not stop threads
            void Flush(); //waits then clears OnComplete callbacks, does not call the callbacks
            void StopThreads(); //stops all the threads

            void AddTask(std::function<void()> task, std::function<void()> on_complete = std::function<void()>()); //add a task to the task pool
            void Update(); //call periodically on the main thread to trigger on_complete callbacks


            //ignore most of this
            //concurrecy shiz
            std::mutex assigntask_mutex;
            std::mutex completetask_mutex;

            std::mutex newtask_cv_mutex;
            std::condition_variable newtask_cv;

            std::mutex completetask_cv_mutex;
            std::condition_variable completetask_cv;

            std::vector<std::thread> workers;
            bool terminated;
        
            //task shiz
            int enqueued_tasks;
            int completed_tasks;
            std::queue<Task> tasks;
            std::queue<Task> completed;
        
            //called from worker threads
            bool ReceiveTask(Task& task);
        
            void CompleteTask(Task task);
            void CallCompletes();
        
            //worker thread mains
            void worker_main();
    };
}