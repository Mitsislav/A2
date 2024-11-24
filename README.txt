###########################################
#					  #
#	Ioannis Chatziantwniou		  #
#		csd5193			  #
#					  #
###########################################

Description of the Code Implementation

Initially, I use global queues to store and monitor the student thread's position at any moment. I have a semaphore that I use for communication between threads with the bus as soon as they are initialized within the main function. By using a global mutex, I ensure that there is no thread race that would result in problems with changing variables and storing them in memory. I have counters for each possible position where students can be, at any time in the appropriate queue. I also have counters for the bus to check the number of students and the department they belong to. I do this in a variable called n/4. I have two enums for the possible locations where a student can be and for the department to which they belong. I have a struct student that contains all the necessary information for each student thread.

Now, regarding the int random, which returns a random number and accepts the range that the number can be within, we have the initializeStudents function that simply initializes each struct student that we have to store each student's information. Then, with the getPlace, getScience, it returns a string that essentially is the enum and just takes the number and checks what it is. With the printInfo(), we just print the appropriate messages on the screen. With the addQueue, we add a student to a queue based on student->position. With the popQueue, we simply remove the first element from a queue and reduce the counter. In the study thread, each student thread runs independently and goes to the university and then goes to stop b. In removeStopA and removeStopB, we remove the nth student from the queue and adjust the counters. For the bus event, which I will explain in more detail, it is the function that runs the bus thread. The startEvent is the function that triggers all threads, and in main, all the necessary initializations happen.

Now, as for the busEvent:

It consists of a while(1) loop, which runs as long as there are students in any queue. As soon as this condition breaks, the while loop stops with mutex lock to ensure that only one thread can be taken at a time. Using the allowBus, I check that the student-thread I am taking can enter the bus. Otherwise, the counter i++ is incremented, and we take the next one and print the appropriate message on the screen. If a student can enter, we use a semaphore to show that I took a student, and thus I can continue the loop of the bus queue. When the bus fills up or it cannot take more students, it goes to stop b where it unloads them and all the counters from students per department become 0, so the bus is ready to take the next batch of students without being affected by previous threads. Then, with bus_stopB, which is a flag that shows where the bus is, it goes up to stop b, and essentially is where the bus picks up students from stop b. If there are no students at stop a, then the urban bus returns to stop a and, if there are no students there either, it checks if there are students in queue b. If not, it waits for the last students and then completes its run.

Now as for startEvent:

Simply, the threads are initialized, and a signal is sent to wait on the bus to wait and not pick up students until the initialization of the student threads is complete.
