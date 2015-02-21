#trash
###Current Version: 0.2 alpha

#### *Arguably better than "bash"! (not really)*

Big project that I really worked hard on to get right. First time really working with malloc/realloc calls, and it was tough finding efficient implementations for parsing commands and forking child processes to run commands. Valuable learning experience that I had a *lot* of fun with!

##Getting Started
####First off, you will have to compile a few of the files to get the basic functionality up and running.
    ~$ clang trash.c -o trash
    ~$ clang cleanup.c -o cleanup
I use clang to compile here but you can use whatever C compiler you wish to use! (Like gcc)

####After compiling these main files, you can run these commands to run their respective executables
#####Start up the *trash* shell
    ~$ ./trash

#####Clean up your log files
    ~$ ./cleanup

##Usage
####Once running in the *trash* environment, you can start using commands!
    trash-v.~$ <...> <...> ...
  
####Get an overview of what's going on in the process slots
    trash-v.~$ i

####Get the output (if any) of the most recent process in slot <N>
    trash-v.~$ o <N> 

####Display the log files directly in the shell environment
    trash-v.~$ l <N>

####Run a command in process slot <N> -- Will kill a process if present in that slot
    trash-v.~$ r <N> <...>

####Run a command in process slot <N> -- Will wait for the process that is present in that slot (if any)
    trash-v.~$ w <N> <...>
  
####Pause a process running in slot <N> (if any)
    trash-v.~$ z <N>
  
####Continue a paused process in slot <N> (if any)
    trash-v.~$ g <N>
  
####List the current directory's contents
    trash-v.~$ ls

####Change directory
    trash-v.~$ cd <...>

####Perform basic calculator operations
    trash-v.~$ <N> <OPERATION> <N>
#####Supports addition, subtraction, division, multiplication, square root, exponents, modulo **AS OF v0.2 ALPHA**
    trash-v.~$ 34 % 5
        4.00000

####Display what commands are available
    trash-v.~$ h

####Quit the trash shell environment
    trash-v.~$ q

##Upcoming Features
+ More process slots
+ Secret *fun* commands :D
+ Support for command queueing

##Changelog
####Version 0.2 alpha
+ Added rudimentary calculator functionality
 - Addition, subtraction, division, multiplication, modulo, exponents
+ Fixed 'w' command bug where queueing would segfault
+ Changed linked_list struct implementation to avoid memory leaks
+ Added 'o' command
+ Added 'z'/'g' commands
+ Started work on secret command

####Version 0.1 alpha
+ First release!
+ Basic commands support
+ Can run commands in process slots (4)
+ Requires user to type ./trash instead of just trash
