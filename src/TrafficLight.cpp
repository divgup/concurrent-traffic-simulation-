#include <iostream>
#include <random>
#include "TrafficLight.h"

/* Implementation of class "MessageQueue" */


template <typename T>
T MessageQueue<T>::receive()
{
    // FP.5a : The method receive should use std::unique_lock<std::mutex> and _condition.wait() 
    // to wait for and receive new messages and pull them from the queue using move semantics. 
    // The received object should then be returned by the receive function. 
    std::unique_lock<std::mutex> lck(mtx);
    cv.wait(lck,[this]{
        return !_queue.empty();
    });
    TrafficLightPhase phase = _queue.front();
    _queue.clear();
    return phase;
}

template <typename T>
void MessageQueue<T>::send(T &&msg)
{
    // FP.4a : The method send should use the mechanisms std::lock_guard<std::mutex> 
    // as well as _condition.notify_one() to add a new message to the queue and afterwards send a notification.
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    std::lock_guard<std::mutex> uLock(mtx);
    _queue.emplace_back(std::move(msg));
    cv.notify_one();
}


/* Implementation of class "TrafficLight" */


TrafficLight::TrafficLight()
{
    _currentPhase = TrafficLightPhase::red;
}

void TrafficLight::waitForGreen()
{
    // FP.5b : add the implementation of the method waitForGreen, in which an infinite while-loop 
    // runs and repeatedly calls the receive function on the message queue. 
    // Once it receives TrafficLightPhase::green, the method returns.
    TrafficLightPhase msg;
    while(1){
        msg = msgQ.receive();
        // std::cout <<"current phase = "<< msg << std::endl;
        if(msg == TrafficLightPhase::green){
            break;
        }
    }
}

TrafficLightPhase TrafficLight::getCurrentPhase()
{
    return _currentPhase;
}

void TrafficLight::simulate()
{
    // FP.2b : Finally, the private method „cycleThroughPhases“ should be started in a thread when the public method „simulate“ is called. To do this, use the thread queue in the base class. 
    // std::unique_lock<std::mutex>lck(_mutex);
    // lck.lock();
    // std::cout << "entered simulation of Traffic light" << std::endl;
    threads.emplace_back(std::thread(&TrafficLight::cycleThroughPhases,this));
    // lck.unlock();
}

// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases()
{
    // FP.2a : Implement the function with an infinite loop that measures the time between two loop cycles 
    // and toggles the current phase of the traffic light between red and green and sends an update method 
    // to the message queue using move semantics. The cycle duration should be a random value between 4 and 6 seconds. 
    // Also, the while-loop should use std::this_thread::sleep_for to wait 1ms between two cycles. 
        std::random_device rd; 
        std::mt19937 eng(rd());
        std::uniform_int_distribution<> dist(4, 6);
        double cycleDuration = dist(eng);
        // std::cout << "duration of cycle is "<< cycleDuration << std::endl;

    for(;;){

        std::chrono::steady_clock::time_point t1 = std::chrono::steady_clock::now();
        std::chrono::steady_clock::time_point t2 = std::chrono::steady_clock::now(); 
        // std::unique_lock<std::mutex> lck(_mutex);
        // lck.lock();
        // std::cout << "duration of cycle is "<< cycleDuration << std::endl;
        this->_currentPhase=red;
        this->msgQ.send(std::move(this->_currentPhase));
        while(std::chrono::duration_cast<std::chrono::seconds>(t2-t1).count() < cycleDuration){    

            t2 = std::chrono::steady_clock::now();
        }
        // lck.unlock();
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        t1 = std::chrono::steady_clock::now();
        t2 = std::chrono::steady_clock::now();
        // lck.lock();
        this->_currentPhase=green;
        msgQ.send(std::move(this->_currentPhase));
        while(std::chrono::duration_cast<std::chrono::seconds>(t2-t1).count() < cycleDuration){    

            t2 = std::chrono::steady_clock::now();
        }        
        // lck.unlock();
    }
}

