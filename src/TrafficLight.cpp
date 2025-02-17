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
    std::unique_lock<std::mutex> uLock(_mtx);
    _var.wait(uLock);

    //Move semantics here
    T receivedMessage = std::move(_queue.back());

    //Remove last element from _message
    _queue.pop_back();

    //return recieved object here
    return receivedMessage;

}   

template <typename T>
void MessageQueue<T>::send(T &&msg)
{
    // FP.4a : The method send should use the mechanisms std::lock_guard<std::mutex> 
    // as well as _condition.notify_one() to add a new message to the queue and afterwards send a notification.
    std::lock_guard<std::mutex> uLock(_mtx);
    
    _queue.emplace_back(std::move(msg));
    
    _var.notify_one();
}


/* Implementation of class "TrafficLight" */
 
TrafficLight::TrafficLight()
{
    _currentPhase = TrafficLightPhase::red;
}

void TrafficLight::WaitForGreen()
{
    // FP.5b : add the implementation of the method waitForGreen, in which an infinite while-loop 
    // runs and repeatedly calls the receive function on the message queue. 
    // Once it receives TrafficLightPhase::green, the method returns.
    while(true){
        
        if(_message.receive() == TrafficLightPhase::green)
            break;
    }
}

TrafficLightPhase TrafficLight::GetCurrentPhase()
{
    return _currentPhase;
}

void TrafficLight::Simulate()
{
    // FP.2b : Finally, the private method „cycleThroughPhases“ should be started in a thread when the public method „simulate“ is called. To do this, use the thread queue in the base class. 
    
    threads.emplace_back(std::thread(&TrafficLight::CycleThroughPhases, this));
}


// virtual function which is executed in a thread
void TrafficLight::CycleThroughPhases()
{
    std::cout<<"Cycle called!"<<std::endl;
    // FP.2a : Implement the function with an infinite loop that measures the time between two loop cycles 
    // and toggles the current phase of the traffic light between red and green and sends an update method 
    // to the message queue using move semantics. The cycle duration should be a random value between 4 and 6 seconds. 
    // Also, the while-loop should use std::this_thread::sleep_for to wait 1ms between two cycles. 
    
    std::chrono::time_point<std::chrono::system_clock> lastUpdate;
    //init stop watch
    lastUpdate = std::chrono::system_clock::now();
    
    //Infinite loop
    while(true){
        
        //Compute time difference to stop watch
        long timeSinceLastUpdate = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - lastUpdate).count();
        
        //Casting based on found stackoverflow post:
        //https://stackoverflow.com/questions/31255486/c-how-do-i-convert-a-stdchronotime-point-to-long-and-back
        
        auto now_s = std::chrono::time_point_cast<std::chrono::seconds>(lastUpdate);
        auto epoch = now_s.time_since_epoch();
        auto value = std::chrono::duration_cast<std::chrono::seconds>(epoch);

        unsigned int duration = value.count();
        //Base the srand seed on duration of now from epoch, should give a different number each time run
        // How is that for randomeness?
        srand(duration);

        //Sleep for 4 to 6 seconds using rand(), Right way to implement it?
        std::this_thread::sleep_for(std::chrono::seconds(rand() % 4 + 2));
        
        //Toggle the current phase of the traffic light, if else statements the way to go?
        if(_currentPhase == TrafficLightPhase::red)
            _currentPhase = TrafficLightPhase::green;
        
        else if(_currentPhase == TrafficLightPhase::green)
            _currentPhase = TrafficLightPhase::red;
    
        //send an update method to the message queue using move semantics, new light, and timeSinceLastUpdate

        //Push each new TrafficLightPhase into _message by calling send into conjuction with move semantics
        _message.send(std::move(_currentPhase)); 
      
        //Sleep for 1 millisecond
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    //reset stop watch
    lastUpdate = std::chrono::system_clock::now();
}

