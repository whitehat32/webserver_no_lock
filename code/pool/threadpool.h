/*
 * @Author       : mark
 * @Date         : 2020-06-15
 * @copyleft Apache 2.0
 */ 

#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <mutex>
#include <condition_variable>
#include <queue>
#include <list>
#include <thread>
#include <functional>
#include "spscq.h"
class ThreadPool {
public:
    explicit ThreadPool(size_t threadCount = 4): pool_(std::make_shared<Pool>()) {
            assert(threadCount > 0);
            this->pool_->tasks = std::vector<SpScLockFreeQueue<std::function<void()>, 10000>>(threadCount);
            // this->pool_->mtx = std::vector<std::mutex>(threadCount);
            // this->pool_->cond = std::vector<std::condition_variable>(threadCount);
            // sleep(100);
            for(size_t i = 0; i < threadCount; i++) {
                std::thread([pool = pool_, p=i] {
                    
                    // std::unique_lock<std::mutex> locker(pool->mtx[p]);
                    while(true) {
                        
                        // 去掉原先的锁机制和条件变量
                        if(!pool->tasks[p].empty()) {
                            std::cout << " p = " << p << std::endl;
                            // auto task = std::move(pool->tasks[p].front());
                            std::function<void()> task;
                            
                            if(pool->tasks[p].pop(task))task();
                            else continue;
                            // pool->tasks[p].pop();
                            // locker.unlock();
                            
                            // locker.lock();
                        } 
                        else if(pool->isClosed) break;
                        // else pool->cond[p].wait(locker);
                    }
                }).detach();
            }
    }

    ThreadPool() = default;

    ThreadPool(ThreadPool&&) = default;
    
    ~ThreadPool() {
        if(static_cast<bool>(pool_)) {
            {
                // std::lock_guard<std::mutex> locker(pool_->mtx_t);
                pool_->isClosed = true;
            }
            // for(int p = 0; p < this->getThreadNum(); p++) {
            //     pool_->cond[p].notify_all();
            // }
            
        }
    }

    template<class F>
    bool AddTask(F&& task, int p) {
        {
            // std::lock_guard<std::mutex> locker(pool_->mtx);
            if(pool_->tasks[p].push(std::forward<F>(task)))return true;
            else return false;
        }
        // std::cerr << " add = " << p << std::endl;
        // std::cout << " a = " <<pool_ << std::endl;
        // std::cout << "p = "<< p << " size = "<< pool_->tasks[p].size() << std::endl;
        // pool_->cond[p].notify_one();
    }

    int getThreadNum() {
        return pool_->tasks.size();
    }
private:
    struct Pool {
        // std::vector<std::mutex> mtx;
        // std::mutex mtx_t;
        // std::vector<std::condition_variable> cond;
        bool isClosed;
        std::vector<SpScLockFreeQueue<std::function<void()>, 10000>> tasks;
        
        // std::queue<std::function<void()>> tasks; 
    };
    std::shared_ptr<Pool> pool_;
};


#endif //THREADPOOL_H