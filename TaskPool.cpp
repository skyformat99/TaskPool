#include "TaskPool.h"

namespace glaiel {
    TaskPool::TaskPool(){
        terminated = false;
        enqueued_tasks = 0;
        completed_tasks = 0;
    }

    TaskPool::~TaskPool(){
        StopThreads();
    }

    void TaskPool::SpinThreads(int nThreads){
        terminated = false;

        for(int i = 0; i<nThreads; i++){
            workers.push_back(std::thread(
                [this]{worker_main();}
            ));
        }
    }
    void TaskPool::StopThreads(){
        terminated = true;
        newtask_cv.notify_all();
        for(int i = 0; i<workers.size(); i++){
            workers[i].join();
        }
    }

    void TaskPool::Wait(){
        while(enqueued_tasks != completed_tasks){
            std::unique_lock<std::mutex> lck(completetask_cv_mutex);
            completetask_cv.wait(lck);
        }
    }

    void TaskPool::Flush(){
        Wait();
        tasks = std::queue<Task>();
        completed = std::queue<Task>();
        enqueued_tasks = 0;
        completed_tasks = 0;
    }
        
    //task interfaces
    //called from main thread
    void TaskPool::AddTask(std::function<void()> task, std::function<void()> on_complete){
        Task newtask;
        newtask.task = task;
        newtask.on_complete = on_complete;

        assigntask_mutex.lock();
        tasks.push(newtask);
        enqueued_tasks++;
        assigntask_mutex.unlock();

        newtask_cv.notify_one();
    }
        
    //called from worker threads
    bool TaskPool::ReceiveTask(Task& task){ //true if there was a task
        assigntask_mutex.lock();
        if(!tasks.empty()){
            task = tasks.front();
            tasks.pop();
            assigntask_mutex.unlock();
            return true;
        }

        assigntask_mutex.unlock();
        return false;
    }
        
        
    void TaskPool::CompleteTask(Task task){
        completetask_mutex.lock();
        completed.push(task);
        completed_tasks++;
        completetask_cv.notify_all();
        completetask_mutex.unlock();
    }
    void TaskPool::CallCompletes(){
        completetask_mutex.lock();
        while(!completed.empty()){
            Task exec = completed.front();
            completed.pop();

            completetask_mutex.unlock();
            if(exec.on_complete) exec.on_complete();
            completetask_mutex.lock();
        }
        completetask_mutex.unlock();
    }
    void TaskPool::Update(){
        CallCompletes();
    }
        
    //worker thread mains
    void TaskPool::worker_main(){
        while(!terminated){
            {
                std::unique_lock<std::mutex> lck(newtask_cv_mutex);
                newtask_cv.wait(lck);
            }
            Task task;
            while(ReceiveTask(task)){
                task.task();
                CompleteTask(task);
            }
        }
    }
}
