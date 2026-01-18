#pragma once

#include <functional>
#include <memory>

#include "noncopyable.h"
#include "Timestamp.h"

class EventLoop;

/**
 * Chanel理解为通道，封装了socketfd和其感兴趣的event，如EPOLLIN, EPOLLOUT事件
 * 还绑定了poller返回的具体事件
 */
class Channel : noncopyable
{
public:
    using EventCallback = std::function<void()>;
    using ReadEventCallback = std::function<void(Timestamp)>;

    Channel(EventLoop* loop, int fd);
    ~Channel();

    // fd得到poller通知后，处理事件的方法
    void handleEvent(Timestamp receviceTime);

    // 设置回调函数对象
    void setReadCallback(ReadEventCallback cb) { readCallback_ = std::move(cb); }
    void setWriteCallback(EventCallback cb) { writeCallback_ = std::move(cb); }
    void setCloseCallback(EventCallback cb) { closeCallback_ = std::move(cb); }
    void setErrorCallback(EventCallback cb) { errorCallback_ = std::move(cb); }

    // 防止channel被remove掉后，还在执行回调操作
    void tie(const std::shared_ptr<void>&);

    int fd() const { return fd_; }
    int events() const { return events_; }
    int index() const { return index_; }
    void set_revents(int revt) { revents_ = revt; }
    void set_index(int idx) { index_ = idx; }

    // 设置fd当前的事件状态
    void enableReading() { events_ |= kReadEvent; update(); }
    void disableReading() { events_ &= kReadEvent; update(); }
    void enableWriting() { events_ |= kWriteEvent; update(); }
    void disableWriting() { events_ &= kWriteEvent; update(); }
    void disableAll() { events_ = kNoneEvent; update(); }

    // 返回当前fd的状态
    bool isNoneEvent() const { return events_ == kNoneEvent; }
    bool isWriting() const { return events_ & kWriteEvent; }
    bool isReading() const { return events_ & kReadEvent; }

    EventLoop* ownerLoop() { return loop_; }
    void remove();
private:
    void update();
    void handleEventWithGuard(Timestamp receviceTime);

    static const int kNoneEvent;    // 不对任何事件感兴趣
    static const int kReadEvent;    // 对读事件感兴趣
    static const int kWriteEvent;   // 对写事件感兴趣

    EventLoop* loop_;   // 事件循环
    const int fd_;      // Poller监听的对象
    int events_;        // 注册fd感兴趣的事件
    int revents_;        // Poller返回的具体发生的事件
    int index_;

    std::weak_ptr<void> tie_;
    bool tied_;

    // 因为channel通道里面能够获知fd最终发生的具体事件revents，所以他负责调用具体事件的回调操作
    ReadEventCallback readCallback_;
    EventCallback writeCallback_;
    EventCallback closeCallback_;
    EventCallback errorCallback_;
};