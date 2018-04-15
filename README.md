# PURPOSE

I'm open sourcing some of the C++ utility code I've written for my game projects in the hopes that others will find them useful, and also possibly find some bugs in them before I do, or add features, or clean up some of the stuff I neglected, or whatever. Feel free to help out!

These utilities are C++11 (or newer, I'm not too strict about that), and meant to not need any wrapper code to use in my personal projects. They are all designed to fit a specific need of mine and aren't meant to be general purpose. 

# TaskPool

Simple threaded code helper, push tasks into the task pool and receive a callback once the task is complete. The callbacks always execute on the main thread (or a thread of your choice) when you call TaskPool::Update(), so you can avoid doing any mutex junk to sync whatever the task did with the rest of the program.

Create a task pool instance 
```TaskPool task_pool;

Spin up some threads on the task pool
```task_pool.SpinThreads(4);

Add some tasks to the task pool
```task_pool.AddTask(mytask, mytask_oncomplete);

Call update periodically to get on_complete callbacks
```task_pool.Update();

