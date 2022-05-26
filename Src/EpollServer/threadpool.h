#ifndef _THREADPOOL_H
#define _THREADPOOL_H

#include <mutex>
#include <condition_variable>
#include <queue>
#include <thread>
#include <functional>

class ThreadPool {
public:
	explicit ThreadPool(size_t threadCount = 8): m_pool(std::make_shared<Pool>()) {				
        assert(threadCount > 0);
        // 1.创建threadCount条线程，线程传入匿名函数
        // 2.detach： 从thread对象分离执行线程，允许执行独立地持续。一旦该线程退出，
        // 则释放任何分配的资源。调用 detach 后 *this 不再占有任何线程。
        for(size_t i = 0; i < threadCount; i++) {
            std::thread([pool = m_pool] {
                std::unique_lock<std::mutex> locker(pool->mtx); 	// 给线程池上锁
                while(true) {
                    if(!pool->tasks.empty()) {						// 任务队列有任务
                        auto task = std::move(pool->tasks.front());	// 从任务队列头部取任务
                        pool->tasks.pop();
                        locker.unlock();							// 任务取出来了，给池子解锁
                        task();										// 运行任务
                        locker.lock();								// 给池子再次上锁，因为循环取任务的
                    } 
                    else if(pool->isClosed) break; 					// 线程池没有开启
                    else pool->cond.wait(locker);					// 没有任务会阻塞，等待有任务通知了，才会唤醒
                }
            }).detach();
        }
    }

    ThreadPool() = default;
    ThreadPool(ThreadPool&&) = default;
    
    ~ThreadPool() {
        if(static_cast<bool>(m_pool)) {
            {
                // lock_guard: 作用域锁， 析构池子，把池子的状态设置为关闭
                std::lock_guard<std::mutex> locker(m_pool->mtx);
                m_pool->isClosed = true;
            }
            m_pool->cond.notify_all();	// 池子关闭了，通知所有线程关闭
        }
    }
	// 给池子的任务队列添加任务
    template<class F>
    void AddTask(F&& task) {
        {
            std::lock_guard<std::mutex> locker(m_pool->mtx);
            m_pool->tasks.emplace(std::forward<F>(task));	// 添加任务函数
        }
        m_pool->cond.notify_one();							// 有任务了，通知一个线程去抢
    }

private:
    // Pool: 存放线程的池子
    struct Pool {
        std::mutex mtx;							 // 用于保护共享数据免受从多个线程同时访问的同步原语
        std::condition_variable cond;			 // 用于阻塞一个线程，或同时阻塞多个线程，直至另一线程修改共享变量（条件）并通知cond
        bool isClosed;							 // 标记线程池是否开启状态
        std::queue<std::function<void()>> tasks; // 任务队列
    };
    std::shared_ptr<Pool> m_pool;
};

#endif /* _THREADPOOL_H */

