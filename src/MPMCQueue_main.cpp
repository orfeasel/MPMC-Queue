// MPMCQueue.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <ios> //for bool alpha
#include "MPMCQueue.h"
#include <thread>
#include <mutex>
#include <chrono>
#include "Person.h"

std::mutex mutex;
int maxRunsPerThread = 15;

void printQueue(const MPMCQueue<int>& q)
{
    std::cout << "Printing queue...\n";
    std::cout << q;
}

template <typename T>
void printQueueTemplate(const MPMCQueue<T>& q)
{
    std::cout << "Printing templated queue...\n";
    std::cout << q;
}

void addRandomToQueue(MPMCQueue<int>& q, int num)
{
    //std::scoped_lock<std::mutex> lock {mutex};
    for (int i = 0; i < maxRunsPerThread; ++i)
    {
        q.push(num);
        //std::cout << "Pushed item:" << num << '\n';
        std::cout << q;
        /*if (q.try_push(num))
        {
            std::cout << "Pushed item:" << num << '\n';
        }*/
        //std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

void removeItemFromQueue(MPMCQueue<int>& q)
{
    for (int i = 0; i < maxRunsPerThread; ++i)
    {
        int popedElement = -1;
        q.pop(popedElement);
        std::cout << q;
        //std::cout << "Removed item:" << popedElement << '\n';
        /*if (q.try_pop(popedElement))
        {
            std::cout << "Removed item:" << popedElement << '\n';
        }*/
    }
}

void singleThreadPushPop(MPMCQueue<int>& q)
{
    for (size_t i = 0; i < q.getMaxCapacity(); ++i)
    {
        //q.try_push(static_cast<int>(i));
        q.push(static_cast<int>(i));
        //q.push_for(static_cast<int>(i),std::chrono::milliseconds(1));
    }
    printQueue(q);
    for (size_t i = 0; i < q.getMaxCapacity(); ++i)
    {
        /*int element = -1;
        q.pop(element);
        if (element!=-1)
        {
            std::cout << "Poped element:" << element << '\n';
            printQueue(q);
        }*/

        // ---- 
        int element=-1;
        if (q.try_pop(element))
        {
            std::cout << "Poped element:" << element << '\n';
            printQueue(q);
        }

        //-----
        /*auto poppedElement = q.try_pop();
        if (poppedElement.has_value())
        {
            std::cout<<"std::optional popped value:"<<poppedElement.value()<<'\n';
        }*/

        //-----
        /*int element=-1;
        if (q.pop_for(element, std::chrono::seconds(1)))
        {
            std::cout<<"pop_for value:"<<element<<'\n';
        }*/
    }
}

void runPersonsExample()
{
    std::shared_ptr<Person> orfeas{ std::make_shared<Person>("Orfeas", 31, "Software Dev") };
    //std::cout << *orfeas.get();

    MPMCQueue<std::shared_ptr<Person>> persons(10);
    auto p3 = std::thread([&]()
    {
        for (int i = 0; i < maxRunsPerThread; ++i)
        {
            persons.push(std::make_shared<Person>("Orfeas", 31, "Software Dev"));
        }

    });

    auto p4 = std::thread([&]()
    {
        for (int i = 0; i < maxRunsPerThread; ++i)
        {
            persons.push(std::make_shared<Person>("Silia", 31, "CEO"));
        }

    });
    auto c3 = std::thread([&]()
    {
        for (int i = 0; i < maxRunsPerThread; ++i)
        {
            std::shared_ptr<Person> outPerson;
            persons.pop(outPerson);
            if (outPerson.get())
            {
                std::cout << *outPerson.get();
            }
        }
    });
    auto c4 = std::thread([&]()
    {
        for (int i = 0; i < maxRunsPerThread; ++i)
        {
            std::shared_ptr<Person> outPerson;
            persons.pop(outPerson);
            if (outPerson.get())
            {
                std::cout << *outPerson.get();
            }
        }
    });

    p3.join();
    p4.join();
    c3.join();
    c4.join();
}

void runMultiThreadExample()
{
    MPMCQueue<int> q(10);

    auto p1 = std::thread(addRandomToQueue, std::ref(q), 1);
    auto p2 = std::thread(addRandomToQueue, std::ref(q), 2);
    auto c1 = std::thread(removeItemFromQueue, std::ref(q));
    auto c2 = std::thread(removeItemFromQueue, std::ref(q));

    p1.join();
    p2.join();
    c1.join();
    c2.join();
    printQueue(q);
}

int main()
{
    
    //runMultiThreadExample();
    //runPersonsExample();

    //Single thread push-pop
    //Simulating delayed close to unblock all waiting consumers
    MPMCQueue<int> q{5};
    auto pushPop = std::thread([&]()
    {
        singleThreadPushPop(q);
    });
    auto close=std::thread([&]()
    {
        std::this_thread::sleep_for(std::chrono::seconds(15));
        q.close();
    });
    pushPop.join();
    close.join();
    return 0;
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
