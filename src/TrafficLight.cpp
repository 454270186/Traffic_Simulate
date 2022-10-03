#include <iostream>
#include <random>
#include <chrono>
#include "TrafficLight.h"

/* Implementation of class "MessageQueue" */


template <typename T>
T MessageQueue<T>::receive()
{
    // FP.5a : The method receive should use std::unique_lock<std::mutex> and _condition.wait() 
    // to wait for and receive new messages and pull them from the queue using move semantics. 
    // The received object should then be returned by the receive function. 
  
    std::unique_lock<std::mutex> uLock(_mutex);
    _condition.wait(uLock, [this](){ return !called_queue.empty(); });
  
    //pull the new message from the queue
    T msg = std::move(called_queue.back());
    called_queue.pop_back();
  
    return msg;
}

template <typename T>
void MessageQueue<T>::send(T &&msg)
{
    // FP.4a : The method send should use the mechanisms std::lock_guard<std::mutex> 
    // as well as _condition.notify_one() to add a new message to the queue and afterwards send a notification.
    
    //perform queue modification under the lock
    std::lock_guard<std::mutex> lck(_mutex);
  
    //add to queue
    std::cout << "   Message " << msg << " has been sent to the queue" << std::endl;
    called_queue.push_back(std::move(msg));
    _condition.notify_one(); //send a notification after add a new message to the queue
    
}


/* Implementation of class "TrafficLight" */


TrafficLight::TrafficLight()
{
    _currentPhase = TrafficLightPhase::red;
}


TrafficLight::~TrafficLight()
{
  
}

void TrafficLight::waitForGreen()
{
    // FP.5b : add the implementation of the method waitForGreen, in which an infinite while-loop 
    // runs and repeatedly calls the receive function on the message queue. 
    // Once it receives TrafficLightPhase::green, the method returns.
    
    while(true)
    {
      if(messages.receive() == green)
        return;
    }
}

TrafficLightPhase TrafficLight::getCurrentPhase()
{
    return _currentPhase;
}

void TrafficLight::simulate()
{
    // FP.2b : Finally, the private method „cycleThroughPhases“ should be started in a thread when the public method „simulate“ is called. To do this, use the thread queue in the base class. 
    threads.emplace_back(std::thread(&TrafficLight::cycleThroughPhases, this));
}

// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases()
{
    // FP.2a : Implement the function with an infinite loop that measures the time between two loop cycles 
    // and toggles the current phase of the traffic light between red and green and sends an update method 
    // to the message queue using move semantics. The cycle duration should be a random value between 4 and 6 seconds. 
    // Also, the while-loop should use std::this_thread::sleep_for to wait 1ms between two cycles. 
    bool infinity = true;
    
    //generate the random time
    constexpr int min_time = 4000;
    constexpr int max_time = 6000;
    std::random_device random_dev;
    std::default_random_engine engine(random_dev());
    std::uniform_int_distribution<int> distr(min_time,max_time);
    auto random_cycle_time = distr(engine);
  
    //infinite while loop
    auto time_1 = std::chrono::steady_clock::now();
    while(infinity)
    {
      //wait 1ms between two cycles
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
      
      auto time_2 = std::chrono::steady_clock::now();
      std::chrono::duration<double> elapsed_time = (time_2 - time_1);  
      double time_difference = (elapsed_time.count())*1000;
      
      if(time_difference > random_cycle_time)
      {
        //toggle the current phase of traffic light
        if(_currentPhase == red)
          _currentPhase = green;
        
        if(_currentPhase == green)
          _currentPhase = red;
      }
      
      auto time_1 = std::chrono::steady_clock::now();
      
      messages.send(std::move(_currentPhase));
    }
  
}

